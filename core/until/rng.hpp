#pragma once
#include <random>

class RNG
{
public:
    RNG(size_t seed) {}
    RNG() : RNG(0) {} // 默认调用第一个构造函数

    void setSeed(size_t seed) { gen.seed((unsigned int)seed); }

    // 生成0到1之间的均匀分布随机数
    float uniform() const
    {
        return uniformDistribution(gen);
    }

private:
    // 可变的 Mersenne Twister 随机数生成器，使用 mutable 关键字允许在 const 成员函数中修改
    mutable std::mt19937 gen;
    // 可变的均匀分布对象，生成范围在 [0, 1) 之间的随机浮点数，使用 mutable 关键字允许在 const 成员函数中修改
    mutable std::uniform_real_distribution<float> uniformDistribution{0, 1};
};