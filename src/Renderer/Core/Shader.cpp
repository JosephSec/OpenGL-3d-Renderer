#include <Renderer/Core/Shader.hpp>
using namespace Renderer;

#include <Renderer/Debug.hpp>
#include <Renderer/Hardware/GPU.hpp>

#include <utility>
#include <fstream>
#include <sstream>


//private
std::filesystem::path Shader::s_shaderFolder;
//private


Shader::Shader(const std::string &_name) : m_name(_name) {
  const std::vector<GLuint> shaders = GetShaders(_name);

  m_id = glCreateProgram();
  for(const GLuint shader : shaders) glAttachShader(m_id, shader);
  
  // glProgramParameteri(m_id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);

  glLinkProgram(m_id);

  for(const GLuint shader : shaders) {
    glDetachShader(m_id, shader);
    glDeleteShader(shader);
  }

  try { CheckLink(m_id); }
  catch(...) {
    glDeleteProgram(m_id);
    LogError("[SHADER ERROR]: failed to link program");
  }
  
  m_uniformLocations = CacheUniforms(m_id);

  GPU::VRAMInUse += GPU::GetShaderVRAMFootprint(m_id);
}
Shader::~Shader() {
  if(m_id != 0) glDeleteProgram(m_id);
}

Shader::Shader(Shader &&_o) noexcept
: m_name(std::move(_o.m_name)),
  m_id(std::exchange(_o.m_id, 0)),
  m_uniformLocations(std::move(_o.m_uniformLocations)) {}
Shader &Shader::operator=(Shader &&_o) noexcept {
  if(this != &_o) {
    if(m_id != 0) glDeleteProgram(m_id);

    m_name             = std::move(_o.m_name);
    m_id               = std::exchange(_o.m_id, 0);
    m_uniformLocations = std::move(_o.m_uniformLocations);
  }
  return *this;
}


void Shader::use() const noexcept {
  glUseProgram(m_id);
}

void Shader::setUniform(UniformLocation _loc, const glm::mat3 &_matrix) const {
  if(_loc.id != -1) glUniformMatrix3fv(_loc.id, 1, GL_FALSE, glm::value_ptr(_matrix));
}
void Shader::setUniform(UniformLocation _loc, const glm::mat4 &_matrix) const {
  if(_loc.id != -1) glUniformMatrix4fv(_loc.id, 1, GL_FALSE, glm::value_ptr(_matrix));
}
void Shader::setUniform(UniformLocation _loc, bool _val) const {
  if(_loc.id != -1) glUniform1i(_loc.id, _val);
}
void Shader::setUniform(UniformLocation _loc, unsigned int _val) const {
  if(_loc.id != -1) glUniform1ui(_loc.id, _val);
}
void Shader::setUniform(UniformLocation _loc, int _val) const {
  if(_loc.id != -1) glUniform1i(_loc.id, _val);
}
void Shader::setUniform(UniformLocation _loc, float _val)const {
  if(_loc.id != -1) glUniform1f(_loc.id, _val);
}
void Shader::setUniform(UniformLocation _loc, const glm::vec2 _vec) const {
  if(_loc.id != -1) glUniform2f(_loc.id, _vec.x,_vec.y);
}
void Shader::setUniform(UniformLocation _loc, const glm::vec3 _vec) const {
  if(_loc.id != -1) glUniform3f(_loc.id, _vec.x,_vec.y,_vec.z);
}
void Shader::setUniform(UniformLocation _loc, const glm::vec4 _vec) const {
  if(_loc.id != -1) glUniform4f(_loc.id, _vec.x,_vec.y,_vec.z,_vec.w);
}
void Shader::setUniform(UniformLocation _loc, const sf::Color _clr) const {
  if(_loc.id != -1) glUniform4f(_loc.id, _clr.r/255.0f, _clr.g/255.0f, _clr.b/255.0f, _clr.a/255.0f);
}


Shader::UniformLocation Shader::getUniform(const std::string &_name) const noexcept {
  const auto it = m_uniformLocations.find(_name);
  return {(it != m_uniformLocations.end())? it->second : -1};
}



std::vector<GLuint> Shader::GetShaders(const std::string &_name) {
  if(std::filesystem::exists(s_shaderFolder/_name) == false) {
    LogError(std::format("[SHADER ERROR]: Shader folder not found: ({})\n", _name));
  }

  std::vector<GLuint> shaders;
  shaders.reserve(3);

  const std::vector<std::pair<GLuint, std::filesystem::path>> shaderTypes = {
    {GL_VERTEX_SHADER,   s_shaderFolder/_name/(_name + ".vert")},
    {GL_FRAGMENT_SHADER, s_shaderFolder/_name/(_name + ".frag")},
    {GL_COMPUTE_SHADER,  s_shaderFolder/_name/(_name + ".comp")},
  };

  for(const auto &[type, filePath] : shaderTypes) {
    if(std::filesystem::exists(filePath) == false) continue;

    std::ifstream file(filePath);
    if(file.is_open() == false) {
      std::cout << "[SHADER ERROR]: Shader file is missing or can't be opened (" + filePath.string() + ")\n";
      continue;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    const std::string str = ss.str();
    shaders.push_back(Compile(type, str.c_str()));
  }

  return shaders;
}
GLuint Shader::Compile(GLenum _type, const char *_src) {
  GLuint shaderID = glCreateShader(_type); // Renamed from m_id to avoid confusion
  glShaderSource(shaderID, 1, &_src, nullptr);
  glCompileShader(shaderID);

  GLint success;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
  if(!success) {
    char log[512];
    // Zero-initialize the buffer to prevent string conversion from reading out of bounds
    std::fill(std::begin(log), std::end(log), '\0'); 
    
    glGetShaderInfoLog(shaderID, 511, nullptr, log); // Leave the last byte as safe null terminator
    glDeleteShader(shaderID);
    
    LogError("[SHADER ERROR]: Failed to compile shader.\n\tLog: " + std::string(log) + '\n');
  }
  return shaderID;
}

std::unordered_map<std::string, GLint> Shader::CacheUniforms(GLuint _program) {
  std::unordered_map<std::string, GLint> uniformLocations;

  GLint linked = GL_FALSE;
  glGetProgramiv(_program, GL_LINK_STATUS, &linked);
  if(linked == GL_FALSE) return uniformLocations;


  GLint activeUniformsCount = 0;
  glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &activeUniformsCount);

  GLint maxNameLength = 0;
  glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

  if(activeUniformsCount <= 0 || maxNameLength <= 0) return uniformLocations;


  std::vector<GLchar> nameBuffer(static_cast<size_t>(maxNameLength));
  uniformLocations.reserve(static_cast<size_t>(activeUniformsCount));

  constexpr std::string_view kArrayPrefix = "[0]";

  for(GLuint i = 0; i < static_cast<GLuint>(activeUniformsCount); i++) {
    GLsizei length = 0;
    GLsizei size = 0;
    GLenum type = GL_NONE;

    glGetActiveUniform(_program, i, maxNameLength, &length, &size, &type, nameBuffer.data());

    std::string name(nameBuffer.data(), static_cast<size_t>(length));

    const GLint location = glGetUniformLocation(_program, name.c_str());
    if(location == -1) continue;

    if(name.size() > kArrayPrefix.size() && name.compare(name.size() - kArrayPrefix.size(), kArrayPrefix.size(), kArrayPrefix) == 0) {
      const std::string base = name.substr(0, name.size() - kArrayPrefix.size());

      uniformLocations[base] = location;
      for(GLint element = 0; element < size; element++) {
        std::string elementName = base + "[" + std::to_string(element) + "]";

        const GLint elementLocation = glGetUniformLocation(_program, elementName.c_str());
        if(elementLocation != -1) uniformLocations[std::move(elementName)] = elementLocation;
      }
    }
    else uniformLocations[std::move(name)] = location;
  }

  return uniformLocations;
}

void Shader::CheckLink(GLuint _program) {
  GLint linked = GL_FALSE;
  glGetProgramiv(_program, GL_LINK_STATUS, &linked);
  if(linked == GL_TRUE) return;

  GLint logLength = 0;
  glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &logLength);
  
  if(logLength <= 0) logLength = 512; 
  
  std::string log(static_cast<size_t>(logLength), '\0'); 
  glGetProgramInfoLog(_program, logLength, nullptr, log.data());

  LogError("[SHADER ERROR]: Failed to link program.\n\tLog: " + log + '\n');
}