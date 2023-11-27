#include "ISFile.h"

ISFile::ISFile(uint32_t BUFFSIZE) {
	filename = "file";
	flags = ios::binary | ios::in | ios::out | ios::trunc;
	this->BUFFSIZE = BUFFSIZE;

	file = new fstream();
	file->open(filename, flags);
	
	
	//Specjalny klucz
	initBuffers();
	Mbuffer[0].key = -1;
	Mbuffer[0].data = {0,0,0};
	writeBlock(0, Mbuffer);
	createOF(1);


	idx = NULL;
	idx = new Index(BUFFSIZE, "idx", flags);
	createIndex();
	bf = int((sizeof(Record) * BUFFSIZE) / sizeof(Record));
	bi = int((sizeof(Record) * BUFFSIZE) / (sizeof(int) + sizeof(short int)));
	printf("bf = %d\nbi = %d\n", bf, bi);

}

void ISFile::initBuffers() {
	Mbuffer = NULL;
	Mbuffer = new Record[BUFFSIZE];
	memset(Mbuffer, 0, sizeof(Record) * BUFFSIZE);

	ofbuffer = NULL;
	ofbuffer = new Record[BUFFSIZE];
	memset(ofbuffer, 0, sizeof(Record) * BUFFSIZE);

}

int ISFile::readBlock(int blockNum, Record* buffer) {
	file->seekg(blockNum * (sizeof(Record) * BUFFSIZE));
	int bytesRead = file->read((char*)buffer, sizeof(Record) * BUFFSIZE).gcount();
	if (bytesRead < sizeof(Record) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(Record) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	//r_ptr += bytesRead;

	return bytesRead;
}


int ISFile::writeBlock(int blockNum, Record* buffer) {
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
	int bytesRead = readBlock(page, Mbuffer);

	//sprwadz czy nie jest w of
	for (int i = 0; i < BUFFSIZE; i++) {
		if (Mbuffer[i].key == key) {
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
	int bytesRead = readBlock(page, Mbuffer);

	///znajdz miejsce
	for (int i = 0; i < BUFFSIZE; i++) {
		if (i + 1 < BUFFSIZE && Mbuffer[i].key < key && Mbuffer[i+1].key == 0) {
			printf("znaleziono miejsce\n");
			Mbuffer[i + 1].key = key;
			Mbuffer[i + 1].data = data;
			//w_ptr = page;
			writeBlock(page, Mbuffer);
			return;
		}
		if (i + 1 < BUFFSIZE && Mbuffer[i].key < key && Mbuffer[i + 1].key > key) {
			//nie ma mniejsca
			printf("daj OV");
			insertToOf(key, data, &Mbuffer[i].ofptr);
			writeBlock(page, Mbuffer);
			return;
		}
		else if (i + 1 >= BUFFSIZE) {
			///nie ma mniejsca i jest za ostatnim
			printf("za ostatnim daj OV");

			///lista kolejnch wskaŸników
			insertToOf(key, data,&Mbuffer[i].ofptr);
			writeBlock(page, Mbuffer);
			return;
		}
	}
}

void ISFile::removeRecord(int key) {
	//oznacz jako del
	return;
}

void ISFile::insertToOf(int key, Data data,short int* ptr) {
	
	int page = ofBlocNo;
	int bytesRead = 0;
	///sprawdz czy juz inny z ov nie wskazuje na niego?
	///jeœli of pe³en to reogranizuj
	int offset = 0;
	while (bytesRead = readBlock(page++, ofbuffer)) {
		//znajdz czy taki jest
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (ofbuffer[i].key == 0) {
				printf("znaleziono miejsce\n");
				ofbuffer[i].key = key;
				ofbuffer[i].data = data;
				writeBlock(page-1, ofbuffer);
				*ptr = offset;
				return;
			}
		}
	}
}

void ISFile::createIndex() {
	resetPtr();
	int bytesRead = 0;
	int page = 0;
	while (page != ofBlocNo) {
		bytesRead = readBlock(page, Mbuffer);
		//weŸ pierwszy i go zapisz do indeksu
		idx->writeIdxRecord(Mbuffer[0].key, page++);
	}
	//idx.resetPtr
	///
}

void ISFile::createOF(int blockNo) {
	ofBlocNo = blockNo;
	memset(ofbuffer, 0, sizeof(Record) * BUFFSIZE);
	writeBlock(ofBlocNo, ofbuffer);
	
}

void ISFile::reorganiseFile(double alpha) {
	printf("reorg");


}

void ISFile::clearFile() {
	file->close();
	file->open(filename, flags | ios::trunc);
	resetPtr();
	idx->clearFile();
	Mbuffer[0].key = -1;
	Mbuffer[0].data = { -1,-1,-1 };
	writeBlock(0, Mbuffer);
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
		bytesRead = readBlock(page++, Mbuffer);
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d\t{%d,%d,%d}\t%d\t%d\n", Mbuffer[i].key, Mbuffer[i].data.a, Mbuffer[i].data.b, Mbuffer[i].data.h, Mbuffer[i].deleted, Mbuffer[i].ofptr);
		}
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
	while (bytesRead = readBlock(page++, ofbuffer)){ 
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d\t{%d,%d,%d}\t%d\t%d\n", ofbuffer[i].key, ofbuffer[i].data.a, ofbuffer[i].data.b, ofbuffer[i].data.h, ofbuffer[i].deleted, ofbuffer[i].ofptr);
		}
	}
}

void ISFile::printBuffer() {
	for (int i = 0; i < BUFFSIZE; i++) {
		printf("%d\t{%d,%d,%d}\t%d\t%d\n", Mbuffer[i].key, Mbuffer[i].data.a, Mbuffer[i].data.b, Mbuffer[i].data.h, Mbuffer[i].deleted, Mbuffer[i].ofptr);
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
	if (Mbuffer)
		delete[] Mbuffer;
	if (ofbuffer)
		delete[] ofbuffer;
	if (file != NULL) {
		file->close();
		delete file;
	}
}