#include "ISFile.h"
#define clearBuffer(x) memset(x, 0, sizeof(Record) * BUFFSIZE)

ISFile::ISFile(uint32_t BUFFSIZE) {
	swc = true;
	this->BUFFSIZE = BUFFSIZE;
	flags = ios::binary | ios::in | ios::out | ios::trunc;
	initBuffers();

	filename = "file0";
	idxname = "idx0";

	file = createFile(filename, 1);
	
	
	//Specjalny klucz

	Mbuffer[0].key = -1;
	Mbuffer[0].data = {0,0,0};
	writeBlock(file, 0, Mbuffer);
	createOF(file, 1, 1);

	idx = createIndex(idxname, 1);
	bf = int((sizeof(Record) * BUFFSIZE) / sizeof(Record));
	bi = int((sizeof(Record) * BUFFSIZE) / (sizeof(int) + sizeof(short int)));
	printf("bf = %d\nbi = %d\n", bf, bi);

	NrecordInMain = 1;
	VrecordInOf = 0;

}

void ISFile::initBuffers() {
	Mbuffer = NULL;
	Mbuffer = new Record[BUFFSIZE];
	clearBuffer(Mbuffer);

	ofbuffer = NULL;
	ofbuffer = new Record[BUFFSIZE];
	clearBuffer(ofbuffer);

}

fstream* ISFile::createFile(string fileName, int nOfpages) {
	fstream* f = new fstream();
	f->open(fileName, flags);
	int page = 0;
	clearBuffer(Mbuffer);
	while (nOfpages--) {
		writeBlock(f, page++, Mbuffer);
	}
	return f;
}

Index* ISFile::createIndex(string idxName, int nOfpages) {
	Index* tmp = new Index(BUFFSIZE, nOfpages, idxName, flags);
	resetPtr(file);
	int bytesRead = 0;
	int page = 0;
	while (page != ofBlocNo) {
		bytesRead = readBlock(file, page, Mbuffer);
		//weŸ pierwszy i go zapisz do indeksu
		tmp->writeIdxRecord(Mbuffer[0].key, page++);
	}
	return tmp;
}

void ISFile::createOF(fstream* currfile, int blockNo, int nOfpages) {
	ofBlocNo = blockNo;
	int page = ofBlocNo;
	clearBuffer(ofbuffer);
	while (nOfpages--) {
		writeBlock(currfile, page++, ofbuffer);
	}
}

int ISFile::readBlock(fstream* currfile, int blockNum, Record* buffer) {
	resetPtr(currfile);
	currfile->seekg(blockNum * (sizeof(Record) * BUFFSIZE));
	int bytesRead = currfile->read((char*)buffer, sizeof(Record) * BUFFSIZE).gcount();
	if (bytesRead < sizeof(Record) * BUFFSIZE) {
		memset((char*)buffer + bytesRead, 0, sizeof(Record) * BUFFSIZE - bytesRead); //jeœli przeczytano mniej ni¿ ca³¹ stronê, wyzeruj dalsze
	}
	return bytesRead;
}


int ISFile::writeBlock(fstream* currfile, int blockNum, Record* buffer) {
	resetPtr(currfile);
	const char* serialRec = (const char*)buffer;
	currfile->seekp(blockNum * (sizeof(Record) * BUFFSIZE));
	size_t poc = currfile->tellp();
	currfile->write(serialRec, sizeof(Record) * BUFFSIZE);
	size_t written = currfile->tellp();
	written = written - poc;
	printf("Zapisano %d\n", written);
	return written;
}

int ISFile::searchRecord(int key, int* found, bool del) {
	resetPtr(file);
	int idxpage = idx->readIdxRecord(key);
	int exists = 0;
	//PAGE MO¯E BYÆ 0!! bo znalaz³
	int bytesRead = readBlock(file, idxpage, Mbuffer);

	//sprwadz czy nie jest w of
	for (int i = 0; i < BUFFSIZE; i++) {
		if (Mbuffer[i].key == key) {
			exists = 1;
			if (del) {
				Mbuffer[i].deleted = 1;
				writeBlock(file, idxpage, Mbuffer);
			}
			//wstawianie
			else {
				Mbuffer[i].deleted = 0;
				writeBlock(file, idxpage, Mbuffer);
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
	resetPtr(file);
	int page = ofBlocNo;
	int bytesRead = 0;
	int exists = 0;
	while (bytesRead = readBlock(file, page, ofbuffer)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			if (ofbuffer[i].key == key) {
				exists = 1;
				if (del) {
					ofbuffer[i].deleted = 1;
					writeBlock(file, page, ofbuffer);
				}
				//wstawianie
				else {
					ofbuffer[i].deleted = 0;
					writeBlock(file, page, ofbuffer);
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
vector<Record> ISFile::getChain(Record first) {
	vector<Record> temp;
	temp.push_back(first);

	int page = ofBlocNo;
	int bytesRead = 0;
	short offset = 0;
	short currPtr = first.ofptr;
	int end = false;
	while (bytesRead = readBlock(file, page++, ofbuffer)) {
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (currPtr == offset) {
				temp.push_back(ofbuffer[i]);
				currPtr = ofbuffer[i].ofptr;
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

void ISFile::insertRecord(int key, Data data) {
	//przebieg gdy jest index i s¹ strony
	int found = 0;
	int page = searchRecord(key,&found);
	if (found) {
		printf("Taki klucz juz istnieje\n");
		return;
	}

	int bytesRead = readBlock(file, page, Mbuffer);
	///znajdz miejsce
	for (int i = 0; i < BUFFSIZE; i++) {
		if (i + 1 < BUFFSIZE && Mbuffer[i].key < key && Mbuffer[i+1].key == 0) {
			printf("znaleziono miejsce\n");
			Mbuffer[i + 1].key = key;
			Mbuffer[i + 1].data = data;
			writeBlock(file, page, Mbuffer);
			NrecordInMain++;
			return;
		}
		if (i + 1 >= BUFFSIZE || i + 1 < BUFFSIZE && Mbuffer[i].key < key && Mbuffer[i + 1].key > key) {
			//nie ma mniejsca
			printf("daj OV");
			insertToOf(key, data, &Mbuffer[i].ofptr);
			writeBlock(file, page, Mbuffer);
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
	
	int page = ofBlocNo;
	int bytesRead = 0;
	///sprawdz czy juz inny z ov nie wskazuje na niego?
	///jeœli of pe³en to reogranizuj
	short offset = 0;
	///CURRENT POINTER!!!

	int savedLastPageNo=-1, oldPtrIdx=-1;
	while (bytesRead = readBlock(file, page++, ofbuffer)) {
		//znajdz czy taki jest
		for (int i = 0; i < BUFFSIZE; i++) {
			offset++;
			if (*ptr == offset) {
				//ju¿ lista
				ptr = &ofbuffer[i].ofptr;
			}
			if (ofbuffer[i].key == 0) {
				printf("znaleziono miejsce w oF\n");
				ofbuffer[i].key = key;
				ofbuffer[i].data = data;
				*ptr = offset;
				writeBlock(file, page - 1, ofbuffer);
				return;
			}
		}
	}
}

void ISFile::reorganiseFile(double alpha) {
	printf("reorg");
	string newfilename, newidxname;
	if (swc) {
		newfilename = "file1";
		newidxname = "idx1";
	}
	else {
		newfilename = "file0";
		newidxname = "idx0";
	}
	swc = !swc;
	int Snnew = int((NrecordInMain + VrecordInOf) / (bf * alpha))+1;
	int Sinew = int(Snnew / bi) + 1;
	int Sonew = int(0.333 * NrecordInMain) + 1;
	printf("Bedzie %d stron\nBedzie %d stron indeksu\nBedzie %d stron nadmiaru",Snnew, Sinew, Sonew);
	
	fstream* newFile = createFile(newfilename, Snnew);
	
	//createOF(newFile, Snnew, Sonew);
	//Index* newIdx = createIndex(newidxname, Sinew);

	resetPtr(file);
	printf("KEY\tDATA\tDEL\tOF\n");
	int bytesRead = 0, page = 0;

	Record* wrtBuff = new Record[BUFFSIZE];
	clearBuffer(wrtBuff);
	int savedIdx = 0;
	int savedBlockNo = 0;

	while (page != ofBlocNo) {
		int bytesRead = readBlock(file, page++, Mbuffer);
		for (int i = 0; i < BUFFSIZE; i++) {

			//Co z zerami?

			//Jest ³añcuch przepe³nien
			//jesli jest usuniety to nastepny
			if (Mbuffer[i].ofptr != 0) {
				vector<Record> chain = getChain(Mbuffer[i]);
				for (int j = 0; j < chain.size(); j++) {
					if (!chain[j].deleted && chain[j].key != 0) {
						chain[j].ofptr = 0;
						wrtBuff[savedIdx++] = chain[j];
						printf("Zapisano %d\n", chain[j].key);
						if (savedIdx > alpha * bf) {
							writeBlock(newFile, savedBlockNo++, wrtBuff);
							savedIdx = 0;
							clearBuffer(wrtBuff);
						}
					}
				}
				if (savedIdx>0) {
					writeBlock(newFile, savedBlockNo++, wrtBuff);
					savedIdx = 0;
					clearBuffer(wrtBuff);
				}
				continue;
			}
			//jesli jest usuniety to nastepny
			if (!Mbuffer[i].deleted && Mbuffer[i].key != 0) {
				Mbuffer[i].ofptr = 0;
				wrtBuff[savedIdx++] = Mbuffer[i];
				printf("Zapisano %d\n", Mbuffer[i].key);
				if (savedIdx > alpha * bf) {
					writeBlock(newFile, savedBlockNo++, wrtBuff);
					savedIdx = 0;
					clearBuffer(wrtBuff);
				}
			}	
		}
		if (savedIdx > 0) {
			writeBlock(newFile, savedBlockNo++, wrtBuff);
			savedIdx = 0;
			clearBuffer(wrtBuff);
		}
	}
	///nowy idx nadmiatu
	createOF(newFile, savedBlockNo+1, Sonew);
	delete[] wrtBuff;

	if (file != NULL) {
		file->close();
		delete file;
	}
	file = newFile;
	delete idx;
	idx = createIndex(newidxname, Sinew);
}

void ISFile::clearFile() {
	file->close();
	file->open(filename, flags | ios::trunc);
	resetPtr(file);
	idx->clearFile();
	Mbuffer[0].key = -1;
	Mbuffer[0].data = { -1,-1,-1 };
	writeBlock(file, 0, Mbuffer);
	if (idx)
		delete idx;
	idx = createIndex(idxname, 1);
}

void ISFile::printRecords() {
	//Nie wypisuj jeœli del

	resetPtr(file);
	int bytesRead = 0;
	int page = 0;
	while (page != ofBlocNo) {
		bytesRead = readBlock(file, page++, Mbuffer);
		for (int i = 0; i < BUFFSIZE; i++) {
			if (Mbuffer[i].ofptr != 0) {
				vector<Record> vec = getChain(Mbuffer[i]);
				printf("{ ");
				for (int j = 0; j < vec.size(); j++) {
					if (vec[j].deleted)
						printf(" $%d$ -> ", vec[j].key);
					else
						printf(" %d -> ", vec[j].key);
				}
				printf(" }");

			}
			else if (Mbuffer[i].key != 0) {
				if (Mbuffer[i].deleted)
					printf(" $%d$ -> ", Mbuffer[i].key);
				else
					printf(" %d -> ", Mbuffer[i].key);
			}
		}
	}
}

void ISFile::printStruct() {
	resetPtr(file);
	printf("KEY\tDATA\tDEL\tOF\n");
	int bytesRead = 0;
	int page = 0;
	while (page != ofBlocNo) {
		bytesRead = readBlock(file, page++, Mbuffer);
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d\t{%d,%d,%d}\t%hd\t%hd\n", Mbuffer[i].key, Mbuffer[i].data.a, Mbuffer[i].data.b, Mbuffer[i].data.h, Mbuffer[i].deleted, Mbuffer[i].ofptr);
		}
	}
	printOF();
}

void ISFile::printOF() {
	resetPtr(file);
	printf("===============\n");
	printf("OVERFLOW AREA\n");
	printf("KEY\tDATA\tDEL\tOF\n");
	int bytesRead = 0;
	int page = ofBlocNo;
	while (bytesRead = readBlock(file, page++, ofbuffer)){
		printf("\tPrzeczytano %d\n", bytesRead);
		for (int i = 0; i < BUFFSIZE; i++) {
			printf("%d\t{%d,%d,%d}\t%hd\t%hd\n", ofbuffer[i].key, ofbuffer[i].data.a, ofbuffer[i].data.b, ofbuffer[i].data.h, ofbuffer[i].deleted, ofbuffer[i].ofptr);
		}
	}
}

void ISFile::resetPtr(fstream* currfile) {
	currfile->clear();
	currfile->seekg(0, ios::beg);
	currfile->seekp(0, ios::beg);
}

ISFile::~ISFile() {
	if (idx)
		delete idx;
	if (Mbuffer)
		delete[] Mbuffer;
	if (ofbuffer)
		delete[] ofbuffer;
	if (file != NULL) {
		file->close();
		delete file;
	}
}