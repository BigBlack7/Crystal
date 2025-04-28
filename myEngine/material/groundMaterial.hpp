#pragma once
#include "material.hpp"
class GroundMaterial : public Material
{
public:
    GroundMaterial(const glm::vec3 &albedo) : mAlbedo(albedo) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    glm::vec3 mAlbedo{};
};