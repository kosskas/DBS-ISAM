#pragma once
#include "Buffered.h"

#define NOTFOUND 0xFFFF

/*
Zazwyczaj, bi >> 1 i SIN <<SN, zatem przeszukiwanie indeksu na dysku jest bardzo szybkie
(lub indeks mieœci siê w pamiêci operacyjnej i w ogóle nie s¹ potrzebne dostêpy do dysku).

UWAGA: Indeksy w organizacji indeksowo-sekwencyjnej nazywamy indeksami rzadkimi (sparse):
nie wszystkie rekordy s¹ indeksowane, a jedynie tylko pierwsze na stronach obszaru g³ównego.
*/
class Index : public Buffered {
private:
	struct IdxRec {
		int key;
		int page;
	}*buffer;
	uint32_t BUFFSIZE;

public:
	Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags);
	int readBlock(); //writeToBuff();
	
	void writeBlock(const char* block); //writeToFile();
	int findPage(int key);
	void writeIdxRecord(IdxRec rec);
	void printIndex();
	~Index();
};

