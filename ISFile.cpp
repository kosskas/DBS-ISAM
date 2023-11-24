#include "ISFile.h"

ISFile::ISFile(string filename, uint32_t BUFFSIZE, ios_base::openmode flags)
{
}

void ISFile::readBlock()
{
}

void ISFile::writeBlock(const char* block)
{
}

Record ISFile::searchRecord(uint16_t key)
{
	return Record();
}

Record ISFile::insertRecord(uint16_t key, Data data)
{
	return Record();
}

Record ISFile::removeRecord(uint16_t key)
{
	return Record();
}
