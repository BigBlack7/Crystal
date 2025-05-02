#pragma once
#include "../until/rng.hpp"
#include <glm/glm.hpp>
#include <optional>

struct BSDFSample // 采样样本
{
    glm::vec3 bsdf;
    float pdf;
    glm::vec3 lightDirection;
};

class Material
{
public:
    // 根据观察方向采样brdf，选择brdf形状相似的pdf
    // BSDF = BRDF + BTDF
    virtual std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const = 0;
    void setEmission(const glm::vec3 &emission) { mEmission = emission; }

public:
    glm::vec3 mEmission{};
};