#include "ISFile.h"

ISFile::ISFile(uint32_t BUFFSIZE) : Buffered("file", ios::binary | ios::in | ios::out | ios::trunc) {
	buffer = NULL;
	this->BUFFSIZE = BUFFSIZE;
	buffer = new Record[BUFFSIZE];
	memset(buffer, 0, sizeof(Record) * BUFFSIZE);
	//Specjalny klucz
	buffer[0].key = -1;
	buffer[0].data = {-1,-1,-1};
	writeBlock(0);



	idx = NULL;
	idx = new Index(BUFFSIZE, "idx", ios::binary | ios::in | ios::out | ios::trunc);
	createIndex();
	bf = int((sizeof(Record) * BUFFSIZE) / sizeof(Record));
	bi = int((sizeof(Record) * BUFFSIZE) / (sizeof(int) + sizeof(short int)));
	printf("bf = %d\nbi = %d\n", bf, bi);

}

int ISFile::readBlock(int blockNum) {
	file->seekg(blockNum * (sizeof(Record) * BUFFSIZE));
	int bytesRead = file->read((char*)buffer, sizeof(Record) * BUFFSIZE).gcount();
	if (bytesRead < sizeof(Record) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(Record) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	//r_ptr += bytesRead;

	return bytesRead;
}


int ISFile::writeBlock(int blockNum) {
	const char* serialRec = (const char*)buffer;

	file->seekp(blockNum * (sizeof(Record) * BUFFSIZE));
	size_t poc = file->tellp();
	file->write(serialRec, sizeof(Record) * BUFFSIZE);
	size_t written = file->tellp();
	written = written - poc;
	printf("Zapisano %d\n", written);
	//w_ptr += sizeof(Record) * BUFFSIZE;
	return written;
}

int ISFile::searchRecord(int key, int* found) {
	resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	//PAGE MO¯E BYÆ 0!! bo znalaz³
	int page = idxpage * (sizeof(Record) * BUFFSIZE);
	int bytesRead = readBlock(page);
	for (int i = 0; i < BUFFSIZE; i++) {
		if (buffer[i].key == key) {
			exists = 1;
		}
			
	}
	*found = exists;
	return page;
}

void ISFile::insertRecord(int key, Data data) {

	/*
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
				reorganiseFile(0.5);
				//insertRecord(key, data);
				return;
			}
		}
	}
	*/
	//przebieg gdy jest index i s¹ strony

	//przypadek gdy wstawiasz na koniec i sortujesz i 
	int found = 0;
	int page = searchRecord(key,&found);
	int bytesRead = readBlock(page);
	/*
	int nInPage = 0;
	for (int i = 0; i < BUFFSIZE; i++) {
		if (buffer[i].key == 0)
			break;
		nInPage++;
	}
	*/
	///znajdz miejsce
	for (int i = 0; i < BUFFSIZE; i++) {
		if (i + 1 <= BUFFSIZE && buffer[i].key < key && buffer[i+1].key == 0) {
			printf("znaleziono miejsce\n");
			buffer[i + 1].key = key;
			buffer[i + 1].data = data;
			//w_ptr = page;
			writeBlock(page);
			return;
		}
		if (i + 1 <= BUFFSIZE && buffer[i].key < key && buffer[i + 1].key > key) {
			//nie ma mniejsca
			printf("daj OV");
			return;
		}
		if (i + 1 < BUFFSIZE && buffer[i].key > key) {
			///nie ma mniejsca i jest za ostatnim
			printf("daj OV");
			return;
		}
	}
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
	while (bytesRead = readBlock(page)) {
		//weŸ pierwszy i go zapisz do indeksu
		idx->writeIdxRecord(buffer[0].key, page++);
	}
	//idx.resetPtr
	///
}

void ISFile::reorganiseFile(double alpha) {
	printf("reorg");

}

void ISFile::clearFile() {
	Buffered::clearFile();
	idx->clearFile();
	firstWrite = true;
	wToBufferF = 1;
	buffer[0].key = -1;
	buffer[0].data = { -1,-1,-1 };
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
	int page = 0;
	while (bytesRead = readBlock(page++)) {
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