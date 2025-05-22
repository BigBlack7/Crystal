#include <iostream>
#include "renderer.hpp"
#include "../../application/threadPool.hpp"
#include "../until/progress.hpp"
#include "../until/profile.hpp"

/**
 * @brief 执行渲染操作，将渲染结果保存到指定文件。
 *
 * 该函数会按照指定的采样数（Samples Per Pixel, SPP）对图像进行渲染，
 * 并在渲染过程中逐步保存中间结果。使用线程池并行处理像素，同时显示进度条。
 *
 * @param spp 每个像素的采样数，控制渲染的质量和耗时。
 * @param fileName 渲染结果保存的文件路径。
 */
void Renderer::render(size_t spp, const std::filesystem::path &fileName)
{
    // 使用 PROFILE 宏记录 "Renderer" 代码块的执行时间，用于性能分析
    PROFILE("Renderer");

    // 当前已经完成的采样数，初始为 0
    size_t currentSpp = 0;
    // 每次迭代增加的采样数，初始为 1
    size_t increase = 1;

    // 获取相机中的胶片对象引用，胶片用于存储渲染结果
    auto &film = mCamera.getFilm();
    // 清空胶片上已有的渲染结果
    film.clear();

    // 创建一个进度条对象，总进度为胶片像素总数乘以采样数
    ProgressBar progressBar(film.getWidth() * film.getHeight() * spp);

    // 循环进行渲染，直到当前采样数达到指定的采样数
    while (currentSpp < spp)
    {
        // 使用线程池并行处理胶片上的每个像素
        threadPool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y)
                               {
            // 对当前像素进行多次采样，采样次数为 increase
            for (int i = 0; i < increase; i++)
            {
                // 渲染指定坐标和采样数的像素，并将结果添加到胶片上
                film.addSample(x, y, renderPixel({x, y, currentSpp + i}));
            }
            // 更新进度条，增加的进度为本次采样的次数
            progressBar.update(increase); });

        // 等待线程池中的所有任务完成
        threadPool.wait();

        // 更新当前采样数，加上本次迭代增加的采样数
        currentSpp += increase;

        // 计算下一次迭代增加的采样数，最大不超过 32
        increase = std::min<size_t>(currentSpp, 32);

        // 将当前的渲染结果保存到指定文件
        film.save(fileName);

        // 输出当前已经完成的采样数信息
        std::cout << currentSpp << " spp has been saved!" << std::endl;
    }
}