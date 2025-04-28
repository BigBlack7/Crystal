// #include "RTRenderer.hpp"
// #include "../until/frame.hpp"
// #include "../sample/spherical.hpp"
// glm::vec3 RTRenderer::renderPixel(const glm::ivec2 &pixelCoord)
// {
//     auto ray = mCamera.generateRay(pixelCoord, {rng.uniform(), rng.uniform()});
//     glm::vec3 beta = {1, 1, 1};
//     glm::vec3 color = {0, 0, 0};
//     size_t bounces = 32;
//     while (bounces--)
//     {
//         auto hitInfo = mScene.intersect(ray);
//         if (hitInfo.has_value())
//         {
//             color += beta * hitInfo->mMaterial->mEmission;
//             beta *= hitInfo->mMaterial->mAlbedo;
//             ray.mOrigin = hitInfo->mHitPoint;
//             Frame frame{hitInfo->mNormal};
//             glm::vec3 lightDirection;
//             if (hitInfo->mMaterial->_IsSpecular) 镜面反射
//             {    视线方向要从像素到相机, 这样可以用法线局部轴对称计算反射方向
//                 glm::vec3 viewDiretion = frame.localFromWorld(-ray.mDirection);
//                 lightDirection = {-viewDiretion.x, viewDiretion.y, -viewDiretion.z};
//             }
//             else 漫反射
//             {
//                 lightDirection = UniformSampleHemisphere(rng);
//             }
//             ray.mDirection = frame.worldFromLocal(lightDirection);
//         }
//         else
//         {
//             break;
//         }
//     }
//     return color;
// }