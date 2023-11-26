#include "Index.h"

Index::Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags) {
	file = new fstream();
	file->open(filename, flags);

	buffer = NULL;
	this->BUFFSIZE = BUFFSIZE;
	buffer = new IdxRec[BUFFSIZE];
	memset(buffer, 0, sizeof(IdxRec) * BUFFSIZE);
}

int Index::readBlock(int blockNum) {
	file->seekg(blockNum * (sizeof(IdxRec) * BUFFSIZE));
	int bytesRead = file->read((char*)buffer, sizeof(IdxRec)* BUFFSIZE).gcount();
	if (bytesRead < sizeof(IdxRec) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(IdxRec) * BUFFSIZE - bytesRead); //je�li przeczytano mniej ni� ca�� stron�, wyzeruj dalsze
	}
	//nOfBuff = bytesRead / sizeof(IdxRec);
	return bytesRead;
	//gdy przeczytano za ca�y plik bR=0, buf=0,0...
}

int Index::writeBlock(int blockNum) {
	//DOPOP
	//sprawdz ilo�� w buforze
	const char* serialRec = (const char*)buffer;
	file->seekp(blockNum*(sizeof(IdxRec) * BUFFSIZE));
	size_t poc = file->tellp();
	file->write(serialRec, sizeof(IdxRec) * BUFFSIZE);
	size_t written = file->tellp();
	written = written - poc;
	printf("Zapisano %d\n", written);
	//w_ptr += sizeof(IdxRec) * BUFFSIZE;
	return written;
}

//readIdxRecord
int Index::readIdxRecord(int key) {
	resetPtr();
	int bytesRead = 0;
	int iPage = 0;
	int iKey = 0;
	int page = 0;
	do {
		bytesRead = readBlock(page++);
		for (int i = 0; i < BUFFSIZE; i++) {
			if (key >= buffer[i].key && buffer[i].key != 0) {
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
	//je�eli 0 to nie ma bo klucz < najmniejszy
}

void Index::writeIdxRecord(int key, int page) {

	//wyszukaj czy taki jest
	//je�li jest to zg�o� lub zamie� (przypadek gdy podmieniasz pierwsze rekordy w stronei)
	// ???
	//je�li nie to na koniec (chyba nie bo reorg nast�pi wcze�niej) chyba �e to pos�u�y do reorg, bo wpisujemy ka�dy wi�kszy od drugiego
	resetPtr();
	//znajdz wolne miejsce
	int bytesRead = 0;
	int pageNum = 0;
	while (true) {
		bytesRead = readBlock(pageNum++);
		for (int i = 0; i < BUFFSIZE; i++) {
			if (buffer[i].key == key) {
				printf("Taki klucz juz jest\n");
				return;
			}
			if (buffer[i].key == 0) {
				buffer[i].key = key;
				buffer[i].page = page;
				//nOfBuff++;
				file->clear();
				writeBlock(pageNum-1);
				return;
			}
		}
	}
}

void Index::swapKey(int odlKey, int key) {
	resetPtr();
	int bytesRead = 0;
	int iPage = 0;
	int iKey = 0;
	int prev = 0;
	int pageNum = 0;
	while (true) {
		bytesRead = readBlock(pageNum++);
		for (int i = 0; i < BUFFSIZE; i++) {
			if (buffer[i].key == odlKey) {
				buffer[i].key = key;
				file->clear();
				writeBlock(pageNum-1);
				return;
			}
			if (buffer[i].key == 0) {
				return;
			}
		}
	}
}

void Index::printIndex() {
	resetPtr();
	printf("KEY\tPAGE\n");
	int bytesRead = 0;
	int page = 0;
	while(bytesRead = readBlock(page++)){
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d\t%d\n", buffer[i].key, buffer[i].page);
		}
	}
}

void Index::resetPtr() {
	file->clear();
	file->seekg(0, ios::beg);
	file->seekp(0, ios::beg);
}

void Index::clearFile() {
	file->close();
	file->open(filename, flags | ios::trunc);
	resetPtr();
}

Index::~Index() {
	if(buffer)
		delete[] buffer;
	if (file != NULL) {
		file->close();
		delete file;
	}
}
