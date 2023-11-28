#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Index.h"
#include "Record.h"
#include <math.h>
#include <algorithm>
#include <vector>
using namespace std;

#define NOTFOUND 0xFFFFFFFF

class ISFile{
private:
	fstream* file;
	string filename;
	string idxname;
	ios_base::openmode flags;
	//ofptr
	int ofBlocNo;
	//paging od 0!!!
	uint32_t BUFFSIZE;

	Record *Mbuffer;
	Record *ofbuffer;
	///rekordów w ob. g³ównym
	int NrecordInMain;
	//rekordów w nadmiarze
	int VrecordInOf;

	int bf, bi;
	
	bool swc;

	fstream* createFile(string fileName, int nOfpages);
	void createOF(fstream* currfile, int blockNo, int nOfpages);
	Index* createIndex(string idxName, int nOfpages);

	void initBuffers();



	int readBlock(fstream* currfile, int blockNum, Record* buffer);
	int writeBlock(fstream* currfile, int blockNum, Record* buffer);
	void resetPtr(fstream* currfile);
	//DO POPRAWY
	vector<Record> getChain(Record first);

	///DO POPRAWY
	void insertToOf(int key, Data data, short int* ptr);
public:
	Index* idx;
	ISFile(uint32_t BUFFSIZE);
	

	//returns page
	//zwraca numer strony

	int searchRecord(int key, int*found, bool del = false);
	int searchInOF(int key, int* found, bool del = false);

	void insertRecord(int key, Data data);
	void removeRecord(int key);
	//TODO
	void updateRecord(int key, Data data);

	//zwraca offset w ov

	void reorganiseFile(double alpha);


	///GetNextRecord????
	

	void printRecords();
	void printStruct();
	void printOF();
	

	void clearFile();
	~ISFile();
};

