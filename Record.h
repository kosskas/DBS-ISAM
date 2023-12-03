#pragma once
struct Data {
	int a, b, h; //12
	Data() : a(0), b(0), h(0) {}
	Data(int a, int b, int h) : a(a), b(b), h(h) {}
};
struct Record {
	int key; //4
	Data data; //12
	short deleted;//2
	short ofptr;//2
	Record() : key(0), data({0,0,0}), deleted(0), ofptr(0) {}
	Record(int key, Data data) : key(key), data(data), deleted(0), ofptr(0) {}
};