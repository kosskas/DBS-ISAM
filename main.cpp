#include <iostream>
#include <string>
#include <random>
#include "ISFile.h"

using namespace std;

int main(int argc, char** argv) {
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> keys(1, 0x032);
    uniform_int_distribution<int> recs(1, 0x0064);

    /*
    for (int i = 0; i < 100; i++) {
        printf("+%d %d %d %d\n", keys(generator), recs(generator), recs(generator), recs(generator));
    }
    return 0;
    */

    ISFile isfile(4, 0.5);
    char cmd;
    int key = 0,a=0,b=0,h=0;
    int i = 0;
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
            printf("\nBylo rekorow %d\n",isfile.NrecordInMain+isfile.VrecordInOf);
            isfile.reorganiseFile(0.5);
            printf("\Jest rekorow %d\n", isfile.NrecordInMain + isfile.VrecordInOf);
        }
        if (cmd == 'n')
            isfile.info(0.5);
        if (cmd == 'c')
            isfile.clearFile();
        if (cmd == 'q')
            break;
        i++;
       // printf("\nRekorow %d\n", isfile.NrecordInMain + isfile.VrecordInOf);
       // printf("\n%d\tBylo zapisow %d\t odczytow %d\tRazem %d\n",i, nOfWrites, nOfReads, nOfReads +nOfWrites);
         isfile.printFileSize();
        //printf("%d %d\n",i, nOfWrites+nOfReads);
    }
    //printf("\nProgram zakonczyl sie poprawnie\n");
	return 0;
}