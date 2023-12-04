#pragma once
#include <iostream>
#include <fstream>

using namespace std;
/*
Zazwyczaj, bi >> 1 i SIN <<SN, zatem przeszukiwanie indeksu na dysku jest bardzo szybkie
(lub indeks mieœci siê w pamiêci operacyjnej i w ogóle nie s¹ potrzebne dostêpy do dysku).

UWAGA: Indeksy w organizacji indeksowo-sekwencyjnej nazywamy indeksami rzadkimi (sparse):
nie wszystkie rekordy s¹ indeksowane, a jedynie tylko pierwsze na stronach obszaru g³ównego.
*/
class Index{
private:
	fstream* file = NULL;
	string filename = "";
	ios_base::openmode flags = 0;
	struct IdxRec {
		int key;
		int page;
	}*buffer;
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