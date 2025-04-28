#include "groundMaterial.hpp"
#include "../sample/spherical.hpp"

std::optional<BSDFSample> GroundMaterial::sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const
{
    glm::vec3 lightDirection = CosineSampleHemisphere({rng.uniform(), rng.uniform()});
    float pdf = CosineSampleHemispherePdf(lightDirection);
    glm::vec3 bsdf = mAlbedo / PI;
    if (static_cast<int>(glm::floor(hitPoint.x * 8)) % 8 == 0 || static_cast<int>(glm::floor(hitPoint.z * 8)) % 8 == 0)
    {
        bsdf *= 0.1;
    }
    return BSDFSample{bsdf, pdf, lightDirection};
}
