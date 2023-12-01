#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include "Record.h"

using namespace std;
template <typename T>
class BFile {
private:
	fstream* file;
	string filename;
	ios_base::openmode flags;
	int BUFFSIZE;
public:
	T* buffer;

	BFile(string filename, int BUFFSIZE, int pages);
	int readBlock(int blockNum);
	int writeBlock(int blockNum);
	void resetPtr();
	void clearFile();

	~BFile();
};

