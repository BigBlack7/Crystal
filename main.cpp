#include <iostream>

#include "application/film.hpp"

#include "myEngine/ray.hpp"
#include "myEngine/scene.hpp"

#include "myEngine/camera/camera.hpp"

#include "myEngine/mesh/sphere.hpp"
#include "myEngine/mesh/model.hpp"
#include "myEngine/mesh/plane.hpp"

#include "myEngine/postProcess/rgb.hpp"

#include "myEngine/renderer/normalRenderer.hpp"
#include "myEngine/renderer/debugRenderer.hpp"
#include "myEngine/renderer/PTRenderer.hpp"

#include "myEngine/material/diffuseMaterial.hpp"
#include "myEngine/material/specularMaterial.hpp"
#include "myEngine/material/dielectricMaterial.hpp"
#include "myEngine/material/conductorMaterial.hpp"
#include "myEngine/material/groundMaterial.hpp"

int main()
{

    Film film(196 * 4, 108 * 4);
    // Film film(2560, 1440);
    Camera camera{film, {-10, 1.5, 0}, {0, 0, 0}, 45};

    Model model("../../models/dragon_871k.obj");
    Sphere sphere{{0, 0, 0}, 1.f};
    Plane plane{{0, 0, 0}, {0, 1, 0}};

    Scene scene{};
    // RNG rng{1234};
    // for (size_t i = 0; i < 10000; i++)
    // {
    //     glm::vec3 random_pos{rng.uniform() * 100 - 50, rng.uniform() * 2, rng.uniform() * 100 - 50};
    //     float u = rng.uniform();
    //     if (u < 0.9)
    //     {
    //         Material *material = nullptr;
    //         if (rng.uniform() > 0.5)
    //             material = new SpecularMaterial{RGB{202, 159, 117}};
    //         else
    //             material = new DiffuseMaterial{RGB{202, 159, 117}};
    //         scene.addShape(model, material, random_pos, {1, 1, 1}, {rng.uniform() * 360, rng.uniform() * 360, rng.uniform() * 360});
    //     }
    //     else if (u < 0.95)
    //     {
    //         scene.addShape(sphere, new SpecularMaterial{{rng.uniform(), rng.uniform(), rng.uniform()}}, random_pos, {0.4, 0.4, 0.4});
    //     }
    //     else
    //     {
    //         random_pos.y += 6;
    //         auto *material = new DiffuseMaterial{{0, 0, 0}};
    //         material->setEmission({rng.uniform() * 4, rng.uniform() * 4, rng.uniform() * 4});
    //         scene.addShape(sphere, material, random_pos);
    //     }
    // }

    for (int i = -3; i <= 3; i++)
    {
        scene.addShape(sphere, new DielectricMaterial{1.f + 0.2f * (i + 3), {1, 1, 1}, (3.f - i) / 18.f, (3.f - i) / 6.f}, {0, 0.5f, i * 2.f}, {0.8f, 0.8f, 0.8f});
    }
    for (int i = -3; i <= 3; i++)
    {
        glm::vec3 c = RGB::GenerateHeatMap((i + 3.f) / 6.f);
        scene.addShape(sphere, new ConductorMaterial{glm::vec3(2.f - c * 2.f), glm::vec3(2.f + c * 3.f), (3.f - i) / 6.f, (3.f - i) / 18.f}, {0, 2.5f, i * 2.f}, {0.8f, 0.8f, 0.8f});
    }
    scene.addShape(model, new DielectricMaterial{1.8f, RGB{128, 211, 131}, 0.1f, 0.1f}, {-5, 0.4, 1.5}, {2, 2, 2});
    scene.addShape(model, new ConductorMaterial{{0.1, 1.2, 1.8}, {5, 2.5, 2}, 0.1f, 0.1f}, {-5, 0.4, -1.5}, {2, 2, 2});
    scene.addShape(plane, new GroundMaterial{RGB(120, 204, 157)}, {0.f, -0.5f, 0.f});
    auto *lightMaterial = new DiffuseMaterial{{1, 1, 1}};
    lightMaterial->setEmission({0.95f, 0.95f, 1.f}); // 面光源光强
    // lightMaterial->setEmission({0.95f * 5, 0.95f * 5, 1.f * 5}); // 点光源光强
    scene.addShape(plane, lightMaterial, {0.f, 10.f, 0.f}); // 面光源
    // scene.addShape(sphere, lightMaterial, {-2.f, 6.f, 0.f}, {2, 2, 2}); // 球光源

    // scene.addShape(model, new DielectricMaterial{1.6f, RGB{255, 255, 255}}, {0, 0, 1.f}, {1.5, 1.5, 1.5});
    // scene.addShape(model, new ConductorMaterial{{0.1, 1.2, 1.8}, {5, 2.5, 2}}, {0, 0, -1.f}, {1.5, 1.5, 1.5});
    // auto *green = new DielectricMaterial{1.8f, RGB{255, 255, 255}};
    // scene.addShape(sphere, green, {3.f, 1.5f, -2.5f});
    // auto *blue = new ConductorMaterial{{0.1, 1.2, 1.8}, {5, 2.5, 2}};
    // scene.addShape(sphere, blue, {3.f, 1.5f, 2.5f});
    // scene.addShape(plane, new GroundMaterial{RGB(120, 204, 157)}, {0.f, -0.5f, 0.f});
    // auto *lightMaterial = new DiffuseMaterial{{1, 1, 1}};
    // lightMaterial->setEmission({1.f, 1.f, 1.f});
    // scene.addShape(plane, lightMaterial, {0.f, 10.f, 0.f});

    scene.build();
    // NormalRenderer normalRenderer{camera, scene};
    // normalRenderer.render(1, "../../ppm/normal.ppm");
    // BoundsTestCountRenderer btcRenderer{camera, scene};
    // btcRenderer.render(1, "../../ppm/btc.ppm");
    // TrianglesTestCountRenderer ttcRenderer{camera, scene};
    // ttcRenderer.render(1, "../../ppm/ttc.ppm");

    PTRenderer ptRenderer{camera, scene};
    ptRenderer.render(128, "../../lover.ppm");

    std::cout << "Hello, PBRT!" << std::endl;

    return 0;
}