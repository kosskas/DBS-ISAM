#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Buffered.h"
#include "Index.h"
#include "Record.h"
using namespace std;



class ISFile : public Buffered {
private:


	Index* idx;

	uint32_t BUFFSIZE;

	Record *buffer;
public:
	ISFile(uint32_t BUFFSIZE, string filename, ios_base::openmode flags);
	int readBlock(); //writeToBuff();
	void writeBlock(const char* block); //writeToFile();

	Record searchRecord(uint16_t key);
	Record insertRecord(uint16_t key, Data data);
	Record removeRecord(uint16_t key);


	//void resetCursor();
	//void resetBufferPtr();
	//void clearBuffer();

	void clearFile();
	void printRecords();

	//void printBuffer();
	~ISFile();
};

