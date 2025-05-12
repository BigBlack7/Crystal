#pragma once
#include <glm/glm.hpp>
#include <array>

class RGB Lerp(const RGB &a, const RGB &b, float t);

class RGB
{
public:
    RGB(int r, int g, int b) : mR(r), mG(g), mB(b) {}
    RGB(const glm::vec3 &color)
    {
        // 将线性空间的颜色值进行伽马校正（1/2.2 次方），并转换为 0 到 255 的整数
        // 同时使用 glm::clamp 确保结果在 0 到 255 之间
        mR = glm::clamp<int>(static_cast<int>(glm::pow(color.r, 1.f / 2.2f) * 255.f), 0, 255);
        mG = glm::clamp<int>(static_cast<int>(glm::pow(color.g, 1.f / 2.2f) * 255.f), 0, 255);
        mB = glm::clamp<int>(static_cast<int>(glm::pow(color.b, 1.f / 2.2f) * 255.f), 0, 255);
    }

    // 将sRGB空间的颜色值转换为线性空间
    operator glm::vec3() const
    {
        return glm::vec3(
            glm::pow(mR / 255.f, 2.2f),
            glm::pow(mG / 255.f, 2.2f),
            glm::pow(mB / 255.f, 2.2f));
    }

    // 热力图，用来dedug，观察bvh的构建情况
    inline static RGB GenerateHeatMap(float t)
    {
        std::array<RGB, 25> color_pallet{
            RGB{68, 1, 84},
            RGB{71, 17, 100},
            RGB{72, 31, 112},
            RGB{71, 45, 123},
            RGB{68, 58, 131},

            RGB{64, 70, 136},
            RGB{59, 82, 139},
            RGB{54, 93, 141},
            RGB{49, 104, 142},
            RGB{44, 114, 142},

            RGB{40, 124, 142},
            RGB{36, 134, 142},
            RGB{33, 144, 140},
            RGB{31, 154, 138},
            RGB{32, 164, 134},

            RGB{39, 173, 129},
            RGB{53, 183, 121},
            RGB{71, 193, 110},
            RGB{93, 200, 99},
            RGB{117, 208, 84},

            RGB{143, 215, 68},
            RGB{170, 220, 50},
            RGB{199, 224, 32},
            RGB{227, 228, 24},
            RGB{253, 231, 37}};

        if (t < 0.f || t > 1.f)
        {
            return RGB{255, 0, 0};
        }
        float idx_f = t * (color_pallet.size() - 1);
        size_t idx = (size_t)glm::floor(idx_f);
        return Lerp(color_pallet[idx], color_pallet[idx + 1], glm::fract(idx_f));
    }

public:
    int mR, mG, mB;
};

// 线性插值，用于颜色的平滑过渡，插值热力图的
inline RGB Lerp(const RGB &a, const RGB &b, float t)
{
    return RGB(
        glm::clamp<int>(static_cast<int>(a.mR + (b.mR - a.mR) * t), 0, 255),
        glm::clamp<int>(static_cast<int>(a.mG + (b.mG - a.mG) * t), 0, 255),
        glm::clamp<int>(static_cast<int>(a.mB + (b.mB - a.mB) * t), 0, 255));
}