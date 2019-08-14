#include "Material.h"

#include "ctmRand.h"
#include "HitRecord.h"

namespace
{
	glm::vec3 randomInUnitSphere()
	{
		glm::vec3 p;
		do {
			p = 2.0f * glm::vec3(ctmRand(),
				ctmRand(),
				ctmRand()) - glm::vec3(1, 1, 1);
		} while (glm::length(p) >= 1);
		return (p);
	}

	glm::vec3 reflect(const glm::vec3& v, const glm::vec3& n)
	{
		return (v - 2 * glm::dot(v, n) * n);
	}

	bool refract(const glm::vec3& v, const glm::vec3& n, const float ni_over_nt, glm::vec3& refracted)
	{
		glm::vec3 uv = glm::normalize(v);
		float dt = glm::dot(uv, n);
		float discriminant = 1 - ni_over_nt * ni_over_nt * (1 - dt * dt);
		if (discriminant > 0)
		{
			refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
			return (true);
		}
		return (false);
	}

	float schlick(const float cosine, const float ri)
	{
		float r0 = (1 - ri) / (1 + ri);
		r0 = r0 * r0;
		return (r0 + (1 - r0) * pow(1 - cosine, 5));
	}
}

Lambert::Lambert(const glm::vec3& albedo)
	: albedo(albedo)
{}

bool Lambert::scatter(const Ray& in, const HitRecord& hit, glm::vec3& attenuation, Ray& scattered) const
{
	glm::vec3 target = hit.p + hit.normal + randomInUnitSphere();
	scattered = Ray(hit.p, target - hit.p);
	attenuation = albedo;
	return (true);
}

Metal::Metal(const glm::vec3& albedo, const float fuzz)
	: albedo(albedo), fuzz(fuzz)
{
	if (fuzz >= 1)
		this->fuzz = 1;
}

bool Metal::scatter(const Ray& in, const HitRecord& hit, glm::vec3& attenuation, Ray& scattered) const
{
	glm::vec3 reflected = reflect(glm::normalize(in.getDirection()), hit.normal);
	scattered = Ray(hit.p, reflected + fuzz * randomInUnitSphere());
	attenuation = albedo;
	return (glm::dot(scattered.getDirection(), hit.normal) > 0);
}

Dialectric::Dialectric(const float ri)
	: ri(ri)
{}

bool Dialectric::scatter(const Ray& in, const HitRecord& hit, glm::vec3& attenuation, Ray& scattered) const
{
	glm::vec3 outwardNormal;
	glm::vec3 reflected = reflect(in.getDirection(), hit.normal);
	float ni_over_nt;
	attenuation = glm::vec3(1, 1, 1);
	glm::vec3 refracted;
	float reflectProb;
	float cosine;
	if (glm::dot(in.getDirection(), hit.normal) > 0)
	{
		outwardNormal = -hit.normal;
		ni_over_nt = ri;
		cosine = glm::dot(in.getDirection(), hit.normal) / in.getDirection().length();
		//cosine = glm::sqrt(1 - _ri * _ri * (1 - cosine * cosine));
	}
	else
	{
		outwardNormal = hit.normal;
		ni_over_nt = 1 / ri;
		cosine = -glm::dot(in.getDirection(), hit.normal) / in.getDirection().length();
	}
	if (refract(in.getDirection(), outwardNormal, ni_over_nt, refracted))
		reflectProb = schlick(cosine, ri);
	else
		reflectProb = 1;
	if (ctmRand() < reflectProb)
		scattered = Ray(hit.p, reflected);
	else
		scattered = Ray(hit.p, refracted);
	return (true);
}