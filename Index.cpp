#include "Index.h"

Index::Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags) {
	this->BUFFSIZE = BUFFSIZE;
	buffer = new IdxRec[BUFFSIZE];
}

void Index::readBlock() {
	file->seekg(r_ptr);
	uint32_t bytesRead = file->read((char*)buffer, sizeof(IdxRec)* BUFFSIZE).gcount();
	if (bytesRead < sizeof(IdxRec) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(IdxRec) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	r_ptr += bytesRead;
}

void Index::writeBlock(const char* block)
{
}

uint16_t Index::findPage(uint16_t key)
{
	return 0;
}

void Index::writeIdxRecord(IdxRec rec)
{
}

void Index::printIndex()
{
}

void Index::clearFile()
{
}

Index::~Index()
{
}
