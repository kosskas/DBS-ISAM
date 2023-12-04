#include "BFile.h"
#define clearBuffer(x) memset(x, 0, sizeof(Record) * BUFFSIZE)

int nOfReads = 0;
int nOfWrites = 0;

BFile::BFile(string filename, int BUFFSIZE, int pages) {
	this->BUFFSIZE = BUFFSIZE;
	this->flags = ios::binary | ios::in | ios::out | ios::trunc;
	buffer = NULL;
	buffer = new Record[BUFFSIZE];
	clearBuffer(buffer);

	file = new fstream();
	file->open(filename, flags);
	int page = 0;
	while (pages--) {
		writeBlock(page++);
	}

}

int BFile::readBlock(int blockNum) {
	resetPtr();
	file->seekg(blockNum * (sizeof(Record) * BUFFSIZE));
	int bytesRead = file->read((char*)buffer, sizeof(Record) * BUFFSIZE).gcount();
	if (bytesRead < sizeof(Record) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(Record) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	nOfReads++;
	return bytesRead;
}

int BFile::writeBlock(int blockNum) {
	resetPtr();
	const char* serialRec = (const char*)buffer;
	file->seekp(blockNum * (sizeof(Record) * BUFFSIZE));
	size_t poc = file->tellp();
	file->write(serialRec, sizeof(Record) * BUFFSIZE);
	size_t written = file->tellp();
	written = written - poc;
	printf("Zapisano %dB\n", written);
	nOfWrites++;
	return written;
}

void BFile::resetPtr() {
	file->clear();
	file->seekg(0, ios::beg);
	file->seekp(0, ios::beg);
}

BFile::~BFile() {
	if (file) {
		file->close();
		delete file;
	}
	if (buffer)
		delete[] buffer;
}
