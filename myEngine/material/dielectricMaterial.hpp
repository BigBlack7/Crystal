#pragma once
#include "material.hpp"

class DielectricMaterial : public Material 
{
public:
    DielectricMaterial(float ior, const glm::vec3 &albedo) : mIor(ior), mAlbedoR(albedo), mAlbedoT(albedo) {}
    DielectricMaterial(float ior, const glm::vec3 &albedoR, const glm::vec3 &albedoT) : mIor(ior), mAlbedoR(albedoR), mAlbedoT(albedoT) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const override;

private:
    float mIor;
    glm::vec3 mAlbedoR, mAlbedoT;
};