#include "dielectricMaterial.hpp"

// L = Fr * Lr + Ft * Lt 会导致递归爆炸
// L' = [(Lr, with prob Fr); (Lt, with prob Ft)]
// 平行 r∥ = Er / Ei = ηt * cosθi - ηi * cosθt / ηt * cosθi + ηi * cosθt
// 垂直 r⊥ = Er / Ei = (ηi * cosθi - ηt * cosθt) / (ηi * cosθi + ηt * cosθt)
// Fr = (r∥^2 + r⊥^2) / 2
// Ft = 1 - Fr
// i和t的物理意义搞反了，或许是认为光线是视线的反转
float Fresnel(float etai_div_etat, float cos_theta_t, float &cos_theta_i)
{
    // etai_div_etat = ηi(透射方向介质) / ηt(视线方向介质)
    // cos_theta_t = cosθt 视线方向
    // cos_theta_i = cosθi 透射方向
    // 计算sinθt和sinθi的平方
    // snell定律 ηi * sinθi = ηt * sinθt
    float sin2_theta_t = 1 - cos_theta_t * cos_theta_t;
    float sin2_theta_i = sin2_theta_t / (etai_div_etat * etai_div_etat);
    // 全反射只会发生在IOR较大的介质向IOR较小的介质传输
    if (sin2_theta_i >= 1) // 视线与法线夹角大于等于90度，全反射
    {
        return 1;
    }
    // cosθi = sqrt(1 - sinθi^2)
    cos_theta_i = glm::sqrt(1 - sin2_theta_i);
    // 计算r∥和r⊥
    float r_parl = (cos_theta_i - etai_div_etat * cos_theta_t) / (cos_theta_i + etai_div_etat * cos_theta_t);
    float r_perp = (etai_div_etat * cos_theta_i - cos_theta_t) / (etai_div_etat * cos_theta_i + cos_theta_t);
    return 0.5f * (r_parl * r_parl + r_perp * r_perp);
}

std::optional<BSDFSample> DielectricMaterial::sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const
{
    float etai_div_etat = mIor; // 从入射介质到透射介质的折射率
    glm::vec3 normal = {0, 1, 0};
    float cos_theta_t = viewDirection.y; // 观察方向与法线的夹角
    if (cos_theta_t < 0)                 // 判断观察点是否在介质内部并做相应的调整
    {
        etai_div_etat = 1.f / mIor;
        normal = {0, -1, 0};
        cos_theta_t = -cos_theta_t;
    }

    float cos_theta_i;
    float fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

    // 根据Fresnel值决定是反射还是折射
    if (rng.uniform() < fr)
    {
        // 反射
        glm::vec3 lightDirection{-viewDirection.x, viewDirection.y, -viewDirection.z};
        return BSDFSample{mAlbedoR / glm::abs(lightDirection.y), 1, lightDirection};
    }
    else
    {
        // 透射，透射得到的方向需要用snell定律计算
        // 因为BTDF的非对称性，Lo = Lt * (ηo/ηi)^2，似乎被丢弃了但是这里加上
        glm::vec3 lightDirection{(-viewDirection / etai_div_etat) + (cos_theta_t / etai_div_etat - cos_theta_i) * normal};
        // return BSDFSample{mAlbedoT / (glm::abs(lightDirection.y) * etai_div_etat * etai_div_etat), 1, lightDirection};
        return BSDFSample{mAlbedoT / glm::abs(lightDirection.y), 1, lightDirection};
    }
}
