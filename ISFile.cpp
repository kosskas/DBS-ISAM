#include "ISFile.h"

ISFile::ISFile(uint32_t BUFFSIZE, string filename, ios_base::openmode flags)
{
}

int ISFile::readBlock()
{
	return 0;
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
