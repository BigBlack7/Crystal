#pragma once
#include "../core/renderer/renderer.hpp"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class Previewer
{

public:
    Previewer(Renderer &renderer, float fps = 30);
    bool preview();
    ~Previewer();

private:
    void rendererFrame();
    void setResolution(float scale);
    void adjustResolution(float dt);

private:
    Renderer &mRenderer;
    std::vector<Renderer *> mRenderModes;
    size_t mRenderModeIndex = 0;

    float mScale;
    float mFPS;
    glm::ivec2 mFilmResolution;

    size_t mCurrentSPP = 0;

    std::shared_ptr<sf::RenderWindow> mWindow;
    std::shared_ptr<sf::Texture> mTexture;
    std::shared_ptr<sf::Sprite> mSprite;
};