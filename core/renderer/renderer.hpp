#pragma once

#include "../camera/camera.hpp"
#include "../scene.hpp"

#define DEFINE_RENDERER(Name)                                                           \
    class Name##Renderer : public Renderer                                              \
    {                                                                                   \
    public:                                                                             \
        Name##Renderer(Camera &camera, const Scene &scene) : Renderer(camera, scene){}; \
                                                                                        \
    private:                                                                            \
        glm::vec3 renderPixel(const glm::ivec3 &pixelCoord) override;                   \
    };

class Renderer
{
    friend class Previewer;

public:
    Renderer(Camera &camera, const Scene &scene) : mCamera(camera), mScene(scene) {};
    void render(size_t spp, const std::filesystem::path &fileName);

private:
    virtual glm::vec3 renderPixel(const glm::ivec3 &pixelCoord) = 0;

protected:
    Camera &mCamera;
    const Scene &mScene;
};