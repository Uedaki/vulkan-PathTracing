#include "Camera.h"

#include <glm/gtc/constants.hpp>

#include <math.h>

#include "ctmRand.h"

namespace
{
	glm::vec3 randomInUnitDisk()
	{
		glm::vec3 p;
		do
		{
			p = 2.0f * glm::vec3(ctmRand(), ctmRand(), 0) - glm::vec3(1, 1, 0);
		} while (glm::dot(p, p) >= 1);
		return (p);
	}
}

Camera::Camera(glm::vec3 lookFrom, glm::vec3 lookAt, glm::vec3 up, float vfov, float aspect, float aperture, float focusDist)
	: origin(lookFrom)
{
	lensRadius = aperture / 2;

	float theta = vfov * glm::pi<float>() / 180;
	float halfHeight = glm::tan(theta / 2);
	float halfWidth = aspect * halfHeight;

	w = glm::normalize(lookFrom - lookAt);
	u = glm::normalize(glm::cross(up, w));
	v = glm::cross(w, u);

	lowerLeft = origin - halfWidth * focusDist * u - halfHeight * focusDist * v - focusDist * w;

	horizontal = focusDist * 2 * halfWidth * u;
	vertical = focusDist * 2 * halfHeight * v;
}

Ray Camera::getRay(const float s, const float t) const
{
	glm::vec3 rd = lensRadius * randomInUnitDisk();
	glm::vec3 offset = u * rd.x + v * rd.y;
	return (Ray(origin + offset, lowerLeft + s * horizontal + t * vertical - origin - offset));
}