#pragma once
#include "normalRenderer.hpp"
#include "../postProcess/rgb.hpp"

glm::vec3 NormalRenderer::renderPixel(const glm::ivec3 &pixelCoord)
{
    auto ray = mCamera.generateRay(pixelCoord);
    auto hitInfo = mScene.intersect(ray);
    if (hitInfo.has_value())
    {
        glm::ivec3 color = (hitInfo->mNormal * 0.5f + 0.5f) * 255.f;
        return RGB(color.r, color.g, color.b);
    }
    return {};
}