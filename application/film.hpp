#pragma once
#include <filesystem> // c++17
#include <vector>
#include <glm/glm.hpp>

struct Pixel
{
    glm::vec3 mColor{0, 0, 0}; // 颜色, 物理意义上的颜色.
    int mSampleCount{0}; // 采样次数
};

class Film
{
public:
    Film(size_t width, size_t height);
    void save(const std::filesystem::path &fileName); // 保存图像

    size_t getWidth() const { return mWidth; }
    size_t getHeight() const { return mHeight; }

    Pixel getPixel(size_t x, size_t y)
    {
        return mPixels[x + y * mWidth];
    }

    void addSample(size_t x, size_t y, const glm::vec3 &color)
    {
        // 用于向指定位置 (x, y) 的像素添加一个采样颜色。同时增加该像素的采样次数。
        mPixels[x + y * mWidth].mColor += color;
        mPixels[x + y * mWidth].mSampleCount++;
    }

    void clear()
    {
        // 清空 mPixels 向量，并重新调整其大小为 mWidth * mHeight。
        mPixels.clear();
        mPixels.resize(mWidth * mHeight);
    }

private:
    size_t mWidth;
    size_t mHeight;
    std::vector<Pixel> mPixels;
};