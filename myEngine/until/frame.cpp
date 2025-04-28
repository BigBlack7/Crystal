#include "frame.hpp"

Frame::Frame(const glm::vec3 &normal)
{
    y_axis = glm::normalize(normal);// 以法向量为y轴
    // 判断法线是否与世界坐标系y轴同向, 同向会导致错误.
    glm::vec3 up = glm::abs(normal.y) < 0.99999 ? glm::vec3{0, 1, 0} : glm::vec3{0, 0, 1};
    x_axis = glm::normalize(glm::cross(up, normal));
    z_axis = glm::normalize(glm::cross(x_axis, y_axis));
}

glm::vec3 Frame::localFromWorld(const glm::vec3 &worldDiretion) const
{
    // 计算世界方向在局部坐标系中的分量, 将世界坐标投影到局部坐标系中.
    return glm::normalize(glm::vec3{
        glm::dot(worldDiretion, x_axis),
        glm::dot(worldDiretion, y_axis),
        glm::dot(worldDiretion, z_axis)});
}

glm::vec3 Frame::worldFromLocal(const glm::vec3 &localDiretion) const
{
    // 这里的xyz轴是世界坐标系下的矢量，而不是局部坐标系的(1,0,0).所以局部的x分量乘上世界坐标系下的局部x轴就代表它在世界坐标系下的x分量.
    return glm::normalize(localDiretion.x * x_axis + localDiretion.y * y_axis + localDiretion.z * z_axis);
}
