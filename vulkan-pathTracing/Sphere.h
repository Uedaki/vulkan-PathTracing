#pragma once

#include <glm/glm.hpp>

#include "IHitable.h"

class IMaterial;

class Sphere : public IHitable
{
	glm::vec3					center;
	IMaterial					*material;
	float						radius;

public:
	Sphere(glm::vec3 center, float radius, IMaterial *material);
	Sphere(const Sphere &ref);
	Sphere &operator=(const Sphere &ref);

	bool hit(const Ray& ray, const float minTime, const float maxTime, HitRecord& record) const override;
};