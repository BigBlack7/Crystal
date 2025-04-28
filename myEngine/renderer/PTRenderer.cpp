#include "PTRenderer.hpp"
#include "../until/frame.hpp"
#include "../until/rng.hpp"

glm::vec3 PTRenderer::renderPixel(const glm::ivec3 &pixelCoord)
{
    thread_local RNG rng{static_cast<size_t>(pixelCoord.x * 1000000 + pixelCoord.y + pixelCoord.z * 10000000)};
    auto ray = mCamera.generateRay(pixelCoord, {rng.uniform(), rng.uniform()});
    glm::vec3 beta = {1, 1, 1};
    glm::vec3 L = {0, 0, 0};
    float q = 0.9f;
    while (true)
    {
        auto hitInfo = mScene.intersect(ray);
        if (hitInfo.has_value())
        {
            L += beta * hitInfo->mMaterial->mEmission;
            if (rng.uniform() > q)
            {
                break;
            }
            beta /= q;

            Frame frame(hitInfo->mNormal);
            glm::vec3 lightDirection;
            if (hitInfo->mMaterial)
            {
                glm::vec3 viewDirection = frame.localFromWorld(-ray.mDirection);
                auto bsdf_sample = hitInfo->mMaterial->sampleBSDF(hitInfo->mHitPoint, viewDirection, rng);
                if (!bsdf_sample.has_value())
                {
                    break;
                }
                beta *= bsdf_sample->bsdf * glm::abs(bsdf_sample->lightDirection.y) / bsdf_sample->pdf;
                lightDirection = bsdf_sample->lightDirection;
            }
            else
            {
                break;
            }
            ray.mOrigin = hitInfo->mHitPoint;
            ray.mDirection = frame.worldFromLocal(lightDirection);
        }
        else
        {
            break;
        }
    }
    return L;
}