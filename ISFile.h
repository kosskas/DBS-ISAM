#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Buffered.h"
#include "Index.h"
#include "Record.h"
using namespace std;

#define NOTFOUND 0xFFFFFFFF


class ISFile : public Buffered {
private:
	//ofptr



	uint32_t BUFFSIZE;

	Record *buffer;
public:
	Index* idx;
	ISFile(uint32_t BUFFSIZE, string filename, ios_base::openmode flags);
	int readBlock();
	//writeToBuff();
	void writeBlock(); //writeToFile();
	//readPage(page)

	//returns page
	int searchRecord(int key);

	Record insertRecord(int key, Data data);
	Record removeRecord(int key);
	//reorg
	//getOF
	void reorganiseFile();
	void clearFile();

	void printRecords();
	void printStruct();
	void printOF();


	//void printBuffer();
	~ISFile();
};

