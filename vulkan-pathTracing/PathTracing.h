#pragma once

#include <thread>

#include "IPixelBlockQueueOwner.h"
#include "PixelBlockQueue.h"

class Camera;
class HitableCollection;
class IHitable;
class Ray;

class PathTracing : public IPixelBlockQueueOwner
{
	static constexpr int NBR_THREAD = 2;
	static constexpr int MAX_DEPTH = 50;
public:
	PathTracing(int width, int heigth, uint32_t nbSamples,
		const HitableCollection &collection, const Camera &cam);
	~PathTracing();

	void startRendering();
	void endRendering();

	void retreiveThreadResult();
	const glm::vec3 *getPic() const;

	bool queueCanContinue() override;
	void fillPixelBlock(PixelBlock &block) override;
	void computePixels();

private:
	bool isRunning = false;
	bool areThreadStopped = true;

	const int width;
	const int height;
	const uint32_t nbSamples;

	std::chrono::time_point<std::chrono::steady_clock> startTime;

	const HitableCollection &collection;
	const Camera &cam;

	glm::vec3 *pic;

	uint32_t cx = 0;
	uint32_t cy = 0;
	uint32_t cs = 0;
	uint32_t renderedSamples = 0;

	PixelBlockQueue queue;
	std::thread *threads[NBR_THREAD];

	glm::vec3 computeColor(const Ray &ray, const IHitable &world, int depth);
};