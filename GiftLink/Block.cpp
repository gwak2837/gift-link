#include <iostream>
#include <vector>
#include <ctime>
#include <queue>
#include <cstring>
#include <cassert>
#include <cstdint>
#include "Block.h"
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
	const int i = sizeof(version) + sizeof(previousBlockHash) + sizeof(merkleRoot) + sizeof(timestamp) + sizeof(bits);

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

bool Block::isValid() const {
	if (transactions.size() == 0) {						// 블록에 거래가 하나도 없으면
		cout << "There is no transaction in " << height << "th block...\n";
		return false;
	}

	//if (!isValidBits())
	//	return false;

	size_t coinbaseTxCount = 0;
	size_t txCount = 0;
	Transaction coinbaseTx;
	for (const Transaction & tx : transactions) {
		if (tx.isCoinbase()) {
			coinbaseTx = tx;
			coinbaseTxCount++;
			if (coinbaseTxCount > 1) {					// 블록에 코인베이스 거래가 1개를 초과하면
				cout << "There are coinbase transactions more than one in " << height << "th block...\n";
				return false;
			}
		}
		txCount++;
		if (txCount > MAX_TRANSACTION_COUNT) {
			cout << "There are transaction more than max tx count in " << height << "th block...\n";
			return false;
		}
	}
		
	if (previousBlock != NULL) {		
		if(previousBlock->timestamp >= timestamp) {		// 이전 블록의 timestamp 비교
			cout << height << "th block timestamp is invalid...\n";
			return false;
		}
		if (!isMemoryEqual(previousBlock->blockHash, previousBlockHash, sizeof(previousBlockHash))) {	// previousBlockHash가 다르면
			cout << height << "th block previousBlockHash is invalid...\n";
				return false;
		}

		Transaction previousBlockCoinbaseTx;
		for (const Transaction & tx : previousBlock->transactions) {
			if (tx.isCoinbase())
				previousBlockCoinbaseTx = tx;
		}
		
		assert(coinbaseTx.outputs.size() > 0 && previousBlockCoinbaseTx.outputs.size() > 0);
		//if (isMemoryEqual(coinbaseTx.outputs[0].recipientPublicKeyHash, previousBlockCoinbaseTx.outputs[0].recipientPublicKeyHash, sizeof(coinbaseTx.outputs[0].recipientPublicKeyHash)))		// 연속 2회 이상 동일 주소로 블록 생성 시
		//	return false;																																											// 테스트를 위해 비활성화
	}
	
	const uint8_t * _merkleRoot = createMerkleRoot();

	if (!isMemoryEqual(merkleRoot, _merkleRoot, sizeof(merkleRoot))) {		// Merkle Root 계산이 틀리면
		cout << height << "th block merkleRoot is invalid...\n";				// 원인: 머클트리 잘못 계산, Transaction 내용 변경
		delete[] _merkleRoot;
		return false;
	}
	delete[] _merkleRoot;

	uint8_t _blockHash[SHA256_DIGEST_VALUELEN];
	const uint8_t * blockHeader = createBlockHeader();
	SHA256_Encrpyt(blockHeader, getBlockHeaderSize(), _blockHash);
	delete[] blockHeader;

	if (!isMemoryEqual(blockHash, _blockHash, sizeof(blockHash))) {			// Block Hash 값이 틀리면
		cout << "\n" << height << "th block blockHash is invalid...\n";
		return false;
	}

	if (!miningSuccess()) {													// Block Hash가 bit와 다르면
		cout << "\n" << height << "th block mining is invalid...\n";
		return false;
	}

	return true;
}

bool Block::setAdditionalInfo() {
	uint8_t * blockHeader = createBlockHeader();
	SHA256_Encrpyt(blockHeader, getBlockHeaderSize(), blockHash);

	//*****************************************거래 해시값 계산

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
	printBlockHeader(o);

	int j = 0;
	for (const Transaction & tx : transactions) {
		o << "<-- Transaction #" << j << " -->\n";
		tx.print(o);
		j++;
	}
}

void Block::printBlockHeader(ostream & o) const {
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
		if (txHashes.size() % 2 != 0) {										// Tx 개수가 홀수이면 제일 마지막 Tx를 복사해서 짝수로 만든다.
			uint8_t * txHash = new uint8_t[SHA256_DIGEST_VALUELEN];			// 깊은 복사(얕은 복사 시 오류 발생)
			memcpy(txHash, txHashes.back(), SHA256_DIGEST_VALUELEN);
			txHashes.push_back(txHash);
		}

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

