#include "conductorMaterial.hpp"
#include "../until/complex.hpp"

glm::vec3 Fresnel(const glm::vec3 &ior, const glm::vec3 &k, float cos_theta_i)
{
    // 初始化反射率向量，每个分量对应 RGB 三个通道
    glm::vec3 fr{};
    // 对 RGB 三个通道分别计算菲涅尔反射系数
    for (int i = 0; i < 3; i++)
    {
        // 构建复数形式的相对折射率，实部为折射率，虚部为消光系数
        Complex etat_div_etai{ior[i], k[i]};
        // 计算入射角的余弦值，并将其限制在 [0, 1] 范围内
        // 局部坐标系表面法线方向为{0, 1, 0}，因此取viewDirection的y分量为入射角余弦值
        cos_theta_i = glm::clamp(cos_theta_i, 0.f, 1.f);
        // 计算入射角正弦值的平方
        float sin2_theta_i = 1.f - cos_theta_i * cos_theta_i;
        // 根据斯涅尔定律计算折射角正弦值的平方，由于导体的特殊性，使用复数运算
        Complex sin2_theta_t = sin2_theta_i / (etat_div_etai * etat_div_etai);
        // 计算折射角余弦值的复数形式
        Complex cos_theta_t = sqrt(1.f - sin2_theta_t);
        // 计算平行偏振分量的反射系数
        Complex r_parl = (etat_div_etai * cos_theta_i - cos_theta_t) / (etat_div_etai * cos_theta_i + cos_theta_t);
        // 计算垂直偏振分量的反射系数
        Complex r_perp = (cos_theta_i - etat_div_etai * cos_theta_t) / (cos_theta_i + etat_div_etai * cos_theta_t);

        // 计算该通道的菲涅尔反射系数，取两个偏振分量模的平均值
        fr[i] = 0.5f * (norm(r_parl) + norm(r_perp));
    }
    return fr;
}

std::optional<BSDFSample> ConductorMaterial::sampleBSDF(const glm::vec3 &hitPoint, const glm::vec3 &viewDirection, const RNG &rng) const
{
    glm::vec3 microfacetNormal{0, 1, 0};
    if (!mMicrofacet.isDeltaDistribution())
    {
        // 决定是否采样一个可见法线
        microfacetNormal = mMicrofacet.sampleVisibleNormal(viewDirection, rng);
    }
    // 根据微表面法线和观察方向计算菲涅尔反射率
    glm::vec3 fr = Fresnel(mIor, k, glm::abs(glm::dot(viewDirection, microfacetNormal)));

    // 计算反射光线的方向，表面法线方向不为 {0, 1, 0}
    glm::vec3 lightDirection = -viewDirection + 2.f * glm::dot(microfacetNormal, viewDirection) * microfacetNormal;
    // 返回 BSDF 样本，包含反射率、概率密度和反射方向
    if (mMicrofacet.isDeltaDistribution())
    {
        // 镜面反射
        return BSDFSample{fr / glm::abs(lightDirection.y), 1, lightDirection};
    }
    glm::vec3 brdf = fr * mMicrofacet.normalDistribution(microfacetNormal) * mMicrofacet.heightCorrelatedMaskShadowing(lightDirection, viewDirection, microfacetNormal) / glm::abs(4.f * lightDirection.y * viewDirection.y);
    float pdf = mMicrofacet.visibleNormalDistribution(viewDirection, microfacetNormal) / glm::abs(4.f * glm::dot(viewDirection, microfacetNormal));
    return BSDFSample{brdf, pdf, lightDirection};
}
