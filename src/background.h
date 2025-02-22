#pragma once
#include "model.h"
#include "camera.h"
#include "../external/glew/include/GL/glew.h"

class Background {
public:
    Background(const char* path);
    void Draw(const Camera &camera);

private:
    Model model;
    float scale = 1.0f;
    Shader shader;
}; 