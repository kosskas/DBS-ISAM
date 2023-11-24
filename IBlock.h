#pragma once
class Block {
protected:
	virtual void ReadBlock() = 0;
	virtual void WriteBlock(const char* block) = 0;
};