#pragma once
#include "normalRenderer.hpp"

glm::vec3 NormalRenderer::renderPixel(const glm::ivec3 &pixelCoord)
{
    auto ray = mCamera.generateRay(pixelCoord);
    auto hitInfo = mScene.intersect(ray);
    if (hitInfo.has_value()) 
    {
        return hitInfo->mNormal * 0.5f + 0.5f;
    }
    return {};
}