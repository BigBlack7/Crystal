#pragma once
#include "material.hpp"

class ConductorMaterial : public Material
{
public:
    ConductorMaterial(const glm::vec3 &ior, const glm::vec3 &k) : mIor(ior), k(k) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    glm::vec3 mIor, k; // 导体的折射率和吸收系数，导体的菲涅尔系数为向量形式，因为三个通道值不一样，而电介质的菲涅尔系数为标量形式，因为三个通道值一样
};