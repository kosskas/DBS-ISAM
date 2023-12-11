#include <iostream>
#include <string>
#include <random>
#include "ISFile.h"

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
    for (int i = 0; i < 1000; i++) {
        printf("+%d 1 1 1 \n", 1000-i);
        /*
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
        */
        
    }
    /*
    for (int i = 0; i < 6; i++) {
        printf("%d ", rozk[i]);
    }
    */
    exit(1);
}
int main(int argc, char** argv) {
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> keys(1, 0x0100);//0x01F4
    uniform_int_distribution<int> recs(1, 0x0064);
    
    ////stats

    map<int,int> glowny, indeks, nadmiar;
    map<char, pair<int, int>> operacja;


    //generate();
    

    int wspb = atoi(argv[1]);
    double alfa = atof(argv[2]);
    ISFile isfile(wspb, alfa);

    char cmd;
    int key = 0,a=0,b=0,h=0;
    int i = 0;
    int sum = 0;
    int reorgop = 0;

    while (cin >> cmd) {
        nOfReads = nOfWrites = 0;
        reorgop = 0;
        if (cmd == 'r') {
            int key = keys(generator);
            printf("Wylosowano klucz %d\n", key);
            isfile.insertRecord(key, { recs(generator), recs(generator), recs(generator) },&reorgop);
        }
        if (cmd == '+') {
            cin >> key >> a >> b >> h;  
            isfile.insertRecord(key, { a, b, h },&reorgop);
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
        if (cmd == 'o')
            isfile.reorganiseFile();
        if (cmd == 'n')
            isfile.info(0.5);
        if (cmd == 'c')
            isfile.clearFile();
        if (cmd == 'q')
            break;
        i++;
        if (reorgop) {
            operacja['o'].first += reorgop;
            operacja['o'].second++;
        }
        operacja[cmd].first += nOfWrites + nOfReads;
        operacja[cmd].second++;
        if (((isfile.NrecordInMain + isfile.VrecordInOf) % 10 == 0 && (isfile.NrecordInMain + isfile.VrecordInOf) != sum)) {
            sum = (isfile.NrecordInMain + isfile.VrecordInOf);
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
    

    string nazwa = "operacje" + string(to_string(wspb)) + "-" + string(to_string(alfa)) + ".txt";
    output.open(nazwa);
    char kom[] = { '+','-','?','u','U','o' };
    for (int i = 0; i < 6; i++) {
        output << kom[i] << " operacji(r+w): " << operacja[kom[i]].first << " ilosc: " << operacja[kom[i]].second << '\n';
    }
    output.close();

	return 0;
}