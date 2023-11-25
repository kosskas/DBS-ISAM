#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "Record.h"
using namespace std;

class Buffered {
protected:
	fstream* file=NULL;
	string filename="";
	ios_base::openmode flags=0;
	uint32_t w_ptr=0;
	uint32_t r_ptr=0;

public:
	Buffered() {}
	Buffered(string filename, ios_base::openmode flags) : filename(filename), flags(flags) {
		file = new fstream();
		file->open(filename, flags);
		w_ptr= 0;
		r_ptr = 0;
	}
	virtual int readBlock() = 0;
	virtual void writeBlock() = 0;
	void resetPtr() {
		r_ptr = 0;
		w_ptr = 0;
		file->clear();
		file->seekg(r_ptr, ios::beg);
		file->seekp(w_ptr, ios::beg);
	}
	void clearFile() {
		file->close();
		file->open(filename, flags | ios::trunc);
		resetPtr();
	}
	~Buffered() {
		if (file != NULL) {
			file->close();
			delete file;
		}
	}
};