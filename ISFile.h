#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Index.h"
#include "Record.h"
#include <math.h>
#include <algorithm>
using namespace std;

#define NOTFOUND 0xFFFFFFFF

class ISFile{
private:
	fstream* file = NULL;
	string filename = "";
	ios_base::openmode flags = 0;
	//ofptr
	int ofBlocNo;

	//paging od 0!!!
	uint32_t BUFFSIZE;

	Record *Mbuffer;
	Record *ofbuffer;
	///rekord�w w ob. g��wnym
	int N;
	//rekord�w w nadmiarze
	int V;

	int bf, bi;
	Index* idx;

	void initBuffers();
public:

	ISFile(uint32_t BUFFSIZE);
	
	int readBlock(int blockNum, Record* buffer);
	int writeBlock(int blockNum, Record* buffer);
	//returns page
	//zwraca numer strony

	int searchRecord(int key, int*found);
	int searchInOF(int key, int* found);

	void insertRecord(int key, Data data);
	void removeRecord(int key);

	//zwraca offset w ov
	void insertToOf(int key, Data data, short int *ptr);
	void createIndex();
	void reorganiseFile(double alpha);
	void createOF(int blockNo);

	///GetNextRecord????
	

	void printRecords();
	void printStruct();
	void printBuffer();
	void printOF();
	
	void resetPtr();
	void clearFile();
	//void printBuffer();
	~ISFile();
};

