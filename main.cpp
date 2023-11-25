#include <iostream>
#include <string>
#include <math.h>
#include "Index.h"

using namespace std;


/*
Oznaczenia:
N – liczba rekordów w obszarze głównym pliku
V - liczba rekordów w obszarze nadmiarowym pliku
B – pojemność strony dyskowej (bajty)
R – wielkość rekordu (bajty)
K – wielkość klucza (bajty)
P – wielkość wskaźnika (bajty)
α – współczynnik wykorzystania strony w obszarze głównym po reorganizacji, <1

Parametry:
bf – współczynnik blokowania w obszarze głównym, równy floor( B / (R+P) )
bi – współczynnik blokowania dla indeksu, równy floor( B / (K+P) )
bi jest (R+P)/(K+P) razy większy niż bf (bo zwykle cały rekord jest dużo większy niż klucz)
SN – wielkość obszaru głównego, w stronach, równa N/(bf × α)
SOV – wielkość obszaru nadmiarowego, w stronach, równa V/bf (strony obszaru nadmiarowego są zapełniane do końca)
SIN – wielkość indeksu 1. poziomu, w stronach, równa ceil( SN/bi )

Zazwyczaj, bi >> 1 i SIN <<SN, zatem przeszukiwanie indeksu na dysku jest bardzo szybkie
(lub indeks mieści się w pamięci operacyjnej i w ogóle nie są potrzebne dostępy do dysku).

UWAGA: Indeksy w organizacji indeksowo-sekwencyjnej nazywamy indeksami rzadkimi (sparse):
nie wszystkie rekordy są indeksowane, a jedynie tylko pierwsze na stronach obszaru głównego.

*/

int main(int argc, char** argv) {
	Index idx(5, "idx1", ios::binary | ios::in | ios::out);
	idx.printIndex();
	idx.resetPtr();

	idx.writeIdxRecord(1235, 1);
	idx.writeIdxRecord(1, 1);
	idx.writeIdxRecord(2, 1);
	idx.writeIdxRecord(3, 1);
	idx.writeIdxRecord(5, 1);

	idx.resetPtr();
	idx.printIndex();
	idx.resetPtr();
	idx.swapKey(5, 2137);
	idx.resetPtr();
	idx.printIndex();

	idx.clearFile();
	idx.writeIdxRecord(1234, 1);
	idx.resetPtr();
	idx.printIndex();

	/*
	for (int i = 5; i < 200; i += 5) {
		idx.resetPtr();
		idx.readIdxRecord(i);
	}
	*/

	return 0;
}