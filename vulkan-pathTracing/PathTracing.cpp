#include "PathTracing.h"

#include <chrono>
#include <string>

#include "Camera.h"
#include "ctmRand.h"
#include "HitableCollection.h"
#include "HitRecord.h"
#include "IHitable.h"
#include "LogMessage.h"
#include "Material.h"
#include "PixelBlock.h"
#include "Ray.h"

PathTracing::PathTracing(int width, int height, uint32_t nbSamples, const HitableCollection &collection, const Camera &cam)
	: width(width), height(height), nbSamples(nbSamples), collection(collection), cam(cam)
	, queue(*this)
{
	pic = new glm::vec3[width * height];
	memset(pic, 0, width * height * sizeof(glm::vec3));
}

PathTracing::~PathTracing()
{
	delete[] pic;
}

void PathTracing::startRendering()
{
	isRunning = true;
	areThreadStopped = false;
	for (int i = 0; i < NBR_THREAD; i++)
	{
		threads[i] = new std::thread([this]() { this->computePixels(); });
	}
	startTime = std::chrono::steady_clock::now();
}

void PathTracing::endRendering()
{
	isRunning = false;
	if (!areThreadStopped)
	{
		LOG_MSG("Stopping thread.");
		for (size_t i = 0; i < NBR_THREAD; i++)
		{
			if (threads[i]->joinable())
				threads[i]->join();
			delete threads[i];
		}
	}
}

void PathTracing::retreiveThreadResult()
{
	if (areThreadStopped)
		return;

	PixelBlock block;
	PixelBlockQueue::ReturnType ret;
	while ((ret = queue.getPixelBlockToDraw(&block)) == PixelBlockQueue::ReturnType::SUCCESS)
	{
		for (uint32_t i = 0; i < block.length; i++)
		{
			pic[block.startingPixel + i] = (pic[block.startingPixel + i] * static_cast<float>(block.nbSample) + block.buffer[i])
				/ static_cast<float>(block.nbSample + 1);
		}

#ifdef _DEBUG
		if (block.nbSample > renderedSamples)
		{
			LOG_MSG("%de sample image rendered", renderedSamples + 1);
			renderedSamples += 1;
		}
#endif
	}

	if (!isRunning && !areThreadStopped && ret == PixelBlockQueue::ReturnType::RENDERING_FINISHED)
	{
		LOG_MSG("%de sample image rendered", renderedSamples + 1);

#ifdef _DEBUG
		auto endTime = std::chrono::steady_clock::now();
		uint32_t total = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count());
		uint32_t hours = total / 60 / 60;
		uint32_t minutes = (total / 60) % 60;
		uint32_t seconds = total % 60;

		LOG_MSG("Rendering finished in %uhours %uminutes %useconds", hours, minutes, seconds);
#endif

		for (size_t i = 0; i < NBR_THREAD; i++)
		{
			if (threads[i]->joinable())
				threads[i]->join();
			delete threads[i];
		}
		areThreadStopped = true;
	}
}

const glm::vec3 *PathTracing::getPic() const
{
	return (pic);
}

bool PathTracing::queueCanContinue()
{
	return (isRunning);
}

void PathTracing::fillPixelBlock(PixelBlock &block)
{
	if (cs >= nbSamples || !isRunning)
	{
		isRunning = false;
		return ;
	}

	block.startingPixel = cx + cy * width;
	block.length = PixelBlock::NBR_PIXELS_PER_BLOCK;
	block.nbSample = cs;
	cy = cy + (cx + block.length) / width;
	cx = (cx + block.length) % width;
	if (cy >= static_cast<uint32_t>(height))
	{
		block.length = width * height - block.startingPixel;
		cx = 0;
		cy = 0;
		cs += 1;
	}
}

void PathTracing::computePixels()
{
	PixelBlock *block;
	LOG_MSG("Thread %p started.", __threadid);
	while (queue.getPixelBlockToProcess(&block) != PixelBlockQueue::ReturnType::RENDERING_FINISHED)
	{
		if (!block)
			continue;

		for (uint32_t i = 0; i < block->length; i++)
		{
			int cx = (block->startingPixel + i) % width;
			int cy = (block->startingPixel + i) / width;

			float u = (static_cast<float>(cx) + ctmRand()) / static_cast<float>(width);
			float v = (static_cast<float>(cy) + ctmRand()) / static_cast<float>(height);
			Ray ray = cam.getRay(u, v);

			glm::vec3 color = computeColor(ray, collection, 0);
			block->buffer[i] = color;
		}
		queue.releaseProcessedPixelBlock(block);
	}
	LOG_MSG("Thread %p stopped.", __threadid);
}

glm::vec3 PathTracing::computeColor(const Ray &ray, const IHitable &world, int depth)
{
	HitRecord record;
	if (world.hit(ray, 0.001f, 100.0f, record))
	{
		Ray scattered;
		glm::vec3 attenuation;
		if (depth < MAX_DEPTH && record.material->scatter(ray, record, attenuation, scattered))
		{
			return (attenuation * computeColor(scattered, world, depth + 1));
		}
		else
			return (glm::vec3(0, 0, 0));
	}
	else
	{
		glm::vec3 direction = glm::normalize(ray.getDirection());
		float t = 0.5f * (direction.y + 1);
		return ((1 - t) * glm::vec3(1, 1, 1) + t * glm::vec3(0.5, 0.7, 1));
	}
}