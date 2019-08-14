#include "PixelBlockQueue.h"

PixelBlockQueue::PixelBlockQueue(IPixelBlockQueueOwner &owner)
	: owner(owner)
{}

PixelBlockQueue::ReturnType PixelBlockQueue::getPixelBlockToProcess(PixelBlock **block)
{
	*block = nullptr;
	ReturnType ret = ReturnType::NO_BLOCK_AVAILABLE;
	
	locker.lock();
	if (!owner.queueCanContinue())
	{
		locker.unlock();
		return (ReturnType::RENDERING_FINISHED);
	}
	for (size_t i = 0; i < NBR_BLOCKS; i++)
	{
		if (!blocks[i].isWaitingForHarvest && !blocks[i].isProcessed)
		{
			*block = &blocks[i];
			owner.fillPixelBlock(blocks[i]);
			blocks[i].isProcessed = true;
			ret = ReturnType::SUCCESS;
			break;
		}
	}
	locker.unlock();
	return (ret);
}

void PixelBlockQueue::releaseProcessedPixelBlock(PixelBlock *block)
{
	locker.lock();
	block->isProcessed = false;
	block->isWaitingForHarvest = true;
	locker.unlock();
}

PixelBlockQueue::ReturnType PixelBlockQueue::getPixelBlockToDraw(PixelBlock *block)
{
	bool isJobProcessing = false;

	locker.lock();
	for (int16_t i = 0; i < NBR_BLOCKS; i++)
	{
		if (blocks[i].isWaitingForHarvest)
		{
			memcpy(block, &blocks[i], sizeof(PixelBlock));
			blocks[i].isWaitingForHarvest = false;
			locker.unlock();
			return (ReturnType::SUCCESS);
		}
		else if (blocks[i].isProcessed)
			isJobProcessing = true;
	}
	locker.unlock();
	if (isJobProcessing)
		return (ReturnType::NO_BLOCK_AVAILABLE);
	return (ReturnType::RENDERING_FINISHED);
}