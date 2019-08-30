#pragma once
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <iostream>
#include <cstdint>
#include <queue>
#include <ctime>
#include <vector>
#include <string>
#include "KISA_SHA256.h"
#include "Utility.h"
#include "Transaction.h"
#include "Block.h"
#include "Wallet.h"

class Blockchain {
	const Block * genesisBlock;					// 첫번째 블록
	const Block * lastBlock;					// 마지막 블록
	Block * waitingBlock;						// 채굴을 기다리는 블록
	std::queue<Transaction> txPool;				// 검증을 기다리는 거래(아직 검증되지 않은 블록)
	std::uint64_t blockCount;					// 블록의 총 개수
	std::string name;							// 블록체인 이름

	inline void addBlock(Block * _block);

public:
	int version;								// 블록체인 현재 버전

	Blockchain(std::string _name, const std::uint8_t * _recipientPublicKeyHash);

	void addTransactionToPool(Transaction & _tx);
	bool produceBlock(const std::uint8_t * _recipientPublicKeyHash);

	Output & findPreviousOutput(std::uint64_t blockHeight, const std::uint8_t * previousTxHash, int outputIndex) const;
	bool isUTXO(Output & output) const;
	std::vector<Output *> getUTXOTable() const;
	std::vector<Output *> getMyUTXOTable(const std::uint8_t * _recipientPublicKeyHash) const;	// -> test 필요함
	//std::vector<Output *> getIssuableGiftcardTable(const std::uint8_t * privateKey) const;

	bool loadBlockchain();								// -> 개발 중	// isValid와 비슷
	bool setHeightAndMainChain();

	bool isValid(const Transaction & tx) const;
	bool isValid() const;

	// getter method
	std::string getFileName() const;
	inline const Block * getGenesisBlock() const { return genesisBlock; }
	inline const Block * getLastBlock() const { return lastBlock; }
	inline std::uint64_t getBlockCount() const { return blockCount; }
	inline std::string getName() const { return name; }

	//for debug
	void print(std::ostream & o) const;
	void printAllBlockHash() const;
	void printAllMerkleHash() const;
	void printAllTransaction(std::ostream & o) const;
	void printWaitingBlock() const;						// -> 개발 중


	// tx 내용 검색 메소드								// -> 개발 중


};

// waiting block을 블록체인에 연결한다.
inline void Blockchain::addBlock(Block * _block) {
	lastBlock = _block;
	blockCount++;
}

/* findPreviousOutput()의 반환형이 참조자인 이유:
원래 있던 Output의 위치를 반환하니까 */

#endif // !BLOCKCHAIN_H




