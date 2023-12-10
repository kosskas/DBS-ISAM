#include <iostream>
#include <string>
#include <random>
#include "ISFile.h"
#include <map>
using namespace std;

void generate() {
    random_device rd;
    mt19937 generator(rd());
    //mt19937 generator(2002);
    uniform_int_distribution<int> keys(1, 0x01F4);//0x01F4
   // uniform_int_distribution<int> keys(1, 0x000FFFFF);//0x01F4
    uniform_int_distribution<int> recs(1, 0x0064);
    uniform_int_distribution<int> los(0, 100);

   // char rozklad[] = { 50, 20, 10, 10, 10, 0 };//'+,'-','?','u','U','o'
    //char rozklad[] = { 50, 20, 10,9, 10, 1 };//'+,'-','?','u','U','o'
    char rozklad[] = { 100,0,0,0,0,0 };//'+,'-','?','u','U','o'

    //autoreorg ///50% 20% 10% 10% 10%
    //manreorg ///50% 20% 10% 5% 10% %5


    ///rozkl + 50%
    int rozk[6] = { 0 };
    for (int i = 0; i < 1000000; i++) {
        int test = los(generator);
        if (test <= rozklad[0]) {//+
            printf("+%d %d %d %d\n", keys(generator), recs(generator), recs(generator), recs(generator));
            rozk[0]++;
        }
        else if (test <= rozklad[0]+ rozklad[1]) {//-
            printf("-%d\n", keys(generator));
            rozk[1]++;
        }
        else if (test <= rozklad[0] + rozklad[1]+ rozklad[2]) {//?
            printf("?%d\n", keys(generator));
            rozk[2]++;
        }
        else if (test <= rozklad[0] + rozklad[1]+ rozklad[2] + rozklad[3]) {//u //85
            printf("u%d %d %d %d\n", keys(generator), recs(generator), recs(generator), recs(generator));
            rozk[3]++;
        }
        else if (test <= rozklad[0] + rozklad[1] + rozklad[2] + rozklad[3] + rozklad[4]) {//U //95
            printf("U%d %d\n", keys(generator), keys(generator));
            rozk[4]++;
        }
        
        else if (test <= rozklad[0] + rozklad[1] + rozklad[2] + rozklad[3] + rozklad[4] + rozklad[5]) {//o
            printf("o%\n");
            rozk[5]++;
        }
        
    }
    for (int i = 0; i < 6; i++) {
        printf("%d ", rozk[i]);
    }
    exit(1);
}
int main(int argc, char** argv) {
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> keys(1, 0x0100);//0x01F4
    uniform_int_distribution<int> recs(1, 0x0064);
    
    ////stats
    map<char, pair<int, int>> operacja;
    map<int,int> glowny, indeks, nadmiar;

    ////
    //generate();
    

    int wspb = 4;//atoi(argv[1]);
    double alfa = 0.5;// atof(argv[2]);
    ISFile isfile(wspb, alfa);

    char cmd;
    int key = 0,a=0,b=0,h=0;
    int i = 0;
    int sum = 0;
    bool stats = true;

    while (cin >> cmd) {
        nOfReads = nOfWrites = 0;
        if (cmd == 'r') {
            int key = keys(generator);
            printf("Wylosowano klucz %d\n", key);
            isfile.insertRecord(key, { recs(generator), recs(generator), recs(generator) });
        }
        if (cmd == '+') {
            cin >> key >> a >> b >> h;  
            isfile.insertRecord(key, { a, b, h });
            
        }
        if (cmd == '-') {
            cin >> key;
            isfile.removeRecord(key);
        }
        if (cmd == '?') {
            int exits = 0;
            Record found;
            cin >> key;
            int ret = isfile.searchRecord(key,&exits,&found);
            
            if (exits && !found.deleted)
                printf("a=%d b=%d h=%d\n",found.data.a, found.data.b, found.data.h);
            else
                printf("0\n");
            
        }
        if (cmd == 'u') {
            cin >> key >> a >> b >> h;
            isfile.updateRecord(key,{a,b,h});
        }
        if (cmd == 'U') {
            int oldkey = 0;
            cin >> oldkey>>key;
            isfile.updateRecord(oldkey,key);
        }
        if (cmd == 'p')
            isfile.printRecords();
        if (cmd == 'P')
            isfile.printStruct();
        if (cmd == 'i')
            isfile.printIndex();
        if (cmd == 'o') {
            //printf("\nBylo rekorow %d\n",isfile.NrecordInMain+isfile.VrecordInOf);
            isfile.reorganiseFile();
            //printf("\Jest rekorow %d\n", isfile.NrecordInMain + isfile.VrecordInOf);
        }
        if (cmd == 'n')
            isfile.info(0.5);
        if (cmd == 'c')
            isfile.clearFile();
        if (cmd == 'q')
            break;
        i++;
        operacja[cmd].first += nOfWrites + nOfReads;
        operacja[cmd].second++;
        if (((isfile.NrecordInMain + isfile.VrecordInOf) % 10 == 0 && (isfile.NrecordInMain + isfile.VrecordInOf) != sum)) {
            sum = (isfile.NrecordInMain + isfile.VrecordInOf);
           // printf("%d ", isfile.NrecordInMain + isfile.VrecordInOf);
            int file, ov, idx;
            isfile.getFileSize(&file, &ov, &idx);
            glowny[sum] = file;
            nadmiar[sum] = ov;
            indeks[sum] = idx;
        }
        printf("\n%d\tBylo zapisow %d\t odczytow %d\n",i, nOfWrites, nOfReads);

    }


    ofstream output;
    
    output.open("glowny.txt");
    for (auto i = glowny.begin(); i != glowny.end(); ++i) {
        output << i->first << " " << i->second << '\n';
    }
    output.close();

    output.open("nadmiar.txt");
    for (auto i = nadmiar.begin(); i != nadmiar.end(); ++i) {
        output << i->first << " " << i->second << '\n';
    }
    output.close();

    output.open("indeks.txt");
    for (auto i = indeks.begin(); i != indeks.end(); ++i) {
        output << i->first << " " << i->second << '\n';
    }
    output.close();
    
    
    string nazwa = "operacje" + string(to_string(wspb)) + "-"+ string(to_string(alfa)) + ".txt";
    output.open(nazwa);
    char kom[] = { '+','-','?','u','U','o' };
    for (int i = 0; i < 6; i++) {
        output << kom[i] << " operacji(r+w): " << operacja[kom[i]].first << " ilosc: " << operacja[kom[i]].second << '\n';
    }
    output.close();




	return 0;
}

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