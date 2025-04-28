#include "conductorMaterial.hpp"
#include "../until/complex.hpp"

std::optional<BSDFSample> ConductorMaterial::sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const
{
    glm::vec3 fr{};
    for (int i = 0; i < 3; i++)
    {
        Complex etat_div_etai{mIor[i], k[i]};
        float cos_theta_i = glm::clamp(viewDirection.y, 0.f, 1.f);
        float sin2_theta_i = 1.f - cos_theta_i * cos_theta_i;
        Complex sin2_theta_t = sin2_theta_i / (etat_div_etai * etat_div_etai);
        Complex cos_theta_t = sqrt(1.f - sin2_theta_t);
        Complex r_parl = (etat_div_etai * cos_theta_i - cos_theta_t) / (etat_div_etai * cos_theta_i + cos_theta_t);
        Complex r_perp = (cos_theta_i - etat_div_etai * cos_theta_t) / (cos_theta_i + etat_div_etai * cos_theta_t);

        fr[i] = 0.5f * (norm(r_parl) + norm(r_perp));
    }
    glm::vec3 lightDirection{-viewDirection.x, viewDirection.y, -viewDirection.z};
    return BSDFSample{fr / glm::abs(lightDirection.y), 1, lightDirection};
}
