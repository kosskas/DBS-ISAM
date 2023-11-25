#include "ISFile.h"

ISFile::ISFile(uint32_t BUFFSIZE) : Buffered("file", ios::binary | ios::in | ios::out | ios::trunc) {
	buffer = NULL;
	this->BUFFSIZE = BUFFSIZE;
	buffer = new Record[BUFFSIZE];
	memset(buffer, 0, sizeof(Record) * BUFFSIZE);
	
	firstWrite = true;
	wToBufferF = 0;
	idx = NULL;
	idx = new Index(BUFFSIZE, "idx", ios::binary | ios::in | ios::out | ios::trunc);
	//createIndex();
	printf("bf = %d\nbi = %d\n",
		int((sizeof(Record) * BUFFSIZE) / sizeof(Record)),
		int((sizeof(Record) * BUFFSIZE) / (sizeof(int) + sizeof(int))));

}

int ISFile::readBlock() {
	file->seekg(r_ptr);
	int bytesRead = file->read((char*)buffer, sizeof(Record) * BUFFSIZE).gcount();
	if (bytesRead < sizeof(Record) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(Record) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	r_ptr += bytesRead;

	return bytesRead;
}


void ISFile::writeBlock() {
	const char* serialRec = (const char*)buffer;
	file->seekp(w_ptr);
	size_t poc = file->tellp();
	file->write(serialRec, sizeof(Record) * BUFFSIZE);
	size_t written = file->tellp();
	written = written - poc;
	printf("Zapisano %d\n", written);
	w_ptr += sizeof(Record) * BUFFSIZE;
}

int ISFile::searchRecord(int key) {
	resetPtr();
	int idxpage = idx->readIdxRecord(key);

	//PAGE MO¯E BYÆ 0!! bo znalaz³
	int page = idxpage * (sizeof(Record) * BUFFSIZE);
	r_ptr = page;
	int bytesRead = readBlock();
	for (int i = 0; i < BUFFSIZE; i++) {
		if (buffer[i].key == key)
			return page;
	}
	if (page == 0 && buffer[0].key > key) {
		//je¿eli 0 to jest mniejszy ni¿ idx na 1 stronie
		//istnieje strona
		//printf("Mniejszy niz");
		return BEFOREZERO;
	}
	return NOTFOUND;//pageno;
}

void ISFile::insertRecord(int key, Data data) {
	//-dopóki nie zape³nimy 1 strony przy 1 wpisywaniu trzymamy j¹ w RAM
	//znajdz czy jest
	//jesli nie ma i jest miejsce to wstaw
	//jesli jest na poczatku to zamien idx i statyklucz daj na of
	//jesli nie ma miejsca to daj na of
	if (firstWrite) {
		int of = 0;
		for (int i = 0; i < BUFFSIZE; i++) {
			if (buffer[i].key == key) {
				//klucz juz jest
				printf("Taki klucz juz jest\n");
				return;
			}
			if (buffer[i].key == 0 && i + 1 <= BUFFSIZE) {
				//ok, znajdz wolne miejce i posortuj po kluczu
				buffer[i].key = key;
				buffer[i].data = data;
				wToBufferF++;
				sort(buffer, buffer + wToBufferF, [](Record r1, Record r2) {return r1.key < r2.key; });
				printf("Zapisano\n");
				return;
			}
			else if (buffer[i].key != 0 && i + 1 >= BUFFSIZE) {
				//full, reorg i wstaw jeszcze raz
				firstWrite = false;
				reorganiseFile();
				//insertRecord(key, data);
				return;
			}
		}
	}
	//przebieg gdy jest index i s¹ strony
	
}

void ISFile::removeRecord(int key) {
	return;
}

void ISFile::insertToOf(int key, Data data) {
	return;
}



void ISFile::createIndex() {
	resetPtr();
	int bytesRead = 0;
	int page = 0;
	while (bytesRead = readBlock()) {
		//weŸ pierwszy i go zapisz do indeksu
		idx->writeIdxRecord(buffer[0].key, page++);
	}
	//idx.resetPtr
}

void ISFile::reorganiseFile() {
	printf("reorg");
}

void ISFile::clearFile() {
	Buffered::clearFile();
	idx->clearFile();
	firstWrite = true;
}

void ISFile::printRecords() {
	//print in order
	//czyli w kolejnoœci nawet przechodz¹c po OF
}

void ISFile::printStruct() {
	if (firstWrite) {
		printBuffer();
		return;
	}
	resetPtr();
	printf("KEY\tDATA\tOF\n");
	int bytesRead = 0;
	while (bytesRead = readBlock()) {
		printf("\tPrzeczytano %d\n", bytesRead);
		printBuffer();
	}
}

void ISFile::printOF() {

}

void ISFile::printBuffer() {
	for (int i = 0; i < BUFFSIZE; i++) {
		printf("%d\t{%d,%d,%d}\t%d\n", buffer[i].key, buffer[i].data.a, buffer[i].data.b, buffer[i].data.h, buffer[i].ofptr);
	}
}

ISFile::~ISFile() {
	if (idx)
		delete idx;
	if (buffer)
		delete[] buffer;
}