#pragma once
#ifndef TRANSACTION_H
#define TRANSACTION_H
#define COINBASE_REWARD 50
#include <iostream>
#include <cstdint>
#include <vector>
#include <ctime>
#include <cmath>
#include <cassert>
#include <string>
#include "KISA_SHA256.h"
#include "uECC.h"

enum class TxType {
	one, two, three, four
};


enum class State {
	own, sale, spent
};

class Type {
public:
	std::string name;
	std::int64_t faceValue;
	std::int64_t marketValue;
	time_t expirataionDate;

	Type() : name("GiftLink Coin"), faceValue(0), marketValue(0), expirataionDate((time_t)pow(2, sizeof(time_t) * 8 - 1) - 1) {}					// GLC 생성 시
	Type(std::string _n, std::int64_t _f, std::int64_t _m, time_t _e) : name(_n), faceValue(_f), marketValue(_m), expirataionDate(_e) {}	// 유가증권 이름, 액면가, 시장가, 유효기간 설정

	void print(std::ostream & o) const;

	friend bool operator==(const Type &, const Type &);
	friend bool operator!=(const Type &, const Type &);
	friend bool operator<(const Type &, const Type &);
};

class Output {
public:
	std::uint8_t recipientPublicKeyHash[SHA256_DIGEST_VALUELEN];	// P2PKH. pubKeyHash(32 bytes)
	std::int64_t value;
	Type type;
	State state;

	Output(const std::uint8_t * _recipientPublicKeyHash, std::int64_t _value, Type _type, State _state);

	void print(std::ostream & o) const;

	friend bool operator==(const Output & obj, const Output & obj2);
	friend bool operator!=(const Output & obj, const Output & obj2);
};

class Input {
public:
	std::uint8_t previousTxHash[SHA256_DIGEST_VALUELEN];
	int outputIndex;
	std::uint8_t signature[SECP256R1_NUM_BYTES * 2];		// P2PKH. signature(64 bytes). 이전 Output의 PubKeyHash(32 byte)만 복사해서 해싱.
	std::uint8_t senderPublicKey[SECP256R1_NUM_BYTES * 2];	// P2PKH. pubKey(64 bytes). 해싱 안 함.

	std::uint64_t blockHeight;								// UTXO가 어느 블록에 있는지 참고하는 용도. 해싱 안 함.

	Input();												// coinbase transaction input
	Input(const std::uint8_t * previousOutputPublicKeyHash, const std::uint8_t * _senderPublicKey, 
		const std::uint8_t * _previousTxHash, int _outputIndex, std::uint64_t _blockHeight);	// normal transaction input

};

class Transaction {
public:
	/* Transaction Data */
	std::vector<Input> inputs;
	std::vector<Output> outputs;

	int version;
	time_t timestamp;
	std::string memo;
	//int relayedBy;
	
	/* Transaction Information */
	std::uint8_t txHash[SHA256_DIGEST_VALUELEN];

	Transaction() {};
	Transaction(std::vector<Input> & _inputs, std::vector<Output> & _outputs, int _version, std::string _memo = "");

	uint8_t * createTxData() const;				// 서명에 참조할 Output의 PubKeyHash를 넣은 상태에서 해싱할 원본 데이터를 추출하는 과정
	size_t getTxDataSize() const;				// 해싱할 원본 데이터의 크기(byte)

	bool isCoinbase() const;
	bool isValid() const;
	void print(std::ostream & o) const;
};



class UTXO {
public:
	std::uint8_t txHash[SHA256_DIGEST_VALUELEN];
	Output output;
	int outputIndex;
	std::uint64_t blockHeight;
	
	UTXO(const std::uint8_t * _txHash, Output _output, int _outputIndex, std::uint64_t _blockHeight);

	void print(std::ostream & o) const;

	friend bool operator<(const UTXO & utxo1, const UTXO & utxo2);		// value를 기준으로 내림차순 정렬
};


#endif