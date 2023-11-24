#pragma once
#include "Buffered.h"

/*
Zazwyczaj, bi >> 1 i SIN <<SN, zatem przeszukiwanie indeksu na dysku jest bardzo szybkie
(lub indeks mieœci siê w pamiêci operacyjnej i w ogóle nie s¹ potrzebne dostêpy do dysku).

UWAGA: Indeksy w organizacji indeksowo-sekwencyjnej nazywamy indeksami rzadkimi (sparse):
nie wszystkie rekordy s¹ indeksowane, a jedynie tylko pierwsze na stronach obszaru g³ównego.
*/
class Index : public Buffered {

public:
	Index();
	void ReadBlock(); //writeToBuff();
	void WriteBlock(const char* block); //writeToFile();
	~Index();
};

