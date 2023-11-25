#include "Index.h"

Index::Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags) : Buffered(filename, flags){
	buffer = NULL;
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
	nOfBuff = bytesRead / sizeof(IdxRec);
	return bytesRead;
	//gdy przeczytano za ca³y plik bR=0, buf=0,0...
}

void Index::writeBlock() {
	//DOPOP
	//sprawdz iloœæ w buforze
	const char* serialRec = (const char*)buffer;
	file->seekp(w_ptr);
	size_t poc = file->tellp();
	file->write(serialRec, sizeof(IdxRec) * BUFFSIZE);
	size_t written = file->tellp();
	written = written - poc;
	printf("Zapisano %d\n", written);
	w_ptr += sizeof(IdxRec) * BUFFSIZE;
	
}

//readIdxRecord
int Index::readIdxRecord(int key) {
	resetPtr();
	int bytesRead = 0;
	int iPage = 0;
	int iKey = 0;
	do {
		bytesRead = readBlock();
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
	while (true) {
		bytesRead = readBlock();
		for (int i = 0; i < BUFFSIZE; i++) {
			if (buffer[i].key == key) {
				printf("Taki klucz juz jest\n");
				return;
			}
			if (buffer[i].key == 0) {
				buffer[i].key = key;
				buffer[i].page = page;
				nOfBuff++;
				w_ptr = r_ptr - bytesRead;
				file->clear();
				writeBlock();
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
	while (true) {
		bytesRead = readBlock();
		for (int i = 0; i < BUFFSIZE; i++) {
			if (buffer[i].key == odlKey) {
				buffer[i].key = key;
				w_ptr = r_ptr - bytesRead;
				file->clear();
				writeBlock();
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
	printf("KEY -- PAGE\n");
	int bytesRead = 0;
	while(bytesRead = readBlock()){
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d -- %d\n", buffer[i].key, buffer[i].page);
		}
	}
}

Index::~Index() {
	if(buffer)
		delete buffer;
}
