#pragma once

struct PixelBlock;

class IPixelBlockQueueOwner
{
public:
	virtual bool queueCanContinue() = 0;
	virtual void fillPixelBlock(PixelBlock &block) = 0;
};