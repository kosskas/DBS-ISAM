#include <iostream>
#include <string>
#include <math.h>
#include "Index.h"
#include "ISFile.h"
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
void checkIdx();
int main(int argc, char** argv) {

	///rozróżnienie czy od nowa wszystko
	//checkIdx();
    ISFile file(10, "isfile", ios::binary | ios::in | ios::out);
    
    char cmd;
    int key = 0,a=0,b=0,h=0;
    while (cin >> cmd) {
        if (cmd == '+') {
            cin >> key>>a>>b>>h;
            file.insertRecord(key, { a, b, h });
        }
        if (cmd == '-') {
            cin >> key;

        }
        if (cmd == '?') {
            cin >> key;
            if (Tree.Search(key))
                wcout << "1\n";
            else
                wcout << "0\n";
        }
        if (cmd == 'p')
            Tree.PrintTree();
        if (cmd == 'b')
            wcout << Tree.GetRootBlackHeight() << '\n';
        if (cmd == 'P')
            Tree.PrintInOrder();
        if (cmd == 'c')
            Tree.Clear();
        if (cmd == 'r')
            wcout << Tree.GetRootKey() << '\n';
        if (cmd == 'm')
            wcout << Tree.GetMin() << '\n';
        if (cmd == 'M')
            wcout << Tree.GetMax() << '\n';
        if (cmd == 's')
            wcout << Tree.GetSize() << '\n';
        if (cmd == 'q')
            break;
    }

    
	return 0;
}

void checkIdx() {
	Index idx(5, "idx1", ios::binary | ios::in | ios::out | ios::trunc);
	idx.clearFile();

	idx.writeIdxRecord(1, 1);
	idx.writeIdxRecord(7, 1);
	idx.writeIdxRecord(9, 1);
	idx.writeIdxRecord(14, 1);
	idx.writeIdxRecord(19, 1);
	idx.writeIdxRecord(22, 1);

	idx.printIndex();
	idx.swapKey(5, 6);
	idx.printIndex();


	for (int i = 1; i < 24; i++) {
		idx.readIdxRecord(i);
	}
}