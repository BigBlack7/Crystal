#pragma once
#include <glm/glm.hpp>

class Frame // 局部坐标系
{
public:
    Frame(const glm::vec3 &normal);
    glm::vec3 localFromWorld(const glm::vec3 &worldDiretion) const;
    glm::vec3 worldFromLocal(const glm::vec3 &localDiretion) const;

private:
    glm::vec3 x_axis;
    glm::vec3 y_axis;
    glm::vec3 z_axis;
};