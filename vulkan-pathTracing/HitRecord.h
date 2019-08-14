#pragma once

#include <glm/glm.hpp>

class IMaterial;
class IHitable;

struct HitRecord
{
	float t;
	glm::vec3 p;
	glm::vec3 normal;
	const IMaterial *material;
	IHitable *hit;
};