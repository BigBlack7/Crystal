#pragma once
#include "specularMaterial.hpp"
#include "../sample/spherical.hpp"

std::optional<BSDFSample> SpecularMaterial::sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const
{
    glm::vec3 lightDirection{-viewDirection.x, viewDirection.y, -viewDirection.z};
    glm::vec3 bsdf = mAlbedo / glm::abs(lightDirection.y);
    float pdf = 1;
    return BSDFSample{bsdf, pdf, lightDirection};
}
