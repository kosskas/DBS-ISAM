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

class ISFile{
private:
	fstream* file = NULL;
	string filename = "";
	ios_base::openmode flags = 0;
	//ofptr
	int ofBlocNo;

	//paging od 0!!!
	uint32_t BUFFSIZE;

	Record *buffer;
	///rekordów w ob. g³ównym
	int N;
	//rekordów w nadmiarze
	int V;

	int bf, bi;


public:
	Index* idx;
	ISFile(uint32_t BUFFSIZE);
	int readBlock(int blockNum);
	//writeToBuff();
	int writeBlock(int blockNum); //writeToFile();

	//returns page

	//zwraca numer strony
	int searchRecord(int key, int*found);
	int searchInOF(int key, int* found);

	void insertRecord(int key, Data data);
	void removeRecord(int key);

	//zwraca offset w ov
	int insertToOf(int key, Data data);
	//reorg
	//getOF
	void createIndex();
	void reorganiseFile(double alpha);
	void createOF(int blockNo);

	

	void printRecords();
	void printStruct();
	void printBuffer();
	void printOF();
	
	void resetPtr();
	void clearFile();
	//void printBuffer();
	~ISFile();
};

