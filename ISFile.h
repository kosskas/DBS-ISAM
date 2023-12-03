#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Index.h"
#include "BFile.h"
#include <math.h>
#include <algorithm>
#include <vector>
using namespace std;

class ISFile{
private:
	BFile* file;
	BFile* overflow;
	Index* idx;

	//fstream* file;
	string filename;
	string ofname;
	string idxname;

	//paging od 0!!!
	int BUFFSIZE;
	int IDXBUFFSIZE;


	///rekordów w ob. g³ównym
	int NrecordInMain;
	//rekordów w nadmiarze
	int VrecordInOf;

	int bf, bi;
	
	bool swc;

	void createOF(fstream* currfile, int blockNo, int nOfpages);
	Index* createIndex(string idxName, int nOfpages);



	//DO POPRAWY
	vector<Record> getChain(Record first);

	///DO POPRAWY
	void insertToOf(int key, Data data, short int* ptr);
public:

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
	void clearFile();
	void reorganiseFile(double alpha);

	void info(double alpha);
	///GetNextRecord????
	

	void printRecords();
	void printIndex();
	void printStruct();
	void printOF();
	


	~ISFile();
};

