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

void Index::writeBlock(const char* block) {
	//sprawdz iloœæ w buforze
	int size = 0;
	for (IdxRec* i = buffer; i->key != 0; i++) {
		size++;
	}
	file->seekp(w_ptr);
	file->write(block, sizeof(IdxRec) * size);
	w_ptr += sizeof(IdxRec) * size;
}

uint16_t Index::findPage(uint16_t key) {
	r_ptr = 0;
	bool found = false;
	while (!buffer[0].key) {
		readBlock();
		for (IdxRec* i = buffer; i->key != 0; i++) {
			if (i->key == key)
				return i->page;
		}
	}
	return NOTFOUND;
}

void Index::writeIdxRecord(IdxRec rec)
{

}

void Index::printIndex() {
	r_ptr = 0;
	bool found = false;
	printf("KEY -- PAGE\n");
	while (!buffer[0].key) {
		readBlock();
		for (IdxRec* i = buffer; i->key != 0; i++) {
			printf("%d -- %d\n", i->key, i->page);
		}
	}
}

Index::~Index()
{
	delete buffer;
}
