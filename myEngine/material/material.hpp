#pragma once
#include "../until/rng.hpp"
#include <glm/glm.hpp>
#include <optional>

struct BSDFSample
{
    glm::vec3 bsdf;
    float pdf;
    glm::vec3 lightDirection;
};

class Material
{
public:
    virtual std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const = 0;
    void setEmission(const glm::vec3 &emission) { mEmission = emission; }

public:
    glm::vec3 mEmission{};
};