#include "microfacet.hpp"
#include "../sample/spherical.hpp"
#include "../until/frame.hpp"

Microfacet::Microfacet(float alpha_x, float alpha_z)
{
    // 粗糙度在分母中存在，避免分母为0
    this->mAlphaX = glm::clamp(alpha_x * alpha_x, 1e-3f, 1.f);
    this->mAlphaZ = glm::clamp(alpha_z * alpha_z, 1e-3f, 1.f);
}

float Microfacet::normalDistribution(const glm::vec3 &microfacetNormal) const
{
    // 计算微平面的斜率
    glm::vec2 slope{-microfacetNormal.x / microfacetNormal.y, -microfacetNormal.z / microfacetNormal.y};
    // 将拉伸后的斜率转换为原始斜率
    slope.x /= mAlphaX;
    slope.y /= mAlphaZ;
    // 用原始斜率计算斜率分布函数
    float slopeDistribution = this->slopeDistribution(slope) / (mAlphaX * mAlphaZ);
    // 根据Jakob行列式计算法线分布函数
    return slopeDistribution / glm::pow(microfacetNormal.y, 4.f);
}

float Microfacet::masking(const glm::vec3 &viewDirection, const glm::vec3 &microfacetNormal) const
{
    // 掩蔽函数
    // 将观察方向翻转到微表面上方
    glm::vec3 viewDirectionUpper = viewDirection.y > 0.f ? viewDirection : -viewDirection;
    if (glm::dot(viewDirectionUpper, microfacetNormal) <= 0.f) // X+(O+·M)函数
    {
        return 0.f;
    }
    return 1.f / (1.f + this->lambda(viewDirectionUpper));
}

float Microfacet::heightCorrelatedMaskShadowing(const glm::vec3 &lightDirection, const glm::vec3 &viewDirection, const glm::vec3 &microfacetNormal) const
{
    // 掩蔽阴影函数
    // 将光源方向翻转到微表面上方
    glm::vec3 lightDirectionUpper = lightDirection.y > 0.f ? lightDirection : -lightDirection;
    if (glm::dot(lightDirectionUpper, microfacetNormal) <= 0.f) // X+(O+·M)函数
    {
        return 0.f;
    }
    // 将观察方向翻转到微表面上方
    glm::vec3 viewDirectionUpper = viewDirection.y > 0.f ? viewDirection : -viewDirection;
    if (glm::dot(viewDirectionUpper, microfacetNormal) <= 0.f) // X+(O+·M)函数
    {
        return 0.f;
    }
    return 1.f / (1.f + this->lambda(viewDirectionUpper) + this->lambda(lightDirectionUpper));
}

bool Microfacet::isDeltaDistribution() const
{
    return glm::max(mAlphaX, mAlphaZ) == 1e-3f; // 避免分母为0而设置的阈值，代表足够光滑
}

float Microfacet::visibleNormalDistribution(const glm::vec3 &viewDirection, const glm::vec3 &microfacetNormal) const
{
    glm::vec3 viewDirectionUpper = viewDirection.y > 0.f ? viewDirection : -viewDirection;
    float cos_theta_o = glm::dot(viewDirectionUpper, microfacetNormal);
    if (cos_theta_o <= 0.f) // X+(O+·M)函数
    {
        return 0.f;
    }
    // 可见法线分布函数
    return this->normalDistribution(microfacetNormal) * cos_theta_o * this->masking(viewDirection, microfacetNormal) / glm::abs(viewDirection.y);
}

glm::vec3 Microfacet::sampleVisibleNormal(const glm::vec3 &viewDirection, const RNG &rng) const
{
    // 将观察方向映射到球体坐标系下
    glm::vec3 viewDirectionUpper = viewDirection.y > 0.f ? viewDirection : -viewDirection;
    glm::vec3 viewDirectionHemi = glm::normalize(glm::vec3(mAlphaX * viewDirectionUpper.x, viewDirectionUpper.y, mAlphaZ * viewDirectionUpper.z));
    // 均匀采样单位圆，根据投影关系将采样点映射到对应的投影面积当中
    glm::vec2 sample = UniformSampleUnitDisk({rng.uniform(), rng.uniform()});
    float h = glm::sqrt(1.f - sample.x * sample.x);
    float t = 0.5f * (1.f + viewDirectionHemi.y);
    sample.y = t * sample.y + (1.f - t) * h;
    // 以球体坐标系下的观察方向为参考y轴建立局部坐标系，随后把采样点所对应的法线从局部坐标系转换到球体坐标系下
    Frame frame(viewDirectionHemi);
    glm::vec3 microfacetNormalHemi = frame.worldFromLocal({sample.x, glm::sqrt(1.f - sample.x * sample.x - sample.y * sample.y), sample.y});
    // 将球体坐标系下的法线转换到椭球坐标系下
    return glm::normalize(glm::vec3(mAlphaX * microfacetNormalHemi.x, microfacetNormalHemi.y, mAlphaZ * microfacetNormalHemi.z));
}

float Microfacet::slopeDistribution(const glm::vec2 &slope) const
{
    // 计算斜率分布函数
    return 1.f / (PI * glm::pow(1 + slope.x * slope.x + slope.y * slope.y, 2));
}

float Microfacet::lambda(const glm::vec3 &directionUpper) const
{
    if (directionUpper.y == 0.f) // 避免分母为0
    {
        return std::numeric_limits<float>::infinity(); // 对应掩蔽函数应该为0，故分母无穷大
    }
    float length2 = directionUpper.x * directionUpper.x + directionUpper.z * directionUpper.z;
    if (length2 == 0.f) // 当directionUpper.y为1时，即向xz平面投影长度平方为0，对应从正上方观看微平面
    {
        return 0.f; // 对应掩蔽函数应该为1，故对应lambda为0
    }
    float cos2_phi = directionUpper.x * directionUpper.x / length2;
    float sin2_phi = directionUpper.z * directionUpper.z / length2;
    float tan2_theta = length2 / (directionUpper.y * directionUpper.y);
    float alpha0_2 = mAlphaX * mAlphaX * cos2_phi + mAlphaZ * mAlphaZ * sin2_phi;
    return 0.5f * (glm::sqrt(1.f + alpha0_2 * tan2_theta) - 1.f);
}
