#pragma once

#include "IHitable.h"
#include "Ray.h"

class HitableCollection : public IHitable
{
	IHitable **collection = nullptr;

public:
	void takeOwnershipOf(IHitable **newCollection);
	bool hit(const Ray& ray, const float minTime, const float maxTime, HitRecord& record) const override;
};