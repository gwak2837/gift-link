#pragma once
#ifndef UTILITY_H
#define UTILITY_H

// ������<< �������̵� �ϰ� ���� �� �� .cpp�� Utility.h�� �߰�

#include <iostream>
#include <ctime>	// localtime()
#include <string>	// string

std::ostream & operator<<(std::ostream& o, const uint8_t * hash);

void printInHex(std::ostream& o, const std::uint8_t * hash, int hashLength, std::string prefix = "");

bool isMemoryEqual(const void * a, const void * b, size_t size);	// (�ַ�) ũ�� 32 bytes�� �� �޸� �� ��
std::string timeToString(time_t t);


#endif