#include "BFile.h"


template <typename T>
BFile<T>::BFile(string filename, int BUFFSIZE, int pages) {
	this->BUFFSIZE = BUFFSIZE;
	this->flags = ios::binary | ios::in | ios::out | ios::trunc;
	buffer = NULL;
	buffer = new T[BUFFSIZE];
	clearBuffer(buffer);

	file = new fstream();
	file->open(filename, flags);
	int page = 0;
	while (pages--) {
		writeBlock(page++);
	}

}

template <typename T>
int BFile<T>::readBlock(int blockNum) {
	resetPtr();
	file->seekg(blockNum * (sizeof(Record) * BUFFSIZE));
	int bytesRead = file->read((char*)buffer, sizeof(Record) * BUFFSIZE).gcount();
	if (bytesRead < sizeof(Record) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(Record) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	return bytesRead;
}

template <typename T>
int BFile<T>::writeBlock(int blockNum) {
	resetPtr();
	const char* serialRec = (const char*)buffer;
	file->seekp(blockNum * (sizeof(Record) * BUFFSIZE));
	size_t poc = file->tellp();
	file->write(serialRec, sizeof(Record) * BUFFSIZE);
	size_t written = file->tellp();
	written = written - poc;
	printf("Zapisano %dB\n", written);
	return written;
}

template <typename T>
void BFile<T>::resetPtr() {
	file->clear();
	file->seekg(0, ios::beg);
	file->seekp(0, ios::beg);
}

template <typename T>
void BFile<T>::clearFile() {
	file->close();
	file->open(filename, flags | ios::trunc);
	resetPtr();
}

template <typename T>
BFile<T>::~BFile() {
	if (file != NULL) {
		file->close();
		delete file;
	}
	if (buffer)
		delete[] buffer;
}
