#include <fstream>
#include "film.hpp"
#include "threadPool.hpp"
#include "../myEngine/postProcess/rgb.hpp"

Film::Film(size_t width, size_t height) : mWidth(width), mHeight(height)
{
    mPixels.resize(width * height); // 初始化像素数组
}

void Film::save(const std::filesystem::path &fileName)
{
    std::ofstream file(fileName, std::ios::binary); // 二进制格式打开文件
    // p3为ASCII格式，p6为二进制格式，每个通道8位，远比p3快
    file << "P6\n"
         << mWidth << " " << mHeight << "\n255\n";

    std::vector<uint8_t> pixelBuffer(mWidth * mHeight * 3);
    threadPool.parallelFor(mWidth, mHeight, [&](size_t x, size_t y)
                           {
        auto pixel = getPixel(x, y);
        if (pixel.mSampleCount == 0)
        {
            //在最坏情况下，所有采样点都可能为不正常的值，导致计算平均值时出现除以0的情况。
            return;
        }
        
        RGB rgb(pixel.mColor / static_cast<float>(pixel.mSampleCount));// 计算平均颜色, 并进行Gamma校正
        auto index = (y * mWidth + x) * 3;
        pixelBuffer[index + 0] = static_cast<uint8_t>(rgb.mR);
        pixelBuffer[index + 1] = static_cast<uint8_t>(rgb.mG);
        pixelBuffer[index + 2] = static_cast<uint8_t>(rgb.mB); }, false);
    threadPool.wait();

    // 将像素数据以二进制写入文件
    file.write(reinterpret_cast<const char *>(pixelBuffer.data()), pixelBuffer.size());
}