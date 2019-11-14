#pragma once
#ifndef UTILITY_H
#define UTILITY_H

// ������<< �������̵� �ϰ� ���� �� �� .cpp�� Utility.h�� �߰�

#include <iostream>
#include <ctime>			// localtime()
#include <string>			// string
#include "Transaction.h"	// Type, State

std::ostream & operator<<(std::ostream & o, const uint8_t * hash);
std::ostream & operator<<(std::ostream & o, const Type & type);
std::ostream & operator<<(std::ostream & o, const State & state);

void printInHex(std::ostream& o, const std::uint8_t * hash, int hashLength, std::string prefix = "");

bool isMemoryEqual(const void * a, const void * b, int size);	// (�ַ�) ũ�� 32 bytes�� �� �޸� �� ��
std::string timeToString(time_t t);


#endif