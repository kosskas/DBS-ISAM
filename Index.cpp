#include "Index.h"

Index::Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags) : Buffered(filename, flags){
	this->BUFFSIZE = BUFFSIZE;
	buffer = new IdxRec[BUFFSIZE];
	memset(buffer, 0, sizeof(IdxRec) * BUFFSIZE);
}

int Index::readBlock() {
	file->seekg(r_ptr);
	int bytesRead = file->read((char*)buffer, sizeof(IdxRec)* BUFFSIZE).gcount();
	if (bytesRead < sizeof(IdxRec) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(IdxRec) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	r_ptr += bytesRead;
	return bytesRead;
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

int Index::findPage(int key) {
	r_ptr = 0;
	int bytesRead = 0;
	int poc, kon;
	int iPage = 0;
	int iKey = 0;
	do {
		bytesRead = readBlock();
		int n = bytesRead / sizeof(IdxRec);
		for (int i = 0; i < n; i++) {
			if (key >= buffer[i].key) {
				iKey = buffer[i].key;
				iPage = buffer[i].page;
				if (buffer[i].key == key) {
					printf("Klucz %d powinien byc key=%d,page=%d\n", key, iKey, iPage);
					return iPage;
				}
			}
		}		
	} while (bytesRead != 0);
	printf("Klucz %d powinien byc key=%d,page=%d\n", key, iKey, iPage);
	return iPage;
	//je¿eli 0 to nie ma bo klucz < najmniejszy
}

void Index::writeIdxRecord(IdxRec rec)
{
	//wyszukaj czy taki jest
	//jeœli jest to zg³oœ lub zamieñ
	//jeœli nie to na koniec
}

void Index::printIndex() {
	r_ptr = 0;
	printf("KEY -- PAGE\n");
	int bytesRead = 0;
	do {
		bytesRead = readBlock();
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < bytesRead/sizeof(IdxRec); i++) {
			printf("%d -- %d\n", buffer[i].key, buffer[i].page);
		}
	} while (bytesRead != 0);
}

Index::~Index()
{
	delete buffer;
}
