#include "PTRenderer.hpp"
#include "../until/frame.hpp"
#include "../until/rng.hpp"

glm::vec3 PTRenderer::renderPixel(const glm::ivec3 &pixelCoord)
{
    // 让每个线程都有一个随机数生成器，确保线程之间不会因为共享一个随机数生成器而资源竞争，导致性能下降
    thread_local RNG rng{static_cast<size_t>(pixelCoord.x * 1000000 + pixelCoord.y + pixelCoord.z * 10000000)};
    auto ray = mCamera.generateRay(pixelCoord, {rng.uniform(), rng.uniform()});
    // 遍历路径上的每一个点
    glm::vec3 beta = {1, 1, 1}; // i=1, beta=1; i>1, beta=∏(brdf*cosθ/pdf)
    glm::vec3 L = {0, 0, 0};    // radiance
    float q = 0.9f;
    while (true)
    {
        auto hitInfo = mScene.intersect(ray);
        if (hitInfo.has_value())
        {
            // 如果是光源，直接累计，要在俄罗斯轮盘赌之前，防止光源上产生黑点
            L += beta * hitInfo->mMaterial->mEmission;
            if (rng.uniform() > q)
            {
                // Russian roulette, 保证递归不会一直进行下去的同时还保证蒙特卡洛积分的期望依旧不变
                break;
            }
            beta /= q;

            Frame frame(hitInfo->mNormal); // 构建局部坐标系
            glm::vec3 lightDirection;
            /* 立体角在半球上的积分为2π，pdf在半球上的积分为1
                1. 漫反射均匀采样半球方向，故pdf为1/(2π)常数，brdf=ρ/π
                2. 镜面反射有且只有一个出射光，故pdf为狄拉克分布，非反射方向为0，brdf=ρ/cosθ
            */

            if (hitInfo->mMaterial)
            {
                glm::vec3 viewDirection = frame.localFromWorld(-ray.mDirection);
                auto bsdf_sample = hitInfo->mMaterial->sampleBSDF(hitInfo->mHitPoint, viewDirection, rng);
                if (!bsdf_sample.has_value())
                {
                    break;
                }
                // 表面法线就是局部坐标系的y轴
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