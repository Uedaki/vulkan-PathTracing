#pragma once

#include <glm/glm.hpp>

#include "Ray.h"

class Camera
{
	glm::vec3 origin;
	glm::vec3 lowerLeft;
	glm::vec3 horizontal;
	glm::vec3 vertical;

	glm::vec3 w;
	glm::vec3 u;
	glm::vec3 v;

	float lensRadius;

public:
	Camera(glm::vec3 lookFrom, glm::vec3 lookAt, glm::vec3 up, float vfov, float aspect, float aperture, float focusDist);

	Ray getRay(const float s, const float t) const;
};