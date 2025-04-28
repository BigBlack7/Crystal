#include "diffuseMaterial.hpp"
#include "../sample/spherical.hpp"

std::optional<BSDFSample> DiffuseMaterial::sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const
{
    glm::vec3 lightDirection = CosineSampleHemisphere({rng.uniform(), rng.uniform()});
    float pdf = CosineSampleHemispherePdf(lightDirection);
    glm::vec3 bsdf = mAlbedo / PI;
    return BSDFSample{bsdf, pdf, lightDirection};
}
