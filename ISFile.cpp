#include "ISFile.h"
#define clearBuffer(x) memset(x, 0, sizeof(Record) * BUFFSIZE)

ISFile::ISFile(uint32_t BUFFSIZE) {
	fileswitcher = true;
	this->BUFFSIZE = BUFFSIZE;
	this->IDXBUFFSIZE = BUFFSIZE;
	filename = "file0";
	ofname = "of0";
	idxname = "idx0";
	file = new BFile(filename, BUFFSIZE, 1);
	overflow = new BFile(ofname, BUFFSIZE, 1);

	file->buffer[0].key = -1;
	file->buffer[0].data = { -1,-1,-1 };
	file->writeBlock(0);

	idx = createIndex(idxname, 1);
	bf = ceil(double(sizeof(Record) * BUFFSIZE) / (sizeof(Record)+ sizeof(int)));
	bi = ceil(double(sizeof(Record) * BUFFSIZE) / (sizeof(int) + sizeof(int)));
	printf("bf = %d\nbi = %d\n", bf, bi);

	NrecordInMain = 1;
	VrecordInOf = 0;
	maxOFsize = BUFFSIZE;
}

Index* ISFile::createIndex(string idxName, int nOfpages) {
	Index* tmp = new Index(IDXBUFFSIZE, nOfpages, idxName, ios::binary | ios::in | ios::out | ios::trunc);
	file->resetPtr();
	int bytesRead = 0;
	int page = 0;
	while (bytesRead = file->readBlock(page)) {
		//we� pierwszy i go zapisz do indeksu
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
	//PAGE MO�E BY� 0!! bo znalaz�
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

//to te� blokowo?
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
	return temp;
}
/*
		///Przypadek gdy nowa niezaindeksowana strona jest wolna - nie ma takiego przypadku
		BUGI gdy jest wi�cej stron! NIE MA TAKIEGO PRZYPADKU!
		Domin m�wi:
			Gdy dodajesz na OF to pilnuj kolejno�ci. Przechodzisz po OF i i sprawdzasz klucze, je�li mniejszy to w to miejsce zapisujesz i szukasz miejsca dla wi�kszego klucza (starszego rekordu)

*/
void ISFile::insertRecord(int key, Data data) {
	//przebieg gdy jest index i s� strony
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

void ISFile::removeRecord(int key) {
	//oznacz jako del
	int found = 0;
	Record frec;
	int page = searchRecord(key, &found,&frec, true);
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
	//Usu�
	int found = 0;
	Record frec;
	int page = searchRecord(oldkey, &found, &frec, true);
	if (found) {
		insertRecord(newkey, frec.data);
	}
}

void ISFile::insertToOf(int key, Data data, short int* ptr) {
///sprawdz czy juz inny z ov nie wskazuje na niego?
///je�li of pe�en to reogranizuj
	int page = 0;
	int bytesRead = 0;
	short offset = 0;
	short del = 0;

	int savedPage = -1;

	///Odczytaj stron� od
	while (bytesRead = overflow->readBlock(page)) {
		//znajdz czy taki jest
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (*ptr == offset) {
				//ju� lista
				if (key < overflow->buffer[i].key) {
					//podmie�
					Data temp = data;
					int tempk = key;

					data = overflow->buffer[i].data;
					key = overflow->buffer[i].key;
					del = overflow->buffer[i].deleted;

					overflow->buffer[i].data = temp;
					overflow->buffer[i].key = tempk;
					overflow->buffer[i].deleted = 0;
					printf("SWAP\n");
					///JE�LI JEST ZMIANA STRONY TO WSKA�NIKI �LE WSKAZUJ�
				}
				ptr = &overflow->buffer[i].ofptr;
			}
			if (overflow->buffer[i].key == 0) {
				printf("znaleziono miejsce w oF\n");
				//wstaw w wolne miejce
				overflow->buffer[i].key = key;
				overflow->buffer[i].data = data;
				overflow->buffer[i].deleted = del;

				//Zaktualizuj wska�niki
				*ptr = offset;

				//zapisz rekord
				overflow->writeBlock(page);

				//odk�adanie wska�nik�w na stos?
				
				
				VrecordInOf++;
				return;
			}
		}
		page++;
	}
}

void ISFile::updateOFPtrs() {
}

//Raczej nie dojdzie do sytuacji przy reorganizacji �e jakie� strony b�d� puste
//Tyle ile stron tyle w idx, nie ma rozsz idxa
void ISFile::reorganiseFile(double alpha) {
	printStruct();
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
	//TO ZLE LICZY!!!
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
			//Jest �a�cuch przepe�nien
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
				if (savedIdx > alpha * bf) {
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
	//Nie wypisuj je�li del

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
	//TODO
	file->clearFile();
	overflow->clearFile();
	idx->clearFile();

	file->buffer[0].key = -1;
	file->buffer[0].data = { -1,-1,-1 };
	file->writeBlock(0);

	if (idx)
		delete idx;
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