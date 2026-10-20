#include <Renderer/Hardware/Monitor.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>

#include <windows.h>


int Monitor::RefreshRate = 0;

int Monitor::GetCurrentMonitorRefreshRate() {
  // Get the raw Win32 HWND handle from SFML
  HWND hwnd = (HWND)Core::window->getNativeHandle();

  // Find the monitor handle intersecting the window 
  // MONITOR_DEFAULTTOPRIMARY acts as a fallback if the window is off-screen
  HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);

  MONITORINFOEX monitorInfo;
  monitorInfo.cbSize = sizeof(MONITORINFOEX);

  if(GetMonitorInfo(hMonitor, &monitorInfo)) {
    DEVMODE dm = { 0 };
    dm.dmSize = sizeof(dm);
    
    // Pass the specific device name (e.g. "\\.\DISPLAY1") instead of NULL
    if(EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
      return static_cast<int>(dm.dmDisplayFrequency);
    }
  }
  return 60; 
}