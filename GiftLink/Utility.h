#pragma once
#ifndef UTILITY_H
#define UTILITY_H

// 연산자<< 오버라이딩 하고 싶을 떄 그 .cpp에 Utility.h를 추가

#include <iostream>
#include <ctime>	// localtime()
#include <string>	// string

std::ostream & operator<<(std::ostream& o, const uint8_t * hash);

void printInHex(std::ostream& o, const std::uint8_t * hash, int hashLength, std::string prefix = "");

bool isMemoryEqual(const void * a, const void * b, size_t size);	// (주로) 크기 32 bytes인 두 메모리 값 비교
std::string timeToString(time_t t);


#endif