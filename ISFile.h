#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "IBlock.h"
#include "Index.h"
#include "Record.h"
using namespace std;



class ISFile : public Block {
private:
	fstream* file;
	string filename;
	ios_base::openmode flags;
	uint32_t r_ptr;
	uint32_t w_ptr;
	uint32_t r_idx;
	uint32_t w_idx;


	Record buffer[BUFFSIZE] = { Record() };
public:
	void writeToBuff();
	void writeToFile();

	void forceWriteRecord(Record rec);

	ISFile(string filename, ios_base::openmode flags);
	Record readNext();

	void writeRecord(Record rec);

	void resetCursor();
	void resetBufferPtr();
	void clearBuffer();
	void clearFile();
	void printRecords();
	void printBuffer();
	~ISFile();
};

