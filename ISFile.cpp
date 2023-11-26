#include "ISFile.h"

ISFile::ISFile(uint32_t BUFFSIZE) {
	file = new fstream();
	file->open("file", ios::binary | ios::in | ios::out | ios::trunc);
	
	buffer = NULL;
	this->BUFFSIZE = BUFFSIZE;
	buffer = new Record[BUFFSIZE];
	memset(buffer, 0, sizeof(Record) * BUFFSIZE);
	//Specjalny klucz
	buffer[0].key = -1;
	buffer[0].data = {-1,-1,-1};
	writeBlock(0);
	createOF(1);


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

	//sprwadz czy nie jest w of
	for (int i = 0; i < BUFFSIZE; i++) {
		if (buffer[i].key == key) {
			exists = 1;
		}
	}
	*found = exists;
	return page;
}

void ISFile::insertRecord(int key, Data data) {
	//przebieg gdy jest index i s¹ strony
	int found = 0;
	int page = searchRecord(key,&found);
	int bytesRead = readBlock(page);

	///znajdz miejsce
	for (int i = 0; i < BUFFSIZE; i++) {
		if (i + 1 < BUFFSIZE && buffer[i].key < key && buffer[i+1].key == 0) {
			printf("znaleziono miejsce\n");
			buffer[i + 1].key = key;
			buffer[i + 1].data = data;
			//w_ptr = page;
			writeBlock(page);
			return;
		}
		if (i + 1 < BUFFSIZE && buffer[i].key < key && buffer[i + 1].key > key) {
			//nie ma mniejsca
			printf("daj OV");
			buffer[i].ofptr = insertToOf(key, data);
			return;
		}
		else if (i + 1 >= BUFFSIZE) {
			///nie ma mniejsca i jest za ostatnim
			printf("za ostatnim daj OV");
			buffer[i].ofptr = insertToOf(key, data);
			return;
		}
	}
}

void ISFile::removeRecord(int key) {
	//oznacz jako del
	return;
}

int ISFile::insertToOf(int key, Data data) {
	
	int page = ofBlocNo;
	int bytesRead = 0;
	///sprawdz czy juz inny z ov nie wskazuje na niego?
	///jeœli of pe³en to reogranizuj
	int offset = 0;
	while (bytesRead = readBlock(page++)) {
		//znajdz czy taki jest
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (buffer[i].key != 0) {
				printf("znaleziono miejsce\n");
				buffer[i + 1].key = key;
				buffer[i + 1].data = data;
				writeBlock(page-1);
				return offset;
			}
		}
	}
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

void ISFile::createOF(int blockNo) {
	ofBlocNo = blockNo;
	memset(buffer, 0, sizeof(Record) * BUFFSIZE);
	writeBlock(ofBlocNo);
	
}

void ISFile::reorganiseFile(double alpha) {
	printf("reorg");


}

void ISFile::clearFile() {
	file->close();
	file->open(filename, flags | ios::trunc);
	resetPtr();
	idx->clearFile();
	buffer[0].key = -1;
	buffer[0].data = { -1,-1,-1 };
	writeBlock(0);
	createIndex();

}

void ISFile::printRecords() {
	//print in order
	//czyli w kolejnoœci nawet przechodz¹c po OF
}

void ISFile::printStruct() {
	resetPtr();
	printf("KEY\tDATA\tDEL\tOF\n");
	int bytesRead = 0;
	int page = 0;
	while (page != ofBlocNo) {
		bytesRead = readBlock(page++);
		printf("\tPrzeczytano %d\n", bytesRead);
		printBuffer();
	}
	printOF();
}

void ISFile::printOF() {
	resetPtr();
	printf("===============\n");
	printf("OVERFLOW AREA\n");
	printf("KEY\tDATA\tDEL\tOF\n");
	int bytesRead = 0;
	int page = ofBlocNo;
	while (bytesRead = readBlock(page++)){ 
		printf("\tPrzeczytano %d\n", bytesRead);
		printBuffer();
	}
}

void ISFile::printBuffer() {
	for (int i = 0; i < BUFFSIZE; i++) {
		printf("%d\t{%d,%d,%d}\t%d\t%d\n", buffer[i].key, buffer[i].data.a, buffer[i].data.b, buffer[i].data.h, buffer[i].deleted, buffer[i].ofptr);
	}
}

void ISFile::resetPtr() {
	file->clear();
	file->seekg(0, ios::beg);
	file->seekp(0, ios::beg);
}

ISFile::~ISFile() {
	if (idx)
		delete idx;
	if (buffer)
		delete[] buffer;
	if (file != NULL) {
		file->close();
		delete file;
	}
}