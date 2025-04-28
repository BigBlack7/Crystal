#include "dielectricMaterial.hpp"

float Fresnel(float etai_div_etat, float cos_theta_t, float &cos_theta_i)
{
    float sin2_theta_t = 1 - cos_theta_t * cos_theta_t;
    float sin2_theta_i = sin2_theta_t / (etai_div_etat * etai_div_etat);
    if (sin2_theta_i >= 1)
    {
        return 1;
    }
    cos_theta_i = glm::sqrt(1 - sin2_theta_i);
    float r_parl = (cos_theta_i - etai_div_etat * cos_theta_t) / (cos_theta_i + etai_div_etat * cos_theta_t);
    float r_perp = (etai_div_etat * cos_theta_i - cos_theta_t) / (etai_div_etat * cos_theta_i + cos_theta_t);
    return 0.5f * (r_parl * r_parl + r_perp * r_perp);
}

std::optional<BSDFSample> DielectricMaterial::sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const
{
    float etai_div_etat = mIor;
    glm::vec3 normal = {0, 1, 0};
    float cos_theta_t = viewDirection.y;
    if (cos_theta_t < 0)
    {
        etai_div_etat = 1.f / mIor;
        normal = {0, -1, 0};
        cos_theta_t = -cos_theta_t;
    }
    float cos_theta_i;
    float fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

    if (rng.uniform() < fr)
    {
        glm::vec3 lightDirection{-viewDirection.x, viewDirection.y, -viewDirection.z};
        return BSDFSample{mAlbedoR / glm::abs(lightDirection.y), 1, lightDirection};
    }
    else
    {
        glm::vec3 lightDirection{(-viewDirection / etai_div_etat) + (cos_theta_t / etai_div_etat - cos_theta_i) * normal};
        return BSDFSample{mAlbedoT / glm::abs(lightDirection.y), 1, lightDirection};
    }
}
