#pragma once

#include <glm/glm.hpp>

class Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

public:
	Ray() = default;
	Ray(const glm::vec3& newOrigin, const glm::vec3& newDirection)
		: origin(newOrigin), direction(newDirection)
	{}

	const glm::vec3& getOrigin() const { return (origin); }
	const glm::vec3& getDirection() const { return (direction); }

	glm::vec3 pointAtTime(float t) const { return (origin + t * direction); }
};