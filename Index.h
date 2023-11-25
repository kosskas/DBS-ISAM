#pragma once
#include "Buffered.h"

#define NOTFOUND 0xFFFF

/*
Zazwyczaj, bi >> 1 i SIN <<SN, zatem przeszukiwanie indeksu na dysku jest bardzo szybkie
(lub indeks mie�ci si� w pami�ci operacyjnej i w og�le nie s� potrzebne dost�py do dysku).

UWAGA: Indeksy w organizacji indeksowo-sekwencyjnej nazywamy indeksami rzadkimi (sparse):
nie wszystkie rekordy s� indeksowane, a jedynie tylko pierwsze na stronach obszaru g��wnego.
*/
class Index : public Buffered {
private:
	struct IdxRec {
		int key;
		int page;
	}*buffer;
	uint32_t BUFFSIZE;
	//int nOfBuff;
public:
	Index(int32_t BUFFSIZE, string filename, ios_base::openmode flags);
	int readBlock(); //writeToBuff();
	void writeBlock(); //writeToFile();
	int readIdxRecord(int key);
	void writeIdxRecord(int key, int page);
	void swapKey(int odlKey, int key);

	void printIndex();
	~Index();
};

