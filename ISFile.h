#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Buffered.h"
#include "Index.h"
#include "Record.h"
#include <math.h>
using namespace std;

#define NOTFOUND 0xFFFFFFFF
#define BEFOREZERO 0xFFFFFFFE


class ISFile : public Buffered {
private:
	//ofptr
	uint32_t ofPtr;

	//paging od 0!!!
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

	void insertRecord(int key, Data data);
	void removeRecord(int key);

	void insertToOf(int key, Data data);
	//reorg
	//getOF
	void createIndex();
	void reorganiseFile();
	void clearFile();

	void printRecords();
	void printStruct();
	void printOF();


	//void printBuffer();
	~ISFile();
};

