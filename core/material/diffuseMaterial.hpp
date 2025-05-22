#pragma once
#include "material.hpp"

class DiffuseMaterial : public Material
{
public:
    DiffuseMaterial(const glm::vec3 &albedo) : mAlbedo(albedo) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    glm::vec3 mAlbedo{};
};