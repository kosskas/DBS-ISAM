#include "ISFile.h"

ISFile::ISFile(uint32_t BUFFSIZE, string filename, ios_base::openmode flags) : Buffered(filename, flags) {
	buffer = NULL;
	this->BUFFSIZE = BUFFSIZE;
	buffer = new Record[BUFFSIZE];
	memset(buffer, 0, sizeof(Record) * BUFFSIZE);
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

Record ISFile::searchRecord(int key)
{
	return Record();
}

Record ISFile::insertRecord(int key, Data data)
{
	return Record();
}

Record ISFile::removeRecord(int key)
{
	return Record();
}
