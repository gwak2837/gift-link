#include <iostream>
#include <vector>
#include <ctime>
#include <queue>
#include <cstring>
#include <cassert>
#include <cstdint>
#include "Block.h"
#include "Transaction.h"
#include "Utility.h"
using namespace std;

Block::Block() : Block(NULL) {}

Block::Block(const Block * _previousBlock) : previousBlock(_previousBlock) {
	assert(0 < bits && bits < SHA256_DIGEST_VALUELEN * 8); // 2진수 기준 난이도 범위

	if (previousBlock != NULL)
		memcpy(previousBlockHash, previousBlock->blockHash, sizeof(previousBlockHash));
	else
		memset(previousBlockHash, 0, sizeof(previousBlockHash));
}

/* 2진수 기준 blockHash 앞에 0이 bits개 이상 있으면 true, 그 외 false. */
bool Block::miningSuccess() const {
	int byte = bits / 8;
	int remainBits = bits - 8 * byte;

	int i;
	for (i = 0; i < byte; i++) {
		if (blockHash[i] != 0)
			return false;
	}

	if ((uint8_t)blockHash[i] > pow(2, 8 - remainBits) - 1)
		return false;
	
	return true;
}

void Block::mining() {
	int i = sizeof(version) + sizeof(previousBlockHash) + sizeof(merkleRoot) + sizeof(timestamp) + sizeof(bits);

MINING:
	nonce = 0;
	timestamp = time(NULL);

	uint8_t * blockHeader = createBlockHeader();
	SHA256_Encrpyt(blockHeader, getBlockHeaderSize(), blockHash);

	while (!miningSuccess()) {
		//if(nonce % 0x200000 == 0)	// 채굴 진행 중 표시
		//	cout << ".\n";
		if (nonce == UINT64_MAX)	// nonce 오버플로우 시 timestamp 초기화
			goto MINING;

		nonce++;
		memcpy(blockHeader + i, &nonce, sizeof(nonce));
		SHA256_Encrpyt(blockHeader, getBlockHeaderSize(), blockHash);
	}

	isMainChain = true;
	if (previousBlock != NULL)
		height = previousBlock->height + 1;
	else
		height = 0;

	delete[] blockHeader;
}

/* 반환된 포인터는 나중에 delete[]로 할당 해제해야 함. */
uint8_t * Block::createBlockHeader() const {
	int i = 0;			// transactionData index
	uint8_t * blockHeader = new uint8_t[getBlockHeaderSize()];

	memcpy(blockHeader + i, &version, sizeof(version));
	i += sizeof(version);

	memcpy(blockHeader + i, previousBlockHash, sizeof(previousBlockHash));
	i += sizeof(previousBlockHash);

	memcpy(blockHeader + i, merkleRoot, sizeof(merkleRoot));
	i += sizeof(merkleRoot);

	memcpy(blockHeader + i, &bits, sizeof(bits));
	i += sizeof(bits);

	memcpy(blockHeader + i, &timestamp, sizeof(timestamp));
	i += sizeof(timestamp);

	memcpy(blockHeader + i, &nonce, sizeof(nonce));
	i += sizeof(nonce);

	assert(i == getBlockHeaderSize());

	return blockHeader;
}

/* 보완점? */
bool Block::isValid() const {
	if (previousBlock != NULL) {		
		if(previousBlock->timestamp - VALID_TIMESTAMP_GAP > timestamp) {	// 블록의 timestamp 간격이 적절한지
			cout << "\n" << height << "th block timestamp is unvalid...\n";
			return false;
		}
		if (!isMemoryEqual(previousBlock->blockHash, previousBlockHash, sizeof(previousBlockHash))) {	// previousBlockHash가 유효한지
			cout << "\n" << height << "th block previousBlockHash is unvalid...\n";
				return false;
		}
	}
	
	const uint8_t * _merkleRoot = createMerkleRoot();

	if (_merkleRoot == NULL) {		// 블록에 거래가 포함되어 있는지
		cout << "\nThere is no transaction in " << height << "th block...\n";
		delete[] _merkleRoot;
		return false;
	}

	if (!isMemoryEqual(merkleRoot, _merkleRoot, sizeof(merkleRoot))) {		// Transaction 해싱 결과 merkleRoot와 일치하는지
		cout << "\n" << height << "th block merkleRoot is unvalid...\n";	// 원인: 머클트리 잘못 계산, Transaction 내용 변경
		delete[] _merkleRoot;
		return false;
	}
	delete[] merkleRoot;

	uint8_t _blockHash[SHA256_DIGEST_VALUELEN];
	const uint8_t * blockHeader = createBlockHeader();
	SHA256_Encrpyt(blockHeader, getBlockHeaderSize(), _blockHash);
	delete[] blockHeader;

	if (!isMemoryEqual(_blockHash, blockHash, SHA256_DIGEST_VALUELEN)) {	// Block Header 해싱 결과 blockHashd와 일치하는지
		cout << "\n" << height << "th block blockHash is unvalid...\n";
		return false;
	}

	if (!miningSuccess()) {			// blockHash가 목표값에 도달했는지
		cout << "\n" << height << "th block mining is unvalid...\n";
		return false;
	}

	return true;
}

void Block::initializeMerkleRoot() {
	assert(transactions.size() > 0);

	const uint8_t * _merkleRoot = createMerkleRoot();

	memcpy(merkleRoot, _merkleRoot, sizeof(merkleRoot));		// Merkle tree의 root node를 merkleRoot에 복사한다.
	delete[] _merkleRoot;
}

void Block::print(ostream & o) const {
	o << "Block #" << height << '\n';
	o << "Block Hash:  " << blockHash << '\n';
	o << "Version:     " << version << '\n';
	o << "Previous \nBlock Hash:  " << previousBlockHash << '\n';
	o << "Merkle Hash: " << merkleRoot << '\n';
	o << "Timestamp:   " << timeToString(timestamp) << '\n';
	o << "Bits:        " << (int)bits << '\n';
	o << "Nonce:       " << nonce << "\n\n";

	int j = 0;
	for (const Transaction & tx : transactions) {
		o << "Transaction #" << j << '\n';
		tx.print(o);
		j++;
	}
}

void Block::printBlockHeader(ostream & o) const {
	o << "Block #" << height << '\n';
	o << "Block Hash:  " << blockHash << '\n';
	o << "Version:     " << version << '\n';
	o << "Previous \nBlock Hash:  " << previousBlockHash << '\n';
	o << "Merkle Hash: " << merkleRoot << '\n';
	o << "Timestamp:   " << timeToString(timestamp) << '\n';
	o << "Bits:        " << (int)bits << '\n';
	o << "Nonce:       " << nonce << "\n\n";
}


/* 반환된 포인터는 나중에 delete[]로 할당 해제해야 함.
블록 안에 Tx가 하나도 없는 경우 NULL을 반환함. */
const uint8_t * Block::createMerkleRoot() const {
	if (transactions.size() == 0)		// 블록 안에 Tx가 하나도 없는 경우
		return NULL;

	vector<uint8_t *> txHashes;
	txHashes.reserve(MAX_TRANSACTION_COUNT);

	for (const Transaction & tx : transactions) {
		uint8_t * txHash = new uint8_t[SHA256_DIGEST_VALUELEN];
		memcpy(txHash, tx.txHash, SHA256_DIGEST_VALUELEN);
		txHashes.push_back(txHash);
	}

	while (txHashes.size() > 1) {
		if (txHashes.size() % 2 != 0)				// Tx 개수가 홀수이면 제일 마지막 Tx를 복사해서 짝수로 만든다.
			txHashes.push_back(txHashes.back());

		assert(txHashes.size() % 2 == 0);

		vector<uint8_t *> newTxHashes;
		newTxHashes.reserve(txHashes.size() / 2);
		
		for (auto iterator = txHashes.cbegin(); iterator != txHashes.cend(); iterator += 2) {	// const_iterator
			uint8_t hashIn[SHA256_DIGEST_VALUELEN * 2];
			memcpy(hashIn, *iterator, SHA256_DIGEST_VALUELEN);									// Tx Hash 첫 번째
			memcpy(hashIn + SHA256_DIGEST_VALUELEN, *(iterator + 1), SHA256_DIGEST_VALUELEN);	// Tx Hash 두 번째를 연결한다.
			delete[] *iterator;
			delete[] *(iterator + 1);

			uint8_t * hashOut = new uint8_t[SHA256_DIGEST_VALUELEN];		// 연결된 2개의 해시를 해싱한다.
			SHA256_Encrpyt(hashIn, SHA256_DIGEST_VALUELEN * 2, hashOut);

			newTxHashes.push_back(hashOut);									// 머클트리 형식으로 계속 해싱한다.
		}
		txHashes.swap(newTxHashes);
	}

	return txHashes[0];
}









