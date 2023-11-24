#pragma once
#include "Buffered.h"

/*
Zazwyczaj, bi >> 1 i SIN <<SN, zatem przeszukiwanie indeksu na dysku jest bardzo szybkie
(lub indeks mie�ci si� w pami�ci operacyjnej i w og�le nie s� potrzebne dost�py do dysku).

UWAGA: Indeksy w organizacji indeksowo-sekwencyjnej nazywamy indeksami rzadkimi (sparse):
nie wszystkie rekordy s� indeksowane, a jedynie tylko pierwsze na stronach obszaru g��wnego.
*/
class Index : public Buffered {

public:
	Index();
	void ReadBlock(); //writeToBuff();
	void WriteBlock(const char* block); //writeToFile();
	~Index();
};

