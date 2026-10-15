#include <Renderer/Debug.hpp>
using namespace Renderer;

void Debug::dumpGLOnChange(const char *tag, GLuint postFbo) {
  using Fields = std::vector<std::pair<std::string, std::string>>;
  static std::map<std::string, Fields> last;
  Fields cur;

  auto I = [&](const std::string &k, long long v) { cur.emplace_back(k, std::to_string(v)); };
  auto H = [&](const std::string &k, unsigned v) {
    char b[16]; snprintf(b, sizeof b, "0x%x", v); cur.emplace_back(k, b);
  };
  auto F4 = [&](const std::string &k, const GLfloat *v) {
    char b[96]; snprintf(b, sizeof b, "%.3f,%.3f,%.3f,%.3f", v[0], v[1], v[2], v[3]);
    cur.emplace_back(k, b);
  };
  auto E = [&](const char *k, GLenum cap) { I(k, glIsEnabled(cap)); };

  // Pending errors (post code never checks glGetError, so surface them here)
  GLenum err; int errCount = 0;
  while ((err = glGetError()) != GL_NO_ERROR && errCount++ < 8) H("glError", err);

  GLint i, v4[4]; GLfloat f4[4]; GLfloat f2[2]; GLboolean b4[4], b1;

  // --- Framebuffer bindings / buffers ---
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &i); const GLint drawFbo = i; I("drawFbo", i);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &i); I("readFbo", i);
  glGetIntegerv(GL_DRAW_BUFFER, &i); H("drawBuffer0", i);
  glGetIntegerv(GL_READ_BUFFER, &i);  H("readBuffer", i);

  // --- Viewport / scissor / clear ---
  glGetIntegerv(GL_VIEWPORT, v4); I("vp.x", v4[0]); I("vp.y", v4[1]); I("vp.w", v4[2]); I("vp.h", v4[3]);
  glGetIntegerv(GL_SCISSOR_BOX, v4); I("sc.x", v4[0]); I("sc.y", v4[1]); I("sc.w", v4[2]); I("sc.h", v4[3]);
  glGetFloatv(GL_COLOR_CLEAR_VALUE, f4); F4("clearColor", f4);
  glGetFloatv(GL_DEPTH_CLEAR_VALUE, f4); { char b[32]; snprintf(b, sizeof b, "%.3f", f4[0]); cur.emplace_back("clearDepth", b); }
  glGetFloatv(GL_DEPTH_RANGE, f2); { char b[48]; snprintf(b, sizeof b, "%.3f,%.3f", f2[0], f2[1]); cur.emplace_back("depthRange", b); }

  // --- Depth / masks / raster state ---
  glGetIntegerv(GL_DEPTH_FUNC, &i); H("depthFunc", i);
  glGetBooleanv(GL_DEPTH_WRITEMASK, &b1); I("depthMask", b1);
  glGetBooleanv(GL_COLOR_WRITEMASK, b4); I("colorMask", b4[0] * 8 + b4[1] * 4 + b4[2] * 2 + b4[3]);
  GLint pm[2] = {0, 0}; glGetIntegerv(GL_POLYGON_MODE, pm); H("polyFront", pm[0]); H("polyBack", pm[1]);
  glGetIntegerv(GL_CULL_FACE_MODE, &i); H("cullMode", i);
  glGetIntegerv(GL_FRONT_FACE, &i); H("frontFace", i);
  E("depthTest", GL_DEPTH_TEST);   E("blend", GL_BLEND);          E("cull", GL_CULL_FACE);
  E("scissorTest", GL_SCISSOR_TEST); E("stencil", GL_STENCIL_TEST); E("srgb", GL_FRAMEBUFFER_SRGB);
  E("discard", GL_RASTERIZER_DISCARD); E("logicOp", GL_COLOR_LOGIC_OP);
  E("primRestart", GL_PRIMITIVE_RESTART); E("polyOffset", GL_POLYGON_OFFSET_FILL);
  E("a2c", GL_SAMPLE_ALPHA_TO_COVERAGE); E("multisample", GL_MULTISAMPLE);

  // --- Program / VAO / buffers ---
  glGetIntegerv(GL_CURRENT_PROGRAM, &i);               I("prog", i);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &i);          I("vao", i);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &i);          I("vbo", i);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &i);  I("ebo", i);

  // --- Texture units (temporarily switches active unit, then restores it) ---
  GLint active; glGetIntegerv(GL_ACTIVE_TEXTURE, &active); I("activeUnit", active - GL_TEXTURE0);
  for (int u = 0; u < 4; ++u) {
    glActiveTexture(GL_TEXTURE0 + u);
    std::string p = "u" + std::to_string(u) + ".";
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &i);  I(p + "tex2D", i);
    glGetIntegerv(GL_SAMPLER_BINDING, &i);     I(p + "sampler", i);
#ifdef DUMP_COMPAT_PROFILE   // fixed-function enable bit; GL_INVALID_ENUM on core profiles
    I(p + "ffTex2D", glIsEnabled(GL_TEXTURE_2D));
#endif
  }
  glActiveTexture(active);

  // --- Attachments of the currently bound draw FBO, and of the post FBO ---
  auto dumpFbo = [&](const std::string &p, GLuint fbo) {
    if (fbo == 0) return;
    GLint o = 0, t = 0;
    glGetNamedFramebufferAttachmentParameteriv(fbo, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &o);
    I(p + "color0", o);
    glGetNamedFramebufferAttachmentParameteriv(fbo, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &o);
    I(p + "depth", o);
    H(p + "status", glCheckNamedFramebufferStatus(fbo, GL_DRAW_FRAMEBUFFER));
  };
  dumpFbo("boundFbo.", (GLuint)drawFbo);
  if ((GLuint)drawFbo != postFbo) dumpFbo("postFbo.", postFbo);

  // --- Print: full dump the first time per tag, then only changed fields ---
  auto it = last.find(tag);
  if (it == last.end()) {
    printf("[%s] first dump:\n", tag);
    for (auto &kv : cur) printf("    %s = %s\n", kv.first.c_str(), kv.second.c_str());
  } else if (it->second != cur) {
    printf("[%s] changed:\n", tag);
    for (size_t k = 0; k < cur.size(); ++k) {
      const std::string old = (k < it->second.size() && it->second[k].first == cur[k].first)
                                ? it->second[k].second : "<n/a>";
      if (old != cur[k].second)
        printf("    %s: %s -> %s\n", cur[k].first.c_str(), old.c_str(), cur[k].second.c_str());
    }
  }
  last[tag] = cur;
}