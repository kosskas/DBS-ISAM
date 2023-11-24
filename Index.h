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
		uint16_t key;
		uint16_t page;
	}*buffer;
	uint32_t BUFFSIZE;

public:
	Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags);
	void readBlock(); //writeToBuff();
	void writeBlock(const char* block); //writeToFile();
	uint16_t findPage(uint16_t key);
	void writeIdxRecord(IdxRec rec);

	void printIndex();
	void clearFile();
	~Index();
};

