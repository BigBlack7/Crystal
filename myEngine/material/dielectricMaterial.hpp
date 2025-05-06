#pragma once
#include "material.hpp"
#include "../pbr/microfacet.hpp"

class DielectricMaterial : public Material
{
public:
    DielectricMaterial(float ior, const glm::vec3 &albedo, float alphaX = 0, float alphaZ = 0) : mIor(ior), mAlbedoR(albedo), mAlbedoT(albedo), mMicrofacet(alphaX, alphaZ) {}
    DielectricMaterial(float ior, const glm::vec3 &albedoR, const glm::vec3 &albedoT, float alphaX = 0, float alphaZ = 0) : mIor(ior), mAlbedoR(albedoR), mAlbedoT(albedoT), mMicrofacet(alphaX, alphaZ) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    float mIor;                   // 折射率越大反射越多，透射越少
    glm::vec3 mAlbedoR, mAlbedoT; // 反射和折射的反照率
    Microfacet mMicrofacet;       // 当粗糙度较低时，余弦重要性采样效果就会很不好。需要一个与被积函数拟合且能包含被积函数定义域的概率密度函数
};