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
    if (mIor == 1.f) // 空气
    {
        // 不会产生折射，即法线方向不会影响光线方向，就不存在Jakob矩阵
        return BSDFSample{mAlbedoT / glm::abs(viewDirection.y), 1, -viewDirection};
    }

    float etai_div_etat = mIor; // 从入射介质到透射介质的折射率
    glm::vec3 microfacetNormal = {0, 1, 0};
    if (!mMicrofacet.isDeltaDistribution())
    {
        microfacetNormal = mMicrofacet.sampleVisibleNormal(viewDirection, rng);
    }

    float cos_theta_t = viewDirection.y; // 观察方向与法线的夹角
    float reversal = 1.f;                // 反转系数，用于调整法线方向
    if (cos_theta_t < 0)                 // 判断观察点是否在介质内部并做相应的调整
    {
        etai_div_etat = 1.f / mIor;
        reversal = -1.f;
        cos_theta_t = -cos_theta_t;
    }

    float cos_theta_i;
    float fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

    // 根据Fresnel值决定是反射还是折射
    if (rng.uniform() <= fr)
    {
        // 反射
        glm::vec3 lightDirection = -viewDirection + 2.f * glm::dot(microfacetNormal, viewDirection) * microfacetNormal;
        // 返回 BSDF 样本，包含反射率、概率密度和反射方向
        if (mMicrofacet.isDeltaDistribution())
        {
            // 镜面反射
            return BSDFSample{mAlbedoR / glm::abs(lightDirection.y), 1, lightDirection};
        }
        glm::vec3 brdf = mAlbedoR * mMicrofacet.normalDistribution(microfacetNormal) * mMicrofacet.heightCorrelatedMaskShadowing(lightDirection, viewDirection, microfacetNormal) / glm::abs(4.f * lightDirection.y * viewDirection.y);
        float pdf = mMicrofacet.visibleNormalDistribution(viewDirection, microfacetNormal) / glm::abs(4.f * glm::dot(viewDirection, microfacetNormal));
        return BSDFSample{brdf, pdf, lightDirection};
    }
    else
    {
        // 透射，透射得到的方向需要用snell定律计算
        // 因为BTDF的非对称性，Lo = Lt * (ηo/ηi)^2，似乎被丢弃了但是这里加上
        glm::vec3 lightDirection{(-viewDirection / etai_div_etat) + (cos_theta_t / etai_div_etat - cos_theta_i) * reversal * microfacetNormal};

        // 折射的Jakob行列式
        float det_J = etai_div_etat * etai_div_etat * glm::abs(glm::dot(lightDirection, microfacetNormal)) / glm::pow(glm::abs(glm::dot(viewDirection, microfacetNormal)) - etai_div_etat * etai_div_etat * glm::abs(glm::dot(lightDirection, microfacetNormal)), 2);
        // BTDF
        glm::vec3 btdf = mAlbedoT * det_J * mMicrofacet.normalDistribution(microfacetNormal) * mMicrofacet.heightCorrelatedMaskShadowing(lightDirection, viewDirection, microfacetNormal) * glm::abs(glm::dot(viewDirection, microfacetNormal) / (lightDirection.y * viewDirection.y));
        // pdf
        float pdf = mMicrofacet.visibleNormalDistribution(viewDirection, microfacetNormal) * det_J;

        return BSDFSample{btdf / (etai_div_etat * etai_div_etat), pdf, lightDirection};
        // return BSDFSample{mAlbedoT / glm::abs(lightDirection.y), 1, lightDirection};
    }
}
