// dllfinder.cpp
// Usage:  dllfinder <target.exe|dll> <destination folder> [extra search folder ...]
//
// Extra search folders are checked for DLLs (e.g. your own bin/ with render3d.dll)
// before PATH and the system folders.
//
// Walks the import tables of the target (recursively), finds every non-system
// DLL it needs, and copies them into the destination folder (created if missing).
//
// Build (MinGW):
//   g++ -std=c++17 -O2 dllfinder.cpp -o dllfinder.exe -lshell32 -static
//
// Build (MSVC):
//   cl /std:c++17 /EHsc dllfinder.cpp shell32.lib
//
// Exit codes: 0 = ok, 1 = one or more copies failed, 2 = bad arguments / unreadable target

#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ---------------------------------------------------------------- helpers

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

static fs::path windowsDirectory() {
    wchar_t buf[MAX_PATH];
    UINT n = GetWindowsDirectoryW(buf, MAX_PATH);
    return fs::path(std::wstring(buf, n));
}

static bool isUnder(const fs::path& file, const fs::path& dir) {
    std::wstring f = fs::weakly_canonical(file).wstring();
    std::wstring d = fs::weakly_canonical(dir).wstring();
    std::transform(f.begin(), f.end(), f.begin(), ::towlower);
    std::transform(d.begin(), d.end(), d.begin(), ::towlower);
    return f.size() > d.size() && f.compare(0, d.size(), d) == 0;
}

// ------------------------------------------------------------- PE parsing

struct PeFile {
    std::vector<uint8_t> data;
    const IMAGE_SECTION_HEADER* sections = nullptr;
    WORD numSections = 0;
    DWORD importRva = 0;
    DWORD delayRva = 0;

    bool load(const fs::path& p) {
        std::ifstream f(p, std::ios::binary);
        if (!f) return false;
        data.assign(std::istreambuf_iterator<char>(f), {});
        if (data.size() < sizeof(IMAGE_DOS_HEADER)) return false;

        auto dos = (const IMAGE_DOS_HEADER*)data.data();
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
        if ((size_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS32) > data.size()) return false;

        auto nt32 = (const IMAGE_NT_HEADERS32*)(data.data() + dos->e_lfanew);
        if (nt32->Signature != IMAGE_NT_SIGNATURE) return false;

        const IMAGE_DATA_DIRECTORY* dirs = nullptr;
        if (nt32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            dirs = nt32->OptionalHeader.DataDirectory;
        } else if (nt32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            auto nt64 = (const IMAGE_NT_HEADERS64*)nt32;
            dirs = nt64->OptionalHeader.DataDirectory;
        } else {
            return false;
        }

        importRva = dirs[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        delayRva  = dirs[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress;

        numSections = nt32->FileHeader.NumberOfSections;
        sections = (const IMAGE_SECTION_HEADER*)((const uint8_t*)&nt32->OptionalHeader +
                                                 nt32->FileHeader.SizeOfOptionalHeader);
        return true;
    }

    // Convert RVA to pointer inside `data`, or nullptr if invalid.
    const uint8_t* rva(DWORD r) const {
        for (WORD i = 0; i < numSections; ++i) {
            const auto& s = sections[i];
            DWORD size = std::max(s.Misc.VirtualSize, s.SizeOfRawData);
            if (r >= s.VirtualAddress && r < s.VirtualAddress + size) {
                size_t off = (size_t)s.PointerToRawData + (r - s.VirtualAddress);
                if (off < data.size()) return data.data() + off;
                return nullptr;
            }
        }
        return nullptr;
    }

    std::string stringAt(DWORD r) const {
        const uint8_t* p = rva(r);
        if (!p) return {};
        const uint8_t* end = data.data() + data.size();
        const uint8_t* q = p;
        while (q < end && *q) ++q;
        return std::string((const char*)p, (const char*)q);
    }

    std::vector<std::string> imports() const {
        std::vector<std::string> names;

        // Regular imports
        if (importRva) {
            auto d = (const IMAGE_IMPORT_DESCRIPTOR*)rva(importRva);
            while (d && d->Name) {
                std::string n = stringAt(d->Name);
                if (!n.empty()) names.push_back(n);
                ++d;
                if ((const uint8_t*)(d + 1) > data.data() + data.size()) break;
            }
        }

        // Delay-load imports (layout: DWORD attrs; DWORD rvaDllName; ...; 32 bytes total)
        if (delayRva) {
            const uint8_t* p = rva(delayRva);
            while (p && p + 32 <= data.data() + data.size()) {
                DWORD attrs = *(const DWORD*)p;
                DWORD nameRva = *(const DWORD*)(p + 4);
                if (!attrs && !nameRva) break;
                if (nameRva) {
                    std::string n = stringAt(nameRva);
                    if (!n.empty()) names.push_back(n);
                }
                p += 32;
            }
        }
        return names;
    }
};

// -------------------------------------------------------- DLL resolution

// Names that are virtual (API sets) and never exist as files.
static bool isApiSet(const std::string& lower) {
    return lower.rfind("api-ms-", 0) == 0 || lower.rfind("ext-ms-", 0) == 0;
}

// Look for a DLL: first beside the analysed file, then the normal Windows search
// (this tool's folder, system dirs, PATH).
static fs::path findDll(const std::string& name, const fs::path& originDir,
                        const std::vector<fs::path>& extraDirs) {
    std::error_code ec;
    fs::path local = originDir / name;
    if (fs::exists(local, ec)) return local;

    for (const fs::path& dir : extraDirs) {
        fs::path candidate = dir / name;
        if (fs::exists(candidate, ec)) return candidate;
    }

    wchar_t buf[MAX_PATH * 4];
    std::wstring wname(name.begin(), name.end());
    DWORD n = SearchPathW(nullptr, wname.c_str(), nullptr, (DWORD)std::size(buf), buf, nullptr);
    if (n > 0 && n < std::size(buf)) return fs::path(std::wstring(buf, n));
    return {};
}

// ------------------------------------------------------------------ main

int main() {
    // Read arguments as UTF-16 so paths with non-ASCII characters work.
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv || argc < 3) {
        std::cerr << "Usage: dllfinder <target.exe|dll> <destination folder> [extra search folder ...]\n";
        if (argv) LocalFree(argv);
        return 2;
    }

    const fs::path target = fs::absolute(fs::path(argv[1]));
    const fs::path destDir = fs::absolute(fs::path(argv[2]));
    std::vector<fs::path> extraDirs;
    for (int i = 3; i < argc; ++i) extraDirs.push_back(fs::absolute(fs::path(argv[i])));
    LocalFree(argv);

    if (!fs::is_regular_file(target)) {
        std::cerr << "Target file not found: " << target.string() << "\n";
        return 2;
    }

    std::error_code mkec;
    fs::create_directories(destDir, mkec);
    if (mkec) {
        std::cerr << "Could not create destination folder: " << destDir.string()
                  << " - " << mkec.message() << "\n";
        return 2;
    }

    const fs::path winDir = windowsDirectory();
    const fs::path originDir = target.parent_path();

    std::cout << "Analysing:  " << target.string() << "\n";
    std::cout << "Copying to: " << destDir.string() << "\n\n";

    std::set<std::string> visited;       // lower-case names already processed
    std::vector<fs::path> toCopy;
    std::vector<std::string> missing;
    std::vector<std::string> skippedSystem;

    std::queue<fs::path> work;
    work.push(target);
    bool first = true;

    while (!work.empty()) {
        fs::path current = work.front();
        work.pop();

        PeFile pe;
        if (!pe.load(current)) {
            if (first) {
                std::cerr << "Target is not a valid PE file: " << current.string() << "\n";
                return 2;
            }
            std::cerr << "Warning: could not parse " << current.string() << "\n";
            continue;
        }
        first = false;

        for (const std::string& name : pe.imports()) {
            std::string lower = toLower(name);
            if (!visited.insert(lower).second) continue;

            if (isApiSet(lower)) continue;

            fs::path found = findDll(name, originDir, extraDirs);
            if (found.empty()) {
                missing.push_back(name);
                continue;
            }

            if (isUnder(found, winDir)) {            // System32, SysWOW64, WinSxS...
                skippedSystem.push_back(name);
                continue;
            }

            toCopy.push_back(found);
            work.push(found);                         // recurse into its imports
        }
    }

    // Copy
    int copied = 0, failed = 0;
    for (const fs::path& src : toCopy) {
        fs::path dst = destDir / src.filename();
        std::error_code ec;
        if (fs::exists(dst, ec) && fs::equivalent(src, dst, ec)) {
            std::cout << "  [same]    " << src.filename().string() << "\n";
            continue;
        }
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cout << "  [FAILED]  " << src.filename().string() << " - " << ec.message() << "\n";
            ++failed;
        } else {
            std::cout << "  [copied]  " << src.string() << "\n";
            ++copied;
        }
    }

    std::cout << "\nSystem DLLs skipped (" << skippedSystem.size() << "): ";
    for (auto& s : skippedSystem) std::cout << s << " ";
    std::cout << "\n";

    if (!missing.empty()) {
        std::cout << "\nNOT FOUND (" << missing.size() << "):\n";
        for (auto& m : missing) std::cout << "  " << m << "\n";
        std::cout << "(Delay-loaded or optional DLLs may be legitimately absent.)\n";
    }

    std::cout << "\nDone. Copied " << copied << ", failed " << failed << ".\n";
    return failed ? 1 : 0;
}