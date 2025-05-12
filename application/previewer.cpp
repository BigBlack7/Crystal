#include "previewer.hpp"
#include "../myEngine/renderer/normalRenderer.hpp"
#include "../myEngine/renderer/debugRenderer.hpp"
#include "threadPool.hpp"
#include <iostream>

Previewer::Previewer(Renderer &renderer, float fps) : mRenderer(renderer), mFPS(fps)
{
    // 记录原始分辨率, 用于缩放和后续恢复
    auto &film = mRenderer.mCamera.getFilm();
    mFilmResolution = {film.getWidth(), film.getHeight()};
    // 将渲染器添加到渲染模式中，按tab键切换
    mRenderModes.push_back(&mRenderer);
    mRenderModes.push_back(new NormalRenderer(mRenderer.mCamera, mRenderer.mScene));
    DEBUG_LINE(mRenderModes.push_back(new BoundsTestCountRenderer(mRenderer.mCamera, mRenderer.mScene)));
    DEBUG_LINE(mRenderModes.push_back(new TrianglesTestCountRenderer(mRenderer.mCamera, mRenderer.mScene)));

    mScale = 1.f;
}

bool Previewer::preview()
{
    // 创建窗口
    mWindow = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(sf::Vector2u(mFilmResolution.x, mFilmResolution.y)), // 窗口大小
        "PBRT",                                                            // 窗口标题
        sf::Style::Titlebar,                                               // 窗口样式
        sf::State::Windowed);                                              // 窗口状态: 无边框窗口化

    mTexture = std::make_shared<sf::Texture>(sf::Vector2u(mFilmResolution.x, mFilmResolution.y)); // 纹理大小
    mTexture->setSmooth(true);                                                                    // 在低分辨率下会更加平滑
    mSprite = std::make_shared<sf::Sprite>(*mTexture);                                            // 精灵大小
    setResolution(0.1f);                                                                          // 默认1/10的分辨率
    auto &camera = mRenderer.mCamera;
    auto &film = camera.getFilm();
    // 窗口事件
    bool mouseGrabbed = false;                          // 鼠标是否被捕获
    sf::Vector2i windowCenter{mWindow->getSize() / 2u}; // 窗口中心
    float dt = 0.f;
    bool renderFinalResult = false;
    // 主循环
    while (mWindow->isOpen())
    {
        while (auto event = mWindow->pollEvent())
        {
            // 键盘事件
            if (auto keyReleased = event->getIf<sf::Event::KeyReleased>())
            {
                if (keyReleased->scancode == sf::Keyboard::Scancode::Escape) // keyboard: ESC 退出
                {
                    mWindow->close();
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::Enter) // keyboard: ENTER 渲染
                {
                    renderFinalResult = true; // 渲染最终结果
                    mWindow->close();
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::Tab) // keyboard: TAB 切换渲染模式
                {
                    mRenderModeIndex = (mRenderModeIndex + 1) % mRenderModes.size(); // 切换渲染模式
                    mCurrentSPP = 0;                                                 // 重置采样次数
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::NumpadPlus) // keypad: + 增加fps
                {
                    mFPS += 1;
                    printf("FPS: %f\n", mFPS);
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::NumpadMinus) // keypad: - 减少fps
                {
                    // 限制 mFPS 不小于 1
                    if (mFPS > 1)
                    {
                        mFPS -= 1;
                        printf("FPS: %f\n", mFPS);
                    }
                    else
                    {
                        printf("FPS cannot be less than 1\n");
                    }
                }
                else if (keyReleased->scancode == sf::Keyboard::Scancode::CapsLock) // keyboard: CapsLock 切换鼠标捕获
                {
                    mouseGrabbed = !mouseGrabbed;                   // 切换鼠标捕获
                    mWindow->setMouseCursorGrabbed(mouseGrabbed);   // 设置鼠标捕获
                    mWindow->setMouseCursorVisible(!mouseGrabbed);  // 设置鼠标被捕获后的可见性
                    sf::Mouse::setPosition(windowCenter, *mWindow); // 设置鼠标位置
                }
            }
            // 鼠标事件
            else if (auto *mouseMoved = event->getIf<sf::Event::MouseMoved>())
            {
                if (!mouseGrabbed) // 鼠标未被捕获, 不处理
                    continue;
                auto delta = mouseMoved->position - windowCenter; // 鼠标移动量
                if (delta.x == 0 && delta.y == 0)                 // 鼠标未移动, 不处理
                    continue;
                camera.turn({delta.x, delta.y});
                mCurrentSPP = 0;                                // 重置采样次数,移动鼠标后在下一帧重新渲染新的图像
                sf::Mouse::setPosition(windowCenter, *mWindow); // 设置鼠标位置
            }
            else if (auto *mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) // 鼠标滚轮事件
            {
                if (!mouseGrabbed) // 鼠标未被捕获, 不处理
                    continue;
                camera.zoom(mouseWheel->delta); // 滚轮滚动量
                mCurrentSPP = 0;                // 重置采样次数,移动鼠标后在下一帧重新渲染新的图像
            }
        }
        if (mouseGrabbed)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            {
                camera.move(dt, Direction::Forward);
                mCurrentSPP = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            {
                camera.move(dt, Direction::Backward);
                mCurrentSPP = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            {
                camera.move(dt, Direction::Left);
                mCurrentSPP = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            {
                camera.move(dt, Direction::Right);
                mCurrentSPP = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            {
                camera.move(dt, Direction::Up);
                mCurrentSPP = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                camera.move(dt, Direction::Down);
                mCurrentSPP = 0;
            }
        }

        auto start = std::chrono::high_resolution_clock::now();            // 记录开始时间
        rendererFrame();                                                   // 渲染一帧
        auto duration = std::chrono::high_resolution_clock::now() - start; // 计算耗时

        auto buffer = film.generateRGBABuffer(); // 生成RGBABuffer
        mTexture->update(buffer.data());         // 更新纹理

        mWindow->clear();
        mWindow->draw(*mSprite);
        mWindow->display();
        // 在当前帧绘制完成后再调整分辨率绘制新帧
        dt = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() * 0.001f;
        adjustResolution(dt);
    }
    film.setResolution(mFilmResolution.x, mFilmResolution.y); // 恢复原始分辨率
    return renderFinalResult;
}

Previewer::~Previewer()
{
}

void Previewer::rendererFrame()
{
    auto *renderer = mRenderModes[mRenderModeIndex];
    size_t renderSPP = mRenderModeIndex == 0 ? 4 : 1; // 只有路径追踪模式需要多采样
    auto &film = mRenderer.mCamera.getFilm();
    if (mCurrentSPP == 0) // 第一次渲染
    {
        film.clear(); // 清空
    }
    threadPool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y)
                           {
                               for (size_t i = mCurrentSPP; i < mCurrentSPP + renderSPP; i++)
                               {
                                   film.addSample(x, y, renderer->renderPixel({x, y, i}));
                               }
                               // end
                           });
    threadPool.wait();
    mCurrentSPP += renderSPP;
}

void Previewer::setResolution(float scale)
{
    scale = glm::clamp(scale, 0.f, 1.f);
    if (scale == mScale)
    {
        return;
    }
    mScale = scale;
    // 调整分辨率
    glm::ivec2 newResolution = glm::vec2(mFilmResolution) * mScale;
    if (newResolution.x == 0)
        newResolution.x = 1;
    if (newResolution.y == 0)
        newResolution.y = 1;
    mRenderer.mCamera.getFilm().setResolution(newResolution.x, newResolution.y);
    auto res = mTexture->resize(sf::Vector2u(newResolution.x, newResolution.y));
    // 调整精灵, 精灵持有的纹理此时小于窗口大小, 所以需要缩放
    mSprite->setTexture(*mTexture);
    mSprite->setScale(sf::Vector2f(static_cast<float>(mFilmResolution.x) / newResolution.x, static_cast<float>(mFilmResolution.y) / newResolution.y));
    mCurrentSPP = 0; // 重置采样次数,清空胶片
}

void Previewer::adjustResolution(float dt)
{
    // 根据当前渲染一帧的时间和当前fps的差值动态调整分辨率
    float expected_dt = 1.f / mFPS;
    if (glm::abs(expected_dt - dt) / expected_dt > 0.4f)
    {
        float new_scale = mScale * (1.f + 0.1f * (glm::sqrt(expected_dt / dt) - 1.f));
        setResolution(new_scale);
    }
}
