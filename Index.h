#pragma once
#include <iostream>
#include <fstream>

using namespace std;

extern int idxReads;
extern int idxWrites;

class Index{
public:
	struct Record {
		int key;
		int page;
	};
private:
	fstream* file = NULL;
	string filename = "";
	ios_base::openmode flags = 0;
	Record *buffer;
	uint32_t BUFFSIZE;
	fstream* createFile(string fileName, int nOfpages);
	int readBlock(fstream* currfile, int blockNum); //writeToBuff();
	int writeBlock(fstream* currfile, int blockNum); //writeToFile();
public:
	Index(int32_t BUFFSIZE,int nOfpages, string filename, ios_base::openmode flags);

	int readIdxRecord(int key);
	void writeIdxRecord(int key, int page);
	void printIndex();
	void resetPtr();
	~Index();
};