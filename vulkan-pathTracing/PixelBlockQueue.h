#pragma once

#include <glm/glm.hpp>

#include <mutex>

#include "IPixelBlockQueueOwner.h"
#include "PixelBlock.h"

class PixelBlockQueue
{
public:
	static constexpr uint16_t NBR_BLOCKS = 4;

	enum class ReturnType
	{
		SUCCESS,
		NO_BLOCK_AVAILABLE,
		RENDERING_FINISHED
	};

	PixelBlockQueue(IPixelBlockQueueOwner &owner);

	ReturnType getPixelBlockToProcess(PixelBlock **block);
	void releaseProcessedPixelBlock(PixelBlock *block);

	ReturnType getPixelBlockToDraw(PixelBlock *block);

private:

#define INC(a) a + 1 < NBR_BLOCKS ? a + 1 : 0 

	IPixelBlockQueueOwner &owner;

	PixelBlock blocks[NBR_BLOCKS];

	std::mutex locker;
};