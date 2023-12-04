#include "ISFile.h"
#define clearBuffer(x) memset(x, 0, sizeof(Record) * BUFFSIZE)

ISFile::ISFile(uint32_t BUFFSIZE) {
	fileswitcher = true;
	this->BUFFSIZE = BUFFSIZE;
	this->IDXBUFFSIZE = BUFFSIZE;
	filename = "file0";
	ofname = "of0";
	idxname = "idx0";

	int ofpages = 1;
	
	file = new BFile(filename, BUFFSIZE, 1);
	overflow = new BFile(ofname, BUFFSIZE, ofpages);

	NIL.key = -1;
	NIL.data = { -1,-1,-1 };
	NIL.deleted = 1;
	NIL.ofptr - 0;

	file->buffer[0] = NIL;
	file->writeBlock(0);

	idx = createIndex(idxname, 1);
	/**
	JAK TO LICZYÆ
	
	*/
	bf = ceil(double(sizeof(Record) * BUFFSIZE) / (sizeof(Record)));
	bi = ceil(double(sizeof(Record) * BUFFSIZE) / (sizeof(int) + sizeof(int)));
	printf("bf = %d\nbi = %d\n", bf, bi);

	NrecordInMain = 1;
	VrecordInOf = 0;
	maxOFsize = ofpages*BUFFSIZE;
}

Index* ISFile::createIndex(string idxName, int nOfpages) {
	Index* tmp = new Index(IDXBUFFSIZE, nOfpages, idxName, ios::binary | ios::in | ios::out | ios::trunc);
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

int ISFile::searchRecord(int key, int* found, Record* rec, bool del) {
	file->resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	//PAGE MO¯E BYÆ 0!! bo znalaz³
	int bytesRead = file->readBlock(idxpage);

	//sprwadz czy nie jest w of
	for (int i = 0; i < BUFFSIZE; i++) {
		if (file->buffer[i].key == key) {
			exists = 1;
			*rec = file->buffer[i];
			if (del) {
				file->buffer[i].deleted = 1;
				file->writeBlock(idxpage);
			}
		}	
	}
	*found = exists;
	if (!exists) {
		searchInOF(key, found, rec, del);
	}

	return idxpage;
}

int ISFile::searchInOF(int key, int* found, Record* rec, bool del) {
	overflow->resetPtr();
	int page = 0;
	int bytesRead = 0;
	int exists = 0;
	while (bytesRead = overflow->readBlock(page)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			if (overflow->buffer[i].key == key) {
				exists = 1;
				*rec = overflow->buffer[i];
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
	while (currPtr != 0) {
		int ofpage = ceil(double(currPtr) / BUFFSIZE) - 1;
		int index = (currPtr - 1) % BUFFSIZE;
		bytesRead = overflow->readBlock(ofpage);
		temp.push_back(overflow->buffer[index]);
		currPtr = overflow->buffer[index].ofptr;
	}
	return temp;
}
/*
		///Przypadek gdy nowa niezaindeksowana strona jest wolna - nie ma takiego przypadku
		BUGI gdy jest wiêcej stron! NIE MA TAKIEGO PRZYPADKU!
*/
void ISFile::insertRecord(int key, Data data) {
	int found = 0;
	Record frec;
	int page = searchRecord(key,&found,&frec);
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
			if (VrecordInOf == maxOFsize) {
				printf("\nBufor pelen - reorganizacja\n");
				reorganiseFile(0.5);
			}
			return;
		}
		///przypadek gdy key == key, na nowo wstawiamy usuniety klucz

	}
	
}

void ISFile::insertToOf(int key, Data data, short *startptr) {
///sprawdz czy juz inny z ov nie wskazuje na niego?
///jeœli of pe³en to reogranizuj
	short ptr = *startptr, next = 0, prev = 0;
	int prevpage = -1, nextpage = 0;
	/*
	1. Dodano pierwszy raz - zmieñ startprt
	2. Dodano na koniec, zmieñ ¿e przedostatni wskazuje na ost(zmieniajace siê strony) i nie zmieniaj startptra JEST 
	3. Dodano w œrodek of, wsk¹zniki prev i next
	5. Dodano miêdzy tym z maina a of, startptr, next
	*/


	int page = 0;
	int bytesRead = 0;
	short offset = 0,del=0;
	int count = 0;


	/*
	je¿eli startptr jest pusty to wstaw w wolne
	jeœli startptr to skacz po stronach
	
	*/
	while (ptr != 0) {
		int ofpage = ceil(double(ptr) / BUFFSIZE) -1;
		int index = (ptr-1) % BUFFSIZE;
		bytesRead = overflow->readBlock(ofpage);
		if (key < overflow->buffer[index].key) {
			//next = index + 1;
			next = ptr;
			break;
		}
		prevpage = ofpage;
		//prev = index+1;
		prev = index+1;
		ptr = overflow->buffer[index].ofptr;
		count++;
	}
	page = 0;
	//wstaw na koniec
	while (bytesRead = overflow->readBlock(page)) {
		//znajdz czy taki jest
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (overflow->buffer[i].key == 0) {
				printf("znaleziono miejsce w oF\n");
				//wstaw w wolne miejce
				overflow->buffer[i].key = key;
				overflow->buffer[i].data = data;
				overflow->buffer[i].ofptr = next;

				//Zaktualizuj wskaŸniki
				if (count == 0)
					*startptr = offset; //ten jest za kluczem z maina
				//zapisz rekord
				overflow->writeBlock(page);
				if (prev) {
					//prev ma wskazywaæ na offset
					bytesRead = overflow->readBlock(prevpage);
					overflow->buffer[(prev - 1) % BUFFSIZE].ofptr = offset;
					overflow->writeBlock(prevpage);
				}

				VrecordInOf++;
				return;
			}
		}
		page++;
	}
}

void ISFile::removeRecord(int key) {
	//oznacz jako del
	int found = 0;
	Record frec;
	int page = searchRecord(key, &found, &frec, true);
	if (found)
		printf("Usunieto\n");
	else
		printf("Nie ma takiego rekordu\n");
	return;
}

void ISFile::updateRecord(int key, Data data) {
	file->resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	int bytesRead = file->readBlock(idxpage);

	//sprwadz czy nie jest w of
	for (int i = 0; i < BUFFSIZE; i++) {
		if (file->buffer[i].key == key) {
			file->buffer[i].data = data;
			file->buffer[i].deleted = 0;
			file->writeBlock(idxpage);
			return;
		}
	}
	if (!exists) {
		int page = 0;
		while (bytesRead = overflow->readBlock(page)) {
			for (int i = 0; i < BUFFSIZE; i++) {
				if (overflow->buffer[i].key == key) {
					overflow->buffer[i].data = data;
					overflow->buffer[i].deleted = 0;
					overflow->writeBlock(page);
					return;
				}
			}
			page++;
		}
	}
}

void ISFile::updateRecord(int oldkey, int newkey) {
	//Usuñ
	int found = 0;
	Record frec;
	int page = searchRecord(oldkey, &found, &frec, true);
	if (found) {
		insertRecord(newkey, frec.data);
	}
}
//Raczej nie dojdzie do sytuacji przy reorganizacji ¿e jakieœ strony bêd¹ puste
//Tyle ile stron tyle w idx, nie ma rozsz idxa
void ISFile::reorganiseFile(double alpha) {
	printf("reorg");
	string newfilename, newidxname, newofname;
	if (fileswitcher) {
		newfilename = "file1";
		newidxname = "idx1";
		newofname = "of1";
	}
	else {
		newfilename = "file0";
		newidxname = "idx0";
		newofname = "of0";
	}
	fileswitcher = !fileswitcher;

	int Snnew = ceil(double(NrecordInMain + VrecordInOf) / double(bf * alpha));
	int Sinew = ceil(double(Snnew) / double(bi));
	int Sonew = ceil(double(0.333 * NrecordInMain));

	NrecordInMain = 0;
	VrecordInOf = 0;
	printf("Bedzie %d stron\nBedzie %d stron indeksu\nBedzie %d stron nadmiaru",Snnew, Sinew, Sonew);
	
	BFile* newfile = new BFile(newfilename, BUFFSIZE, Snnew);
	BFile* newof = new BFile(newofname, BUFFSIZE, Sonew);

	file->resetPtr();
	int bytesRead = 0, page = 0;

	int savedIdx = 0;
	int savedBlockNo = 0;

	while (bytesRead = file->readBlock(page++)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			//Jest ³añcuch przepe³nien
			if (file->buffer[i].ofptr != 0) {
				vector<Record> chain = getChain(file->buffer[i]);
				for (int j = 0; j < chain.size(); j++) {
					if (chain[j].key == -1 || !chain[j].deleted && chain[j].key != 0) {
						chain[j].ofptr = 0;
						newfile->buffer[savedIdx++] = chain[j];
						NrecordInMain++;
						printf("Zapisano %d\n", chain[j].key);
						if (savedIdx >= alpha * bf) {
							newfile->writeBlock(savedBlockNo++);
							savedIdx = 0;
							clearBuffer(newfile->buffer);
						}
					}
				}
				if (savedIdx >= alpha * bf) {
					newfile->writeBlock(savedBlockNo++);
					savedIdx = 0;
					clearBuffer(newfile->buffer);
				}
				continue;
			}
			//jesli jest usuniety to nastepny
			if (file->buffer[i].key == -1 || !file->buffer[i].deleted && file->buffer[i].key != 0) {
				file->buffer[i].ofptr = 0;
				newfile->buffer[savedIdx++] = file->buffer[i];
				NrecordInMain++;
				printf("Zapisano %d\n", file->buffer[i].key);
				if (savedIdx >= alpha * bf) {
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
	delete file;
	file = newfile;
	delete overflow;
	overflow = newof;
	delete idx;
	idx = createIndex(newidxname, Sinew);
	maxOFsize = Sonew * BUFFSIZE;
}

void ISFile::info(double alpha) {
	printf("bf = %d\nbi = %d\n", bf, bi);
	printf("Strona dyskowa: %d wpisy\nStrona indeksu: %d wpisy\n", BUFFSIZE, IDXBUFFSIZE);
	printf("Jest rekordow: %d\nW nadmiarze: %d\n", NrecordInMain, VrecordInOf);
	int Snnew = ceil(double(NrecordInMain + VrecordInOf) / double(bf * alpha));
	int Sinew = ceil(double(Snnew) / double(bi));
	int Sonew = ceil(double(0.333 * NrecordInMain));
	printf("Alfa %lf\n", alpha);
	printf("Bedzie %d stron\nBedzie %d stron indeksu\nBedzie %d stron nadmiaru", Snnew, Sinew, Sonew);
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
					if (!vec[j].deleted)
						printf(" %d -> ", vec[j].key);
				}
				printf(" }");

			}
			else if (file->buffer[i].key != 0) {
				if (!file->buffer[i].deleted)
					printf(" %d -> ", file->buffer[i].key);
			}
		}
	}
}

void ISFile::printIndex() {
	idx->printIndex();
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

void ISFile::clearFile() {
	delete file;
	delete overflow;
	delete idx;
	file = new BFile(filename, BUFFSIZE, 1);
	overflow = new BFile(ofname, BUFFSIZE, 1);

	file->buffer[0].key = -1;
	file->buffer[0].data = { -1,-1,-1 };
	file->writeBlock(0);

	idx = createIndex(idxname, 1);
}

ISFile::~ISFile() {
	if (idx)
		delete idx;
	if (file)
		delete file;
	if (overflow)
		delete overflow;
}