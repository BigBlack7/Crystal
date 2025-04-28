#pragma once
#include "material.hpp"

class ConductorMaterial : public Material
{
public:
    ConductorMaterial(const glm::vec3 &ior, const glm::vec3 &k) : mIor(ior), k(k) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    glm::vec3 mIor, k;
};