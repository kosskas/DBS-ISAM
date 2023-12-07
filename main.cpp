#include <iostream>
#include <string>
#include <random>
#include "ISFile.h"
#include <map>
using namespace std;

void generate() {
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> keys(1, 0x1004);//0x01F4
    uniform_int_distribution<int> recs(1, 0x0064);
    uniform_int_distribution<int> los(0, 100);

    char kom[] = { '+','-','?','u','U','o' };///50% 20% 10% 5% 10% %5

    //autoreorg ///50% 20% 10% 10% 10%
    //manreorg ///50% 20% 10% 5% 10% %5


    ///rozkl + 50%
    int rozk[6] = { 0 };
    for (int i = 0; i < 10000; i++) {
        int rozklad = los(generator);
        if (rozklad < 50) {//+
            printf("+%d %d %d %d\n", keys(generator), recs(generator), recs(generator), recs(generator));
            rozk[0]++;
        }
        else if (rozklad < 70) {//-
            printf("-%d\n", keys(generator));
            rozk[1]++;
        }
        else if (rozklad < 80) {//?
            printf("?%d\n", keys(generator));
            rozk[2]++;
        }
        else if (rozklad < 85) {//u //85
            printf("u%d %d %d %d\n", keys(generator), recs(generator), recs(generator), recs(generator));
            rozk[3]++;
        }
        else if (rozklad < 95) {//U //95
            printf("U%d %d\n", keys(generator), keys(generator));
            rozk[4]++;
        }
        
        else if (rozklad < 100) {//o
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
    ISFile isfile(4, 0.5);
    char cmd;
    int key = 0,a=0,b=0,h=0;
    int i = 0;
    int sum = 0;
    bool stats = true;

    while (cin >> cmd) {
        nOfReads = nOfWrites = 0;
        if (cmd == 'r') {
            isfile.insertRecord(keys(generator), { recs(generator), recs(generator), recs(generator) });
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
            printf("%d ", isfile.NrecordInMain + isfile.VrecordInOf);
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

    output.open("operacje.txt");
    char kom[] = { '+','-','?','u','U','o' };
    for (int i = 0; i < 6; i++) {
        output << kom[i] << " operacji(r+w): " << operacja[kom[i]].first << " ilosc: " << operacja[kom[i]].second << '\n';
    }
    output.close();




	return 0;
}