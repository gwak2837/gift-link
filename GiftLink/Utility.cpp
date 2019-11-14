#define _CRT_SECURE_NO_WARNINGS	/* localtime() */
#include <iostream>
#include <cassert>
#include "KISA_SHA256.h"
#include "Utility.h"
using namespace std;

ostream & operator<<(ostream& o, const uint8_t * hash) {
	o << "0x";
	for (int i = 0; i < SHA256_DIGEST_VALUELEN; i++) {
		o.width(2);
		o.fill('0');
		o << hex << (int)hash[i];
	}
	o << dec;
	return o;
}

void printInHex(ostream& o, const uint8_t * hash, int hashLength, string prefix) {
	o << prefix << "0x";
	for (int i = 0; i < hashLength; i++) {
		o.width(2);
		o.fill('0');
		o << hex << (int)hash[i];
	}
	o << '\n';
	o << dec;
}

bool isMemoryEqual(const void * a, const void * b, int byteSize) {
	assert(byteSize % 4 == 0);

	int * p1 = (int *)a;
	int * p2 = (int *)b;

	int intSize = byteSize / sizeof(int);
	for (int i = 0; i < intSize; i++, p1++, p2++) {
		if (*p1 != *p2)
			return false;
	}

	return true;
}

string timeToString(time_t t) {		// -> 분, 초는 2자리 고정에 비어있는 자리는 0으로 채우기...어떻게?
	struct tm * date = localtime(&t);
	string result = to_string(date->tm_year + 1900) + ". " + to_string(date->tm_mon + 1) + ". " + to_string(date->tm_mday)
		+ ". " + to_string(date->tm_hour) + ':' + to_string(date->tm_min) + ':' + to_string(date->tm_sec);
	return result;
}


ostream & operator<<(ostream & o, const Type & type) {
	if (type == Type())
		return o << type.name;
	else
		return o << type.name << ", " << type.faceValue << ", " << type.marketValue << ", " << timeToString(type.expirationDate);
}

ostream & operator<<(ostream& o, const State & state) {
	switch (state) {
	case State::OWN:
		o << "OWN";
		break;
	case State::SALE:
		o << "SALE";
		break;
	case State::SPENT:
		o << "SPENT";
		break;
	default:
		o << "UNDEFINED...\n";
		break;
	}

	return o;
}