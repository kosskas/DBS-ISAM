#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Index.h"
#include "BFile.h"
#include <math.h>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
#include <map>
using namespace std;

class ISFile{
private:
	BFile* file;
	BFile* overflow;
	Index* idx;

	string filename;
	string ofname;
	string idxname;

	int BUFFSIZE;
	int IDXBUFFSIZE;

	Record NIL;
	int lastFreeOFPage;
	map<char, pair<int, int>> operacja;

	int maxOFsize;
	bool fileswitcher;
	Index* createIndex(string idxName, int nOfpages);

	vector<Record> getChain(Record first);
	void insertToOf(int key, Data data, int *startptr);

	int searchIfDeleted(int key, int* found, Record* rec);
	void setVars();
public:
	int bf, bi;
	int realN;
	int realV;
	int mainPages;
	int ofPages;
	int idxPages;
	double VNratio;
	double alpha;
	int idxRecs;
	int NrecordInMain;
	int VrecordInOf;
	ISFile(uint32_t BUFFSIZE, double alfa);
	int searchRecord(int key, int* found, Record* rec);
	void searchInOF2(int ptr, int key, int* found, Record* rec);
	void insertRecord(int key, Data data, int*sorg=NULL);
	Record removeRecord(int key);
	void updateRecord(int key, Data data);
	void updateRecord(int oldkey, int newkey);
	void clearFile();
	void reorganiseFile();
	void info(double alpha);
	void printRecords();
	void printIndex();
	void printStruct();
	void printOF();
	void getFileSize(int* filesize, int* ovsize, int* idxsize);
	~ISFile();
};
int GetFileSize(string filename);