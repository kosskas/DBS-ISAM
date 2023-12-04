#include <iostream>
#include <string>
#include <math.h>
#include "Index.h"
#include "ISFile.h"
#include <random>
using namespace std;


int main(int argc, char** argv) {
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> keys(1, 0xFF);
    uniform_int_distribution<int> recs(1, 0x25);

    ISFile isfile(4);
    isfile.printStruct();
    char cmd;
    int key = 0,a=0,b=0,h=0;
    while (cin >> cmd) {
        nOfReads = nOfWrites = 0;
        if (cmd == '+') {
            cin >> key >> a >> b >> h;
            isfile.insertRecord(key, { a, b, h });
        }
        if (cmd == 'w') {
            isfile.insertRecord(keys(generator), { recs(generator), recs(generator), recs(generator) });
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
            if (exits)
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
        if (cmd == 'r')
            isfile.reorganiseFile(0.5);
        if (cmd == 'n')
            isfile.info(0.5);
        if (cmd == 'c')
            isfile.clearFile();
        if (cmd == 'q')
            break;
        printf("\nBylo zapisow %d\t odczytow %d\n", nOfWrites, nOfReads);
    }

    
	return 0;
}