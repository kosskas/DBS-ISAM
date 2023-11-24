#pragma once
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Buffered {
protected:
	fstream* file;
	string filename;
	ios_base::openmode flags;
	uint32_t r_ptr;
	uint32_t w_ptr;
	uint32_t r_idx;
	uint32_t w_idx;
public:
	Block()
	virtual void ReadBlock() = 0;
	virtual void WriteBlock(const char* block) = 0;
};