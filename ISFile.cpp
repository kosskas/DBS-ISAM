#include "ISFile.h"
#define clearBuffer(x) memset(x, 0, sizeof(Record) * BUFFSIZE)

ISFile::ISFile(uint32_t BUFFSIZE, double alpha) {
	fileswitcher = true;
	this->alpha = alpha;
	this->BUFFSIZE = BUFFSIZE;
	this->IDXBUFFSIZE = ceil(double(sizeof(Record) * BUFFSIZE) / sizeof(Index::Record));
	filename = "file0";
	ofname = "of0";
	idxname = "idx0";

	setVars();

	file = new BFile(filename, BUFFSIZE, mainPages);
	overflow = new BFile(ofname, BUFFSIZE, ofPages);

	NIL.key = -1;
	NIL.data = { -1,-1,-1 };
	NIL.deleted = 1;
	NIL.ofptr - 0;

	file->buffer[0] = NIL;
	file->writeBlock(0);

	idx = createIndex(idxname, idxPages);

	bf = ceil(double(sizeof(Record) * BUFFSIZE) / (sizeof(Record)));
	bi = ceil(double(sizeof(Record) * BUFFSIZE) / sizeof(Index::Record));
}

void ISFile::setVars() {
	ofPages = 1;
	mainPages = 1;
	idxPages = 1;
	VNratio = 0.1;
	NrecordInMain = 1;
	VrecordInOf = 0;
	realN = 1;
	realV = 0;
	maxOFsize = ofPages * BUFFSIZE;
	idxRecs = 0;
	lastFreeOFPage = 0;
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
			idxRecs++;
		}
		page++;
	}
	return tmp;
}

int ISFile::searchRecord(int key, int* found, Record* rec) {
	file->resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	int bytesRead = file->readBlock(idxpage);
	*found = 0;
	for (int i = 0; i < BUFFSIZE; i++) {
		if (file->buffer[i].key == key) {
			*found = 1;
			*rec = file->buffer[i];
			break;
		}
		else if (i + 1 >= BUFFSIZE || i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i + 1].key > key) {
			if (file->buffer[i].ofptr) {
				searchInOF2(file->buffer[i].ofptr, key, found, rec);
			}
		}
	}
	return idxpage;
}

void ISFile::searchInOF2(int ptr, int key, int* found, Record* rec) {
	int bytesRead = 0;
	int currPtr = ptr;
	int prevPage = -1;
	while (currPtr != 0) {
		int ofpage = ceil(double(currPtr) / BUFFSIZE) - 1;
		int index = (currPtr - 1) % BUFFSIZE;
		if (ofpage != prevPage) {
			bytesRead = overflow->readBlock(ofpage);
		}
		prevPage = ofpage;
		if (overflow->buffer[index].key == key) {
			*found = 1;
			*rec = overflow->buffer[index];
			break;
		}
		currPtr = overflow->buffer[index].ofptr;
	}
}

vector<Record> ISFile::getChain(Record first) {
	vector<Record> temp;
	temp.push_back(first);

	int bytesRead = 0;
	int offset = 0;
	int currPtr = first.ofptr;
	int prevPage = -1;
	
	while (currPtr != 0) {
		int ofpage = ceil(double(currPtr) / BUFFSIZE) - 1;
		int index = (currPtr - 1) % BUFFSIZE;
		if (ofpage != prevPage) {
			bytesRead = overflow->readBlock(ofpage);
		}
		prevPage = ofpage;
		temp.push_back(overflow->buffer[index]);
		currPtr = overflow->buffer[index].ofptr;
	}
	return temp;
}

void ISFile::insertRecord(int key, Data data) {
	int found = 0;
	Record frec;
	frec.key = key;
	frec.data = data;
	int page = searchIfDeleted(key,&found,&frec);
	if (found) {
		printf("Taki klucz juz istnieje\n");
		return;
	}

	int bytesRead = file->readBlock(page);
	///znajdz miejsce
	for (int i = 0; i < BUFFSIZE; i++) {
		if (i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i+1].key == 0) {
			//printf("znaleziono miejsce\n");
			file->buffer[i + 1].key = key;
			file->buffer[i + 1].data = data;

			file->writeBlock(page);
			NrecordInMain++;
			realN++;
			return;
		}
		if (i + 1 >= BUFFSIZE || i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i + 1].key > key) {
			//nie ma mniejsca
			//printf("daj OV");
			insertToOf(key, data, &file->buffer[i].ofptr);
			file->writeBlock(page);
			if (realV == maxOFsize) {
				//printf("\nBufor pelen - reorganizacja\n");
				reorganiseFile();
			}
			return;
		}
	}
	
}

void ISFile::insertToOf(int key, Data data, int *startptr) {
///sprawdz czy juz inny z ov nie wskazuje na niego?
///jeœli of pe³en to reogranizuj
	int ptr = *startptr, next = 0, prev = 0;
	//int prevpage = -1, nextpage = 0;
	/*
	1. Dodano pierwszy raz - zmieñ startprt
	2. Dodano na koniec, zmieñ ¿e przedostatni wskazuje na ost(zmieniajace siê strony) i nie zmieniaj startptra JEST 
	3. Dodano w œrodek of, wsk¹zniki prev i next
	5. Dodano miêdzy tym z maina a of, startptr, next
	*/


	int page = 0;
	int bytesRead = 0;
	int offset = 0,del=0;
	int count = 0;
	int lastPage = -1;

	/*
	je¿eli startptr jest pusty to wstaw w wolne
	jeœli startptr to skacz po stronach
	
	*/
	printf("test");
	while (ptr != 0) {
		int ofpage = ceil(double(ptr) / BUFFSIZE) -1;
		int index = (ptr-1) % BUFFSIZE;
		//bytesRead = overflow->readBlock(ofpage);
		if (ofpage != lastPage) {
			bytesRead = overflow->readBlock(ofpage);
		}
		lastPage = ofpage;


		if (key < overflow->buffer[index].key) {
			next = ptr;
			break;
		}
		//prevpage = ofpage;
		//prev = index+1; ///TO SAMO BO RESZTA Z DZIELENIA??
		prev = ptr;
		ptr = overflow->buffer[index].ofptr;
		
		count++;
	}
	//printf("prev=%d\tnext=%d\n", prev, next);
	page = 0;
	page = lastFreeOFPage;
	printf("lp %d\n", lastFreeOFPage);
	//wstaw na koniec
	while (bytesRead = overflow->readBlock(page)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			//offset++;
			offset = BUFFSIZE*page+i+1; //ZMIENIONO OBLICZANIE NA DYNAMICZNE
			if (overflow->buffer[i].key == 0) {
				//printf("znaleziono miejsce w oF\n");
				//wstaw w wolne miejce
				overflow->buffer[i].key = key;
				overflow->buffer[i].data = data;
				overflow->buffer[i].ofptr = next;

				//Zaktualizuj wskaŸniki
				if (count == 0)
					*startptr = offset; //ten jest za kluczem z maina
				//zapisz rekord
				overflow->writeBlock(page);
				printf("Ostrona %d\n", page);
				lastFreeOFPage = page;
				if (prev) {
					//prev ma wskazywaæ na offset
					bytesRead = overflow->readBlock(ceil(double(prev) / BUFFSIZE) - 1);
					//bytesRead = overflow->readBlock(prevpage);
					overflow->buffer[(prev - 1) % BUFFSIZE].ofptr = offset;
					overflow->writeBlock(ceil(double(prev) / BUFFSIZE) - 1);
					//overflow->writeBlock(prevpage);
				}

				VrecordInOf++;
				realV++;
				return;
			}
		}
		page++;
	}
}

int ISFile::searchIfDeleted(int key, int* found, Record* rec) {
	file->resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	int bytesRead = file->readBlock(idxpage);
	*found = 0;
	for (int i = 0; i < BUFFSIZE; i++) {
		if (file->buffer[i].key == key) {
			*found = 1;
			file->buffer[i].data = rec->data;
			if (file->buffer[i].deleted) {
				file->buffer[i].deleted = 0;
				file->writeBlock(idxpage);
				NrecordInMain++;
			}
			break;
		}
		//else if (file->buffer[i].ofptr) {
		else if (i + 1 >= BUFFSIZE || i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i + 1].key > key) {
			if (file->buffer[i].ofptr) {
				int bytesRead = 0;
				int currPtr = file->buffer[i].ofptr;
				int prevPage = -1;
				while (currPtr != 0) {
					int ofpage = ceil(double(currPtr) / BUFFSIZE) - 1;
					int index = (currPtr - 1) % BUFFSIZE;
					if (ofpage != prevPage) {
						bytesRead = overflow->readBlock(ofpage);
					}
					prevPage = ofpage;
					if (overflow->buffer[index].key == key) {
						*found = 1;
						overflow->buffer[index].data = rec->data;
						if (overflow->buffer[index].deleted) {
							overflow->buffer[index].deleted = 0;
							overflow->writeBlock(ofpage);
							VrecordInOf++;
						}
						break;
					}
					currPtr = overflow->buffer[index].ofptr;
				}
			}

		}
	}
	return idxpage;
}

Record ISFile::removeRecord(int key) {
	Record ret;
	file->resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	int bytesRead = file->readBlock(idxpage);
	for (int i = 0; i < BUFFSIZE; i++) {
		if (file->buffer[i].key == key && !file->buffer[i].deleted) {
			file->buffer[i].deleted = 1;
			file->writeBlock(idxpage);
			printf("Usunieto\n");
			NrecordInMain--;
			return file->buffer[i];
		}
		else if (i + 1 >= BUFFSIZE || i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i + 1].key > key) {
			if (file->buffer[i].ofptr) {
				int bytesRead = 0;
				int currPtr = file->buffer[i].ofptr;
				int prevPage = -1;
				while (currPtr != 0) {
					int ofpage = ceil(double(currPtr) / BUFFSIZE) - 1;
					int index = (currPtr - 1) % BUFFSIZE;
					if (ofpage != prevPage) {
						bytesRead = overflow->readBlock(ofpage);
					}
					prevPage = ofpage;
					if (overflow->buffer[index].key == key && !overflow->buffer[index].deleted) {
						overflow->buffer[index].deleted = 1;
						overflow->writeBlock(ofpage);
						printf("Usunieto\n");
						VrecordInOf--;
						return overflow->buffer[index];
					}
					currPtr = overflow->buffer[index].ofptr;
				}
			}
		}
	}
	printf("Nie ma takiego rekordu\n");
	return ret;
}

void ISFile::updateRecord(int key, Data data) {
	file->resetPtr();
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	int bytesRead = file->readBlock(idxpage);
	for (int i = 0; i < BUFFSIZE; i++) {
		if (file->buffer[i].key == key && !file->buffer[i].deleted) {
			file->buffer[i].data = data;
			file->writeBlock(idxpage);
			return;
		}
		else if (i + 1 >= BUFFSIZE || i + 1 < BUFFSIZE && file->buffer[i].key < key && file->buffer[i + 1].key > key) {
			if (file->buffer[i].ofptr) {
				int bytesRead = 0;
				int currPtr = file->buffer[i].ofptr;
				int prevPage = -1;
				while (currPtr != 0) {
					int ofpage = ceil(double(currPtr) / BUFFSIZE) - 1;
					int index = (currPtr - 1) % BUFFSIZE;
					if (ofpage != prevPage) {
						bytesRead = overflow->readBlock(ofpage);
					}
					prevPage = ofpage;
					if (overflow->buffer[index].key == key && !overflow->buffer[index].deleted) {
						overflow->buffer[index].data = data;
						overflow->writeBlock(ofpage);
						return;
					}
					currPtr = overflow->buffer[index].ofptr;
				}
			}
		}
	}
}

void ISFile::updateRecord(int oldkey, int newkey) {
	//Usuñ
	int found = 0;
	Record frec = removeRecord(oldkey);
	if(frec.key != 0)
		insertRecord(newkey, frec.data);
}
//Raczej nie dojdzie do sytuacji przy reorganizacji ¿e jakieœ strony bêd¹ puste
//Tyle ile stron tyle w idx, nie ma rozsz idxa
void ISFile::reorganiseFile() {
	//printf("reorg");
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
	int Sonew = ceil(double(VNratio * NrecordInMain));

	NrecordInMain = 1;
	VrecordInOf = 0;
	//printf("Bedzie %d stron\nBedzie %d stron indeksu\nBedzie %d stron nadmiaru",Snnew, Sinew, Sonew);
	
	BFile* newfile = new BFile(newfilename, BUFFSIZE, Snnew);
	BFile* newof = new BFile(newofname, BUFFSIZE, Sonew);

	file->resetPtr();
	int bytesRead = 0, page = 0;

	int savedIdx = 0;
	int savedBlockNo = 0;

	newfile->buffer[savedIdx++] = NIL;

	while (bytesRead = file->readBlock(page++)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			//Jest ³añcuch przepe³nien
			if (file->buffer[i].ofptr != 0) {
				vector<Record> chain = getChain(file->buffer[i]);
				for (int j = 0; j < chain.size(); j++) {
					if (!chain[j].deleted && chain[j].key != 0) {
						chain[j].ofptr = 0;
						newfile->buffer[savedIdx++] = chain[j];
						NrecordInMain++;
						//printf("Zapisano %d\n", chain[j].key);
						if (savedIdx >= (double)(alpha * bf)) {
							newfile->writeBlock(savedBlockNo++);
							savedIdx = 0;
							clearBuffer(newfile->buffer);
						}
					}
				}
				if (savedIdx >= (double)(alpha * bf)) {
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
				//printf("Zapisano %d\n", file->buffer[i].key);
				if (savedIdx >= (double)(alpha * bf)) {
					newfile->writeBlock(savedBlockNo++);
					savedIdx = 0;
					clearBuffer(newfile->buffer);
				}
			}	
		}
	}
	if (savedIdx > 0) {
		newfile->writeBlock(savedBlockNo++);
		savedIdx = 0;
		clearBuffer(newfile->buffer);
	}
	delete file;
	file = newfile;
	delete overflow;
	overflow = newof;
	delete idx;
	idx = createIndex(newidxname, Sinew);
	maxOFsize = Sonew * BUFFSIZE;

	realN = NrecordInMain;
	realV = 0;
	lastFreeOFPage = 0;
	mainPages = Snnew;
	ofPages = Sonew;
	idxPages = Sinew;

	filename = newfilename;
	ofname = newofname;
	idxname = newidxname;
}

void ISFile::info(double alpha) {
	printf("bf = %d\tbi = %d\n", bf, bi);
	printf("Stron na ob. glowny %d\tStron na of %d\tStron na idx %d\n",mainPages,ofPages,idxPages);
	printf("Jest rekordow nieusunietych: %d\tW nadmiarze: %d\n", NrecordInMain, VrecordInOf);
	printf("Jest rekordow wszystkich: %d\tW nadmiarze: %d\n", realN, realV);
	printf("Wsp V/N %lf\n", VNratio);
	int Snnew = ceil(double(NrecordInMain + VrecordInOf) / double(bf * alpha));
	int Sinew = ceil(double(Snnew) / double(bi));
	int Sonew = ceil(double(VNratio * NrecordInMain));
	printf("Alfa %lf\n", alpha);
	printf("Bedzie %d stron\tBedzie %d stron indeksu\tBedzie %d stron nadmiaru\n", Snnew, Sinew, Sonew);
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

void ISFile::getFileSize(int *filesize, int* ovsize, int* idxsize) {
	*filesize = GetFileSize(filename);//BUFFSIZE * mainPages;
	*ovsize = GetFileSize(ofname);//BUFFSIZE * ofPages;
	*idxsize = GetFileSize(idxname);
	//printf("m=%d o=%d i=%d\n",filesize, ovsize, idxsize);
	//printf("Plik %d\tOv %d\tIdx %d\n", filesize, ovsize, idxsize);
}

void ISFile::clearFile() {
	delete file;
	delete overflow;
	delete idx;

	setVars();
	file = new BFile(filename, BUFFSIZE, 1);
	overflow = new BFile(ofname, BUFFSIZE, 1);

	file->buffer[0] = NIL;
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

int GetFileSize(string filename) {
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

/*
	/*SEARCHINOF
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
	return page - 1;
}
*/


