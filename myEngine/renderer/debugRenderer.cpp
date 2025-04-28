#include "debugRenderer.hpp"
#include "../postProcess/rgb.hpp"

glm::vec3 BoundsTestCountRenderer::renderPixel(const glm::ivec3 &pixelCoord)
{

#ifdef WITH_DEBUG_INFO
    auto ray = mCamera.generateRay(pixelCoord);
    mScene.intersect(ray);
    return RGB::GenerateHeatMap(ray.bounds_test_count / 150.f);
#else
    return {};
#endif
}

glm::vec3 TrianglesTestCountRenderer::renderPixel(const glm::ivec3 &pixelCoord)
{

#ifdef WITH_DEBUG_INFO
    auto ray = mCamera.generateRay(pixelCoord);
    mScene.intersect(ray);
    return RGB::GenerateHeatMap(ray.triangles_test_count / 7.f);
#else
    return {};
#endif
}