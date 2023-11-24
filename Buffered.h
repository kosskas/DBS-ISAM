#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "Record.h"
using namespace std;

class Buffered {
protected:
	fstream* file;
	string filename;
	ios_base::openmode flags;

public:
	Buffered(string filename, ios_base::openmode flags) : filename(filename), flags(flags) {
		file = new fstream();
		file->open(filename, flags);
	}
	virtual void readBlock() = 0;
	virtual void writeBlock(const char* block) = 0;
	~Buffered() {
		file->close();
		delete file;
	}
};