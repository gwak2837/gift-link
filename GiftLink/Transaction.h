#pragma once
#ifndef TRANSACTION_H
#define TRANSACTION_H
#define COINBASE_REWARD 50
#include <iostream>
#include <cstdint>
#include <vector>
#include <ctime>
#include <string>
#include "KISA_SHA256.h"
#include "uECC.h"

enum class Type {
	GLC, A, B, C, D, E, F, G, H, I
};

class Output {
public:
	std::uint8_t recipientPublicKeyHash[SHA256_DIGEST_VALUELEN];	// P2PKH. pubKeyHash(32 bytes)
	std::int64_t value;
	Type type;

	Output(const std::uint8_t * _recipientPublicKeyHash, std::int64_t _value, Type _type);
};

class Input {
public:
	std::uint8_t previousTxHash[SHA256_DIGEST_VALUELEN];
	int outputIndex;

	std::uint8_t signature[SECP256R1_NUM_BYTES * 2];		// P2PKH. signature(64 bytes)
	std::uint8_t senderPublicKey[SECP256R1_NUM_BYTES * 2];	// P2PKH. pubKey(64 bytes). 해싱 안 함.
	std::uint64_t blockHeight;								// UTXO가 어느 블록에 있는지 참고하는 용도. 해싱 안 함.

	Input();												// coinbase transaction input
	Input(const std::uint8_t * previousOutputPublicKeyHash, const std::uint8_t * _senderPublicKey, 
		const std::uint8_t * _previousTxHash, int _outputIndex, std::uint64_t _blockHeight);	// normal transaction input

};

class Transaction {
public:
	std::uint8_t txHash[SHA256_DIGEST_VALUELEN];

	std::vector<Input> inputs;
	std::vector<Output> outputs;

	int version;
	time_t timestamp;
	std::string memo;
	//int relayedBy;
	
	Transaction(std::vector<Input> & _inputs, std::vector<Output> & _outputs, int _version, std::string _memo = "");

	uint8_t * createTxData() const;				// 서명에 참조할 Output의 PubKeyHash를 넣은 상태에서 해싱할 원본 데이터를 추출하는 과정
	std::uint64_t getTotalCoinOutputs() const;

	inline size_t getTxDataSize() const;		// 해싱할 원본 데이터의 크기(byte)
	inline const std::uint8_t * getTxHash() const;

	bool isCoinbase() const;
	void print(std::ostream & o) const;
};

inline size_t Transaction::getTxDataSize() const {
	return inputs.size() * (sizeof(inputs[0].previousTxHash) + sizeof(inputs[0].outputIndex) + sizeof(outputs[0].recipientPublicKeyHash))
		+ outputs.size() * (sizeof(outputs[0].value) + sizeof(outputs[0].type) + sizeof(outputs[0].recipientPublicKeyHash))
		+ sizeof(version) + sizeof(timestamp) + 4 + memo.length(); // 4는 SigHashCode 크기
}

inline const std::uint8_t * Transaction::getTxHash() const {
	return txHash;
}


#endif