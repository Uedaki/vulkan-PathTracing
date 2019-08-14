#include "Sphere.h"

#include "HitRecord.h"
#include "Material.h"

Sphere::Sphere(glm::vec3 center, float radius, IMaterial *material)
	: center(center), radius(radius), material(material)
{}

Sphere::Sphere(const Sphere &ref)
{
	center = ref.center;
	radius = ref.radius;
	material = ref.material;
}

Sphere &Sphere::operator=(const Sphere &ref)
{
	if (this != &ref)
	{
		center = ref.center;
		radius = ref.radius;
		material = ref.material;
	}
	return (*this);
}

bool Sphere::hit(const Ray& ray, const float t_min, const float t_max, HitRecord& record) const
{
	record.material = material;

	glm::vec3 oc = ray.getOrigin() - center;
	float a = glm::dot(ray.getDirection(), ray.getDirection());
	float b = glm::dot(oc, ray.getDirection());
	float c = glm::dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0)
	{
		float temp1 = (-b - sqrt(discriminant)) / a;
		float temp2 = (-b + sqrt(discriminant)) / a;
		if (t_min < temp1 && temp1 < t_max)
		{
			record.t = temp1;
			record.p = ray.pointAtTime(temp1);
			record.normal = (record.p - center) / radius;
			return (true);
		}
		else if (t_min < temp2 && temp2 < t_max)
		{
			record.t = temp2;
			record.p = ray.pointAtTime(temp2);
			record.normal = (record.p - center) / radius;
			return (true);
		}

	}
	return (false);
}