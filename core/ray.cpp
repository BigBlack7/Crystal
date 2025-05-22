#include "ray.hpp"

Ray Ray::objectFromWorld(const glm::mat4 &_objectFromWorld) const
{
    // 将射线从世界空间转换到物体空间
    glm::vec3 o = _objectFromWorld * glm::vec4(mOrigin, 1);
    glm::vec3 d = _objectFromWorld * glm::vec4(mDirection, 0);
    return Ray{o, d};
}
