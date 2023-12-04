#include "Index.h"

Index::Index(int32_t BUFFSIZE,int nOfpages, string filename, ios_base::openmode flags) {
	this->flags = flags;
	buffer = NULL;
	this->BUFFSIZE = BUFFSIZE;
	buffer = new IdxRec[BUFFSIZE];
	memset(buffer, 0, sizeof(IdxRec) * BUFFSIZE);
	file = createFile(filename, nOfpages);
}

fstream* Index::createFile(string fileName, int nOfpages) {
	fstream* f = new fstream();
	f->open(fileName, flags);
	int page = 0;
	memset(buffer, 0, sizeof(IdxRec) * BUFFSIZE);
	while (nOfpages--) {
		writeBlock(f,page++);
	}
	return f;
}
int Index::readBlock(fstream* currfile, int blockNum) {
	currfile->seekg(blockNum * (sizeof(IdxRec) * BUFFSIZE));
	int bytesRead = currfile->read((char*)buffer, sizeof(IdxRec)* BUFFSIZE).gcount();
	if (bytesRead < sizeof(IdxRec) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(IdxRec) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	//nOfBuff = bytesRead / sizeof(IdxRec);
	return bytesRead;
	//gdy przeczytano za ca³y plik bR=0, buf=0,0...
}

int Index::writeBlock(fstream* currfile, int blockNum) {
	//DOPOP
	//sprawdz iloœæ w buforze
	const char* serialRec = (const char*)buffer;
	currfile->seekp(blockNum*(sizeof(IdxRec) * BUFFSIZE));
	size_t poc = currfile->tellp();
	currfile->write(serialRec, sizeof(IdxRec) * BUFFSIZE);
	size_t written = currfile->tellp();
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
		bytesRead = readBlock(file,page++);
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
	//je¿eli 0 to nie ma bo klucz < najmniejszy
}

void Index::writeIdxRecord(int key, int page) {

	//wyszukaj czy taki jest
	//jeœli jest to zg³oœ lub zamieñ (przypadek gdy podmieniasz pierwsze rekordy w stronei)
	// ???
	//jeœli nie to na koniec (chyba nie bo reorg nast¹pi wczeœniej) chyba ¿e to pos³u¿y do reorg, bo wpisujemy ka¿dy wiêkszy od drugiego
	resetPtr();
	//znajdz wolne miejsce
	int bytesRead = 0;
	int pageNum = 0;
	while (true) {
		bytesRead = readBlock(file, pageNum);
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
				writeBlock(file, pageNum);
				return;
			}
		}
		pageNum++;
	}
}


void Index::printIndex() {
	resetPtr();
	printf("KEY\tPAGE\n");
	int bytesRead = 0;
	int page = 0;
	while(bytesRead = readBlock(file, page++)){
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

Index::~Index() {
	if(buffer)
		delete[] buffer;
	if (file != NULL) {
		file->close();
		delete file;
	}
}
