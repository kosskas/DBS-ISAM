#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Index.h"
#include "BFile.h"
#include <math.h>
#include <algorithm>
#include <vector>
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

Po każdej przeprowadzonej operacji podawaj liczbę faktycznie zrealizowanych operacji odczytu lub zapisu stron dyskowych.
Program powinien dawać możliwość wyświetlania zawartości pliku z danymi i indeksu po każdej operacji zmieniającej zawartość pliku
	- wstawieniu
	- aktualizacji
	- usunięciu rekordu
[wyświetlanie zawartości pliku oznacza przedstawienie w sposób czytelny dla człowieka wewnętrznej struktury tych plików.
dla struktury indeksowo-sekwencyjnej wyświetlenie zawartości pliku z danymi powinno dać możliwość sprawdzenia,
czy rekord znajduje się w części głównej, czy nadmiarowej,
dokładnie w którym miejscu, ile jest pustych miejsc na poszczególnych stronach pliku,
czy też zawartości poszczególnych łańcuchów przepełnień.]

Dodatkowo program powinien dawać możliwość wczytywania danych testowych z pliku testowego.
Danymi testowymi powinna być dowolna sekwencja operacji.

Program nie czytający poleceń z pliku testowego powinien działać w sposób interaktywny,
tzn. po pobraniu komendy wykonać ją, przedstawić wynik jej wykonania, po czym czekać na następną komendę.

	Na pierwszej stonie jest specjalny niemożliwy klucz wartość -1
	żeby nie było możliwe wstawianie przed niego

	usuwanie to zaznaczenie flagi że jest usunięty.
	prawdziwe usuwanie przy reorganizacji
*/
class ISFile{
private:
	BFile* file;
	BFile* overflow;
	Index* idx;

	string filename;
	string ofname;
	string idxname;

	int BUFFSIZE;
	int IDXBUFFSIZE;


	///rekordów w ob. głównym
	int NrecordInMain;
	//rekordów w nadmiarze
	int VrecordInOf;
	int bf, bi;

	int maxOFsize;

	bool fileswitcher;
	Index* createIndex(string idxName, int nOfpages);

	///STAN - wersja prostsza
	vector<Record> getChain(Record first);
	void insertToOf(int key, Data data, short *startptr);
public:
	ISFile(uint32_t BUFFSIZE);

	int searchRecord(int key, int* found, Record* rec, bool del = false);
	int searchInOF(int key, int* found, Record* rec, bool del = false);

	void insertRecord(int key, Data data);
	void removeRecord(int key);
	//TODO
	void updateRecord(int key, Data data);
	void updateRecord(int oldkey, int newkey);


	void clearFile();
	void reorganiseFile(double alpha);
	void info(double alpha);
	void printRecords();
	void printIndex();
	void printStruct();
	void printOF();
	~ISFile();
};

/*
	///Odczytaj stronę od
	while (bytesRead = overflow->readBlock(page)) {
		//znajdz czy taki jest
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (ptr == offset) {
				//już lista
				if (key < overflow->buffer[i].key) {
					//podmień
					Data temp = data;
					int tempk = key;

					data = overflow->buffer[i].data;
					key = overflow->buffer[i].key;
					del = overflow->buffer[i].deleted;

					overflow->buffer[i].data = temp;
					overflow->buffer[i].key = tempk;
					overflow->buffer[i].deleted = 0;
					printf("SWAP\n");
					overflow->writeBlock(page);
				}
				prevpage = page;
				prev = offset;
				ptr = overflow->buffer[i].ofptr;
				count++;

			}
			if (overflow->buffer[i].key == 0) {
				printf("znaleziono miejsce w oF\n");
				//wstaw w wolne miejce
				overflow->buffer[i].key = key;
				overflow->buffer[i].data = data;
				overflow->buffer[i].deleted = del;

				//Zaktualizuj wskaźniki
				if(count ==0)
					*startptr = offset; //ten jest za kluczem z maina
				//zapisz rekord
				overflow->writeBlock(page);
				if (prev) {
					//prev ma wskazywać na offset
					bytesRead = overflow->readBlock(prevpage);
					overflow->buffer[(prev-1)%BUFFSIZE].ofptr = offset;
					overflow->writeBlock(prevpage);
				}

				VrecordInOf++;
				return;
			}
		}
		page++;
	}
*/