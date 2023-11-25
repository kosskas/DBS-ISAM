#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Buffered.h"
#include "Index.h"
#include "Record.h"
#include <math.h>
#include <algorithm>
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
	bool firstWrite;
	int wToBufferF;

public:
	Index* idx;
	ISFile(uint32_t BUFFSIZE);
	int readBlock();
	//writeToBuff();
	void writeBlock(); //writeToFile();

	//returns page

	//zwraca numer strony
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
	void printBuffer();
	void printOF();


	//void printBuffer();
	~ISFile();
};

