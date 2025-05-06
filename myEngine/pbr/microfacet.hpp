#pragma once
#include <glm/glm.hpp>
#include "../until/rng.hpp"
class Microfacet
{
    // smith models 史密斯模型
    // GGX Distribution GGX分布
    // stretching invariance 伸缩不变性
public:
    Microfacet(float alpha_x, float alpha_z);
    float normalDistribution(const glm::vec3 &microfacetNormal) const;                                                                             // 法线分布函数
    float masking(const glm::vec3 &viewDirection, const glm::vec3 &microfacetNormal) const;                                                        // 掩蔽函数
    float heightCorrelatedMaskShadowing(const glm::vec3 &lightDirection, const glm::vec3 &viewDirection, const glm::vec3 &microfacetNormal) const; // 高度相关阴影掩蔽函数
    bool isDeltaDistribution() const;                                                                                                              // 是否为delta分布, 即根据粗糙度判断是否为镜面反射

    // 可见法线分布函数
    float visibleNormalDistribution(const glm::vec3 &viewDirection, const glm::vec3 &microfacetNormal) const;
    // 采样可见法线
    glm::vec3 sampleVisibleNormal(const glm::vec3 &viewDirection, const RNG &rng) const;
private:
    float slopeDistribution(const glm::vec2 &slope) const; // 斜率分布函数
    float lambda(const glm::vec3 &directionUpper) const;

private:
    // 各项异性粗糙度
    float mAlphaX{};
    float mAlphaZ{};
};