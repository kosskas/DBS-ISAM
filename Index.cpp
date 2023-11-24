#include "Index.h"

Index::Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags) {
}

void Index::readBlock()
{
}

void Index::writeBlock(const char* block)
{
}

uint16_t Index::findPage(uint16_t key) {
	int size = 0;
	for (IdxRec* i = buffer; i->key != 0; i++) {
		size++;
	}
	r_ptr = 0;
	while (!buffer[0].key) {
		readBlock();
		///to wskazuje stronê gdzie POWINIEN byæ klucz!
		//wysz binarne
		for (IdxRec* i = buffer; i->key != 0; i++) {
			if (i->key == key)
				return i->page;
		}
	}
	return NOTFOUND;
}

void Index::writeIdxRecord(IdxRec rec) {

void Index::printIndex()
{
}

void Index::printIndex() {
	r_ptr = 0;
	printf("KEY -- PAGE\n");
	while (!buffer[0].key) {
		readBlock();
		for (IdxRec* i = buffer; i->key != 0; i++) {
			printf("%d -- %d\n", i->key, i->page);
		}
	}
}

Index::~Index() {
	delete buffer;
}
