#pragma once
#ifndef BLOCK_H
#define BLOCK_H
#define MAX_TRANSACTION_COUNT 10		// 한 블록에 들어갈 수 있는 최대 transaction의 개수(coinbase Tx 포함)
#define VALID_TIMESTAMP_GAP 60			// 블록의 timestamp의 유효 범위
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <cstdint>
#include <ctime>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/serialization.hpp>
#include "KISA_SHA256.h"
#include "Transaction.h"

class Block {
public:
	/* Block Header */
	int version;											// Blockchain 버전
	std::uint8_t previousBlockHash[SHA256_DIGEST_VALUELEN];	// 이전 블록 해시
	std::uint8_t merkleRoot[SHA256_DIGEST_VALUELEN];		// 개별 transaction 해시로 만든 머클트리의 머클루트
	time_t timestamp;										// 해당 블록의 채굴 시작 시간
	std::uint8_t bits = 19;									// 2진수 기준 blockHash 앞에 나와야 할 0의 개수
	std::uint64_t nonce;									// 임의 대입 수

	/* Transactions */
	std::vector<Transaction> transactions;					// Transaction 내역

	/* Additional Information */
	std::uint8_t blockHash[SHA256_DIGEST_VALUELEN];			// Block 해시값
	std::uint64_t height;									// Block의 높이. 해싱 안 함.
	const Block * previousBlock;							// 이전 블록의 메모리 주소. 해싱 안 함.

	const Block * nextBlock;								// 다음 블록의 메모리 주소. 해싱 안 함.
	bool isMainChain;										// main chain 블록인지. 해싱 안 함.

	// Blockchain class에서 사용하는 메소드
	inline bool isFull() const;
	void mining();
	void initializeMerkleRoot();
	void printBlockHeader(std::ostream & o) const;

	// serialization
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		//ar & height;
		ar & version;
		ar & previousBlockHash;
		ar & merkleRoot;
		ar & timestamp;
		ar & bits;
		ar & nonce;
		ar & transactions;
	}

	Block();
	Block(const Block * _previousBlock);
	void print(std::ostream & o) const;
	bool isValid() const;									// 블록 유효성 검사
	bool setAdditionalInfo();
	inline int getBlockHeaderSize() const;

private:
	std::uint8_t * createBlockHeader() const;
	const std::uint8_t * createMerkleRoot() const;
	bool miningSuccess() const;
};

inline int Block::getBlockHeaderSize() const {
	return sizeof(version) + sizeof(previousBlockHash) + sizeof(merkleRoot) 
		+ sizeof(timestamp) + sizeof(bits) + sizeof(nonce);	// 6가지
}

inline bool Block::isFull() const {
	return transactions.size() >= MAX_TRANSACTION_COUNT ? true : false;
}

#endif // !BLOCK_H



