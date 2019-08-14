#pragma once

class Ray;
class Material;
struct HitRecord;

class IHitable
{
public:
	virtual bool hit(const Ray& ray, const float minTime, const float maxTime, HitRecord& record) const = 0;
};