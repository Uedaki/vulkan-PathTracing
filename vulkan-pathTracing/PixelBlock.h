#pragma once

#include <glm/glm.hpp>

#include <stdint.h>

class PixelBlockQueue;

struct PixelBlock
{
	static constexpr uint32_t NBR_PIXELS_PER_BLOCK = 512;

	uint32_t startingPixel = 0;
	uint32_t length = 0;
	uint32_t nbSample = 0;
	glm::vec3 buffer[NBR_PIXELS_PER_BLOCK] = {};

private:
	friend class PixelBlockQueue;
	bool isWaitingForHarvest = false;
	bool isProcessed = false;
};