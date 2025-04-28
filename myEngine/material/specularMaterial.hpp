#pragma once
#include "material.hpp"

class SpecularMaterial : public Material
{
public:
    SpecularMaterial(const glm::vec3 &albedo) : mAlbedo(albedo) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    glm::vec3 mAlbedo{};
};