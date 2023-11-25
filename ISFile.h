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
	//ofptr

	Index* idx;

	uint32_t BUFFSIZE;

	Record *buffer;
public:
	ISFile(uint32_t BUFFSIZE, string filename, ios_base::openmode flags);
	int readBlock();
	//writeToBuff();
	void writeBlock(); //writeToFile();
	//readPage(page)

	Record searchRecord(int key);
	Record insertRecord(int key, Data data);
	Record removeRecord(int key);
	//reorg
	//getOF


	void printRecords();

	//void printBuffer();
	~ISFile();
};

