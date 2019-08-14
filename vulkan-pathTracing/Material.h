#pragma once

#include <glm/glm.hpp>

#include "Ray.h"
#include "IHitable.h"

class IMaterial
{
public:
	virtual	bool scatter(const Ray& in, const HitRecord& hit, glm::vec3& attenuation, Ray& scattered) const = 0;
};

class Lambert : public IMaterial
{
	glm::vec3 albedo;

public:
	Lambert(const glm::vec3& albedo);
	
	bool scatter(const Ray& in, const HitRecord& hit, glm::vec3& attenuation, Ray& scattered) const override;
};

class Metal : public IMaterial
{
	glm::vec3 albedo;
	float fuzz;

public:
	Metal(const glm::vec3& albedo, const float fuzz);

	bool scatter(const Ray& in, const HitRecord& hit, glm::vec3& attenuation, Ray& scattered) const override;
};

class Dialectric : public IMaterial
{
	float ri;

public:
	Dialectric(const float ri);

	bool scatter(const Ray& in, const HitRecord& hit, glm::vec3& attenuation, Ray& scattered) const override;
};