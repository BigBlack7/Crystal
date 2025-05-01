#pragma once
#include "../until/rng.hpp"
#include <glm/glm.hpp>

// 为蒙特卡洛积分器选择一个与被积函数的分布拟合的采样器
// 对于漫反射的均匀采样，pdf正比于ρcosθ/π
// 随机变量x服从pdf(x)的分布，函数y=T(x)，有Y=T(X)，则Y服从pdf(y)的分布
// 一维pdf(y) = pdf(x) / f'(x)
// 多维pdf(y) = pdf(x) / |J(x)| (雅可比矩阵是一阶偏导数矩阵)
const float PI = 3.14159265358979323846f;
inline glm::vec2 UniformSampleUnitDisk(const glm::vec2 &u)
{
    // 圆盘(s=πr^2)的pdf为1/π，积分为1
    // pdf(x,y) = 1/π --> pdf(r,θ) = r/π极坐标
    // T(r,θ)=(rcosθ,rsinθ)列向量转换到极坐标函数
    // J(r,θ) = [cosθ, -rsinθ; sinθ, rcosθ] <-- [rcosθ/dr, rcosθ/dθ; rsinθ/dr, rsinθ/dθ]
    // |J(x)| 行列式= rcosθ^2 + rsinθ^2 = r
    // pdf(x,y) = pdf(r,θ) / |J(r,θ)| = pdf(r,θ) / r
    // pdf(r,θ) = pdf(x,y) * |J(r,θ)| = r/π
    // pdf(r) = ∫(0->2π)pdf(r,θ)dθ = 2r; cdf(r) = ∫2rdr = r^2
    // pdf(θ|r) = pdf(r,θ) / pdf(r) = 1/2π; cdf(θ|r) = ∫pdf(θ|r)dθ = θ/2π
    // // 逆变换采样，对cdf函数值k进行采样，x=cdf^-1(k)
    // k1 = cdf(r) = r^2; r = sqrt(k1)
    // k2 = cdf(θ|r) = θ/2π; θ = 2πk2
    float r = glm::sqrt(u.x);
    float theta = 2.f * PI * u.y;
    // 极坐标转换到笛卡尔坐标T(r,θ)=(rcosθ,rsinθ)
    return {r * glm::cos(theta), r * glm::sin(theta)};
}

inline glm::vec3 CosineSampleHemisphere(const glm::vec2 &u)
{
    // 余弦重要性采样pdf正比于cosθ，设pdf(w) = c*cosθ，w为微分立体角，c = 1/π
    // pdf(w) = pdf(θ,φ)/(dw/dθdφ) = pdf(θ,φ)/(sinθdθdφ/dθdφ) = pdf(θ,φ)/sinθ
    // pdf(θ,φ) = pdf(w)sinθ = c*sinθcosθ = sinθcosθ/π
    // 马利方法,将极坐标的点添加y轴，映射到半球面上
    float r = glm::sqrt(u.x);
    float phi = 2.f * PI * u.y;
    return {r * glm::cos(phi), glm::sqrt(1 - r * r), r * glm::sin(phi)};
}

inline float CosineSampleHemispherePdf(const glm::vec3 &direction)
{
    return direction.y / PI;
}

inline glm::vec3 UniformSampleHemisphere(const RNG &rng)
{
    // 接受拒绝采样
    glm::vec3 result;
    do
    {
        // 在立方体内得到一个在单位球内的点，不在球内的点会被拒绝
        result = {rng.uniform(), rng.uniform(), rng.uniform()};
        result = result * 2.f - 1.f;
    } while (glm::length(result) > 1);
    // 将下半球的点映射到上半球
    if (result.y < 0)
    {
        result.y = -result.y;
    }
    return glm::normalize(result);
}