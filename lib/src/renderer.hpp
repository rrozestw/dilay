#ifndef DILAY_RENDERER
#define DILAY_RENDERER

#include <glm/fwd.hpp>
#include "configurable.hpp"
#include "macro.hpp"

class Color;
class Config;
enum  class RenderMode;

class Renderer : public Configurable {
  public:
    DECLARE_BIG3 (Renderer, const Config&)

    void setupRendering       ();
    void setProgram           (RenderMode);
    void setMvp               (const float*);
    void setModel             (const float*);
    void setColor3            (const Color&);
    void setColor4            (const Color&);
    void setWireframeColor3   (const Color&);
    void setWireframeColor4   (const Color&);
    void setAmbient           (const Color&);
    void setEyePoint          (const glm::vec3&);
    void setLightPosition     (unsigned int, const glm::vec3&);
    void setLightColor        (unsigned int, const Color&);
    void setLightIrradiance   (unsigned int, float);
    void updateLights         (const glm::mat4x4&);

    static bool requiresGeometryShader  (RenderMode);
    static bool requiresNormalAttribute (RenderMode);

  private:
    IMPLEMENTATION

    void runFromConfig (const Config&);
};

#endif
