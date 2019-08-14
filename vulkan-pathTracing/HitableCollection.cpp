#include "HitableCollection.h"

#include "HitRecord.h"

void HitableCollection::takeOwnershipOf(IHitable **newCollection)
{
	if (collection)
	{
		for (size_t i = 0; collection[i] != nullptr; i++)
		{
			delete collection[i];
		}
		delete[] collection;
	}
	collection = newCollection;
}

bool HitableCollection::hit(const Ray& ray, const float minTime, const float maxTime, HitRecord& record) const
{
	HitRecord tmpRecord;
	float closest = maxTime;
	bool hasHitAnything = false;
	for (size_t i = 0; collection[i] != nullptr; i++)
	{
		if (collection[i]->hit(ray, minTime, closest, tmpRecord))
		{
			hasHitAnything = true;
			closest = tmpRecord.t;
			record = tmpRecord;
			record.hit = collection[i];
		}
	}
	return (hasHitAnything);
}