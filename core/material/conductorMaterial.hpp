#pragma once
#include "material.hpp"
#include "../pbr/microfacet.hpp"

class ConductorMaterial : public Material
{
public:
    ConductorMaterial(const glm::vec3 &ior, const glm::vec3 &k, float alphaX = 0, float alphaZ = 0) : mIor(ior), k(k), mMicrofacet(alphaX, alphaZ) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    glm::vec3 mIor, k;      // 导体的折射率和吸收系数，导体的菲涅尔系数为向量形式，因为三个通道值不一样，而电介质的菲涅尔系数为标量形式，因为三个通道值一样
    Microfacet mMicrofacet; // 当粗糙度较低时，余弦重要性采样效果就会很不好。需要一个与被积函数拟合且能包含被积函数定义域的概率密度函数
};