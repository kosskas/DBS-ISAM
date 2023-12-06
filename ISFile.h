#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Index.h"
#include "BFile.h"
#include <math.h>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
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

	Record NIL;

	int bf, bi;
	int realN;
	int realV;
	int mainPages;
	int ofPages;
	int idxPages;
	double VNratio;
	double alpha;

	int maxOFsize;
	bool fileswitcher;
	Index* createIndex(string idxName, int nOfpages);

	vector<Record> getChain(Record first);
	void insertToOf(int key, Data data, short *startptr);

	int searchIfDeleted(int key, int* found, Record* rec);
	void setVars();
public:
	int NrecordInMain;
	int VrecordInOf;
	ISFile(uint32_t BUFFSIZE, double alfa);
	int searchRecord(int key, int* found, Record* rec);
	void searchInOF2(short ptr, int key, int* found, Record* rec);
	void insertRecord(int key, Data data);
	Record removeRecord(int key);
	void updateRecord(int key, Data data);
	void updateRecord(int oldkey, int newkey);
	void clearFile();
	void reorganiseFile(double alpha);
	void info(double alpha);
	void printRecords();
	void printIndex();
	void printStruct();
	void printOF();
	void printFileSize();
	~ISFile();
};
int GetFileSize(string filename);