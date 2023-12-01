#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include "Record.h"

using namespace std;


class BFile {
private:
	fstream* file;
	string filename;
	ios_base::openmode flags;
	int BUFFSIZE;
public:
	Record* buffer;

	BFile(string filename, int BUFFSIZE, int pages);
	int readBlock(int blockNum);
	int writeBlock(int blockNum);
	void resetPtr();
	void clearFile();

	~BFile();
};

