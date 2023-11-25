#include "ISFile.h"

ISFile::ISFile(uint32_t BUFFSIZE, string filename, ios_base::openmode flags) : Buffered(filename, flags) {
	buffer = NULL;
	this->BUFFSIZE = BUFFSIZE;
	buffer = new Record[BUFFSIZE];
	memset(buffer, 0, sizeof(Record) * BUFFSIZE);
	///zlokalizuj OF
	///moze byc inny buffsize
	idx = NULL;
	idx = new Index(BUFFSIZE, "idx1", flags | ios::trunc);
	CreateIndex();
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
	//je¿eli 0 to jest mniejszy ni¿ idx na 1 stronie
	if (idxpage == 0)
		return 0;
	//istnieje strona
	int page = idxpage * BUFFSIZE;
	r_ptr = page;
	int bytesRead = readBlock();
	for (int i = 0; i < BUFFSIZE; i++) {
		if (buffer->key == key)
			return page;
	}
	return NOTFOUND;//pageno;
}

Record ISFile::insertRecord(int key, Data data) {
	return Record();
}

Record ISFile::removeRecord(int key) {
	return Record();
}

void ISFile::reorganiseFile() {

}

void ISFile::clearFile() {
	Buffered::clearFile();
	idx->clearFile();
}

void ISFile::printRecords() {

}

void ISFile::printStruct() {
	resetPtr();
	printf("KEY\tDATA\tOF\n");
	int bytesRead = 0;
	while (bytesRead = readBlock()) {
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			//printf("%d -- %d\n", buffer[i].key, buffer[i].page);
			printf("%d\t{%d,%d,%d}\t%d\n", buffer[i].key, buffer[i].data.a, buffer[i].data.b, buffer[i].data.h, buffer[i].ofptr);
		}
	}
}

void ISFile::printOF() {

}

ISFile::~ISFile() {
	if (idx)
		delete idx;
	if (buffer)
		delete[] buffer;
}