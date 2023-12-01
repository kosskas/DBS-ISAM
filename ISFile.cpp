#include "ISFile.h"
#define clearBuffer(x) memset(x, 0, sizeof(Record) * BUFFSIZE)

ISFile::ISFile(uint32_t BUFFSIZE) {
	swc = true;
	this->BUFFSIZE = BUFFSIZE;
	filename = "file0";
	ofname = "of0";
	idxname = "idx0";
	file = new BFile<Record>(filename, BUFFSIZE, 1);
	overflow = new BFile<Record>(ofname, BUFFSIZE, 1);

	file->buffer[0].key = -1;
	file->buffer[0].data = { -1,-1,-1 };
	file->writeBlock(0);

	idx = createIndex(idxname, 1);
	bf = int((sizeof(Record) * BUFFSIZE) / sizeof(Record));
	bi = int((sizeof(Record) * BUFFSIZE) / (sizeof(int) + sizeof(short int)));
	printf("bf = %d\nbi = %d\n", bf, bi);

	NrecordInMain = 1;
	VrecordInOf = 0;

}


Index* ISFile::createIndex(string idxName, int nOfpages) {
	Index* tmp = new Index(BUFFSIZE, nOfpages, idxName, ios::binary | ios::in | ios::out | ios::trunc);
	file->resetPtr();
	int bytesRead = 0;
	int page = 0;
	while (bytesRead = file->readBlock(page)) {
		//weŸ pierwszy i go zapisz do indeksu
		if (file->buffer[0].key != 0) {
			tmp->writeIdxRecord(file->buffer[0].key, page);
		}
		page++;
	}
	return tmp;
}

int ISFile::searchRecord(int key, int* found, bool del) {

	file->resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	//PAGE MO¯E BYÆ 0!! bo znalaz³
	int bytesRead = file->readBlock(idxpage);

	//sprwadz czy nie jest w of
	for (int i = 0; i < BUFFSIZE; i++) {
		if (file->buffer[i].key == key) {
			exists = 1;
			if (del) {
				file->buffer[i].deleted = 1;
				file->writeBlock(idxpage);
			}
		}	
	}
	*found = exists;
	if (!exists) {
		searchInOF(key, found, del);
	}
	return idxpage;
}

int ISFile::searchInOF(int key, int* found, bool del) {
	overflow->resetPtr();
	int page = 0;
	int bytesRead = 0;
	int exists = 0;
	while (bytesRead = overflow->readBlock(page)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			if (overflow->buffer[i].key == key) {
				exists = 1;
				if (del) {
					overflow->buffer[i].deleted = 1;
					overflow->writeBlock(page);
				}
			}
		}
		if (exists) {
			break;
		}
		page++;
	}
	*found = exists;
	return page-1;
}

//to te¿ blokowo?
/*
DO ZMIANY
*/
vector<Record> ISFile::getChain(Record first) {
	vector<Record> temp;
	temp.push_back(first);

	int page = 0;
	int bytesRead = 0;
	short offset = 0;
	short currPtr = first.ofptr;
	int end = false;
	while (bytesRead = overflow->readBlock(page++)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (currPtr == offset) {
				temp.push_back(overflow->buffer[i]);
				currPtr = overflow->buffer[i].ofptr;
			}
			if (currPtr == 0) {
				end = true;
				break;
			}
		}
		if (end)
			break;
	}
	sort(temp.begin(), temp.end(), [](Record a, Record b) { return a.key < b.key; });
	return temp;
}
/*
		///Przypadek gdy nowa niezaindeksowana strona jest wolna
		BUGI gdy jest wiêcej stron!
		Domin mówi:
			Gdy dodajesz na OF to pilnuj kolejnoœci. Przechodzisz po OF i i sprawdzasz klucze, jeœli mniejszy to w to miejsce zapisujesz i szukasz miejsca dla wiêkszego klucza (starszego rekordu)

*/
void ISFile::insertRecord(int key, Data data) {
	//przebieg gdy jest index i s¹ strony
	int found = 0;
	int page = searchRecord(key,&found);
	if (found) {
		printf("Taki klucz juz istnieje\n");
		return;
	}

	int bytesRead = file->readBlock(page);
	///znajdz miejsce
	for (int i = 0; i < BUFFSIZE; i++) {
		if (i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i+1].key == 0) {
			printf("znaleziono miejsce\n");
			file->buffer[i + 1].key = key;
			file->buffer[i + 1].data = data;

			file->writeBlock(page);
			NrecordInMain++;
			return;
		}
		if (i + 1 >= BUFFSIZE || i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i + 1].key > key) {
			//nie ma mniejsca
			printf("daj OV");
			insertToOf(key, data, &file->buffer[i].ofptr);
			file->writeBlock(page);
			VrecordInOf++;
			return;
		}
		///przypadek gdy key == key, na nowo wstawiamy usuniety klucz

	}
}

void ISFile::removeRecord(int key) {
	//oznacz jako del
	int found = 0;
	int page = searchRecord(key, &found, true);
	if (found) 
		printf("Usunieto\n");
	else
		printf("Nie ma takiego rekordu\n");
	return;
}

void ISFile::insertToOf(int key, Data data,short int* ptr) {
	
	int page = 0;
	int bytesRead = 0;
	///sprawdz czy juz inny z ov nie wskazuje na niego?
	///jeœli of pe³en to reogranizuj
	short offset = 0;
	///CURRENT POINTER!!!

	int savedLastPageNo=-1, oldPtrIdx=-1;

	while (bytesRead = overflow->readBlock(page++)) {
		//znajdz czy taki jest
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (*ptr == offset) {
				//ju¿ lista
				ptr = &overflow->buffer[i].ofptr;
			}
			if (overflow->buffer[i].key == 0) {
				printf("znaleziono miejsce w oF\n");
				overflow->buffer[i].key = key;
				overflow->buffer[i].data = data;
				*ptr = offset;
				overflow->writeBlock(page - 1);
				return;
			}
		}
	}
}

void ISFile::reorganiseFile(double alpha) {
	printf("reorg");
	string newfilename, newidxname, newofname;
	if (swc) {
		newfilename = "file1";
		newidxname = "idx1";
		newofname = "of1";
	}
	else {
		newfilename = "file0";
		newidxname = "idx0";
		newofname = "of0";
	}
	swc = !swc;
	int Snnew = int((NrecordInMain + VrecordInOf) / (bf * alpha))+1;
	int Sinew = int(Snnew / bi) + 1;
	int Sonew = int(0.333 * NrecordInMain) + 1;

	NrecordInMain = 0;
	VrecordInOf = 0;
	printf("Bedzie %d stron\nBedzie %d stron indeksu\nBedzie %d stron nadmiaru",Snnew, Sinew, Sonew);
	
	BFile<Record>* newfile = new BFile<Record>(newfilename, BUFFSIZE, Snnew);
	BFile<Record>* newof = new BFile<Record>(newofname, BUFFSIZE, Sonew);

	file->resetPtr();
	int bytesRead = 0, page = 0;

	int savedIdx = 0;
	int savedBlockNo = 0;

	while (bytesRead = file->readBlock(page++)) {
		for (int i = 0; i < BUFFSIZE; i++) {

			//Co z zerami?

			//Jest ³añcuch przepe³nien
			//jesli jest usuniety to nastepny
			if (file->buffer[i].ofptr != 0) {
				vector<Record> chain = getChain(file->buffer[i]);
				for (int j = 0; j < chain.size(); j++) {
					if (!chain[j].deleted && chain[j].key != 0) {
						chain[j].ofptr = 0;
						newfile->buffer[savedIdx++] = chain[j];
						NrecordInMain++;
						printf("Zapisano %d\n", chain[j].key);
						if (savedIdx > alpha * bf) {
							newfile->writeBlock(savedBlockNo++);
							savedIdx = 0;
							clearBuffer(newfile->buffer);
						}
					}
				}
				if (savedIdx>0) {
					newfile->writeBlock(savedBlockNo++);
					savedIdx = 0;
					clearBuffer(newfile->buffer);
				}
				continue;
			}
			//jesli jest usuniety to nastepny
			if (!file->buffer[i].deleted && file->buffer[i].key != 0) {
				file->buffer[i].ofptr = 0;
				newfile->buffer[savedIdx++] = file->buffer[i];
				NrecordInMain++;
				printf("Zapisano %d\n", file->buffer[i].key);
				if (savedIdx > alpha * bf) {
					newfile->writeBlock(savedBlockNo++);
					savedIdx = 0;
					clearBuffer(newfile->buffer);
				}
			}	
		}
		if (savedIdx > 0) {
			newfile->writeBlock(savedBlockNo++);
			savedIdx = 0;
			clearBuffer(newfile->buffer);
		}
	}
	///nowy idx nadmiatu

	if (file != NULL) {
		delete file;
	}
	file = newfile;
	delete idx;
	idx = createIndex(newidxname, Sinew);
}

void ISFile::printRecords() {
	//Nie wypisuj jeœli del

	file->resetPtr();
	int bytesRead = 0;
	int page = 0;
	while (bytesRead = file->readBlock(page++)) {
		
		for (int i = 0; i < BUFFSIZE; i++) {
			if (file->buffer[i].ofptr != 0) {
				vector<Record> vec = getChain(file->buffer[i]);
				printf("{ ");
				for (int j = 0; j < vec.size(); j++) {
					if (vec[j].deleted)
						printf(" $%d$ -> ", vec[j].key);
					else
						printf(" %d -> ", vec[j].key);
				}
				printf(" }");

			}
			else if (file->buffer[i].key != 0) {
				if (file->buffer[i].deleted)
					printf(" $%d$ -> ", file->buffer[i].key);
				else
					printf(" %d -> ", file->buffer[i].key);
			}
		}
	}
}

void ISFile::printStruct() {
	file->resetPtr();
	printf("KEY\tDATA\tDEL\tOF\n");
	int bytesRead = 0;
	int page = 0;
	while (bytesRead = file->readBlock(page++)) {
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d\t{%d,%d,%d}\t%hd\t%hd\n", file->buffer[i].key, file->buffer[i].data.a, file->buffer[i].data.b, file->buffer[i].data.h, file->buffer[i].deleted, file->buffer[i].ofptr);
		}
	}
	printOF();
}

void ISFile::printOF() {
	overflow->resetPtr();
	printf("===============\n");
	printf("OVERFLOW AREA\n");
	printf("KEY\tDATA\tDEL\tOF\n");
	int bytesRead = 0;
	int page = 0;
	while (bytesRead = overflow->readBlock(page++)){
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d\t{%d,%d,%d}\t%hd\t%hd\n", overflow->buffer[i].key, overflow->buffer[i].data.a, overflow->buffer[i].data.b, overflow->buffer[i].data.h, overflow->buffer[i].deleted, overflow->buffer[i].ofptr);
		}
	}
}


ISFile::~ISFile() {
	if (idx)
		delete idx;
	if (file)
		delete file;
	if (overflow)
		delete overflow;
}