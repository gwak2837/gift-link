#pragma once
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <iostream>
#include <cstdint>
#include <list>
#include <ctime>
#include <vector>
#include <string>
#include <map>
#include "KISA_SHA256.h"
#include "Utility.h"
#include "Transaction.h"
#include "Block.h"
#include "Wallet.h"

enum class CoinbaseType {
	normalCoinbase, administratorCoinbase
};

class Blockchain {
	const Block * genesisBlock;					// 첫번째 블록
	Block * lastBlock;							// 마지막 블록
	Block * waitingBlock;						// 채굴을 기다리는 블록
	std::list<Transaction> txPool;				// 검증을 기다리는 거래(아직 검증되지 않은 블록)
	std::list<Transaction> orphanTxPool;		// 고아 거래 풀
	std::list<Block *> lastBlocks;				// 여러 분기
	size_t blockCount;							// 블록의 총 개수
	std::string name;							// 블록체인 이름

	inline void addBlock(Block * _block);

public:
	int version;								// 블록체인 현재 버전

	Blockchain(std::string _name, const std::uint8_t * _recipientPublicKeyHash);

	bool addTransactionToPool(Transaction & _tx);
	bool produceBlock(const std::uint8_t * _recipientPublicKeyHash, int txCount, const State feeState = State::own);
	bool issueSecurities(const std::uint8_t * _recipientPublicKeyHash, int txCount, Type & type, int issueAmount, const State securitiesState, const State feeState = State::own);

	bool findPreviousTx(Transaction & previousTx, const std::uint8_t * previousTxHash, std::uint64_t blockHeight) const;
	bool isTxOutputReferenceCount(const Transaction & tx, size_t txOutputIndex, std::uint64_t referenceCount) const;

	bool findUTXOTable(std::vector<UTXO> & UTXOTable) const;
	bool findUTXOTable(std::vector<UTXO> & UTXOTable, const State state) const;
	bool findMyUTXOTable(std::vector<UTXO> & myUTXOTable, const std::uint8_t * _recipientPublicKeyHash) const;	// -> test 필요함
	bool findMyUTXOTable(std::vector<UTXO> & myUTXOTable, const std::uint8_t * _recipientPublicKeyHash, const State state) const;	// -> test 필요함

	bool getBlockchain();								// -> 개발 중	// isValid와 비슷
	bool setHeightAndMainChain();						// -> 개발 중
	bool calculateTotalTransactionFee(const Block * block, std::map<Type, int64_t> & mapTypeValue) const;
	bool calculateTotalTransactionFee(const Transaction & tx, std::map<Type, int64_t> & mapTypeValue) const;

	bool isValidCoinbase(const Block * block, const Transaction & coinbaseTx, CoinbaseType coinbaseType) const;
	bool isValid(const Transaction & tx, int previousOutputReferenceCount) const;	// 블록체인 단위 거래 유효성 검사
	bool isValid() const;															// 블록체인 유효성 검사

	
	// getter method
	std::string getFileName() const;
	inline const Block * getGenesisBlock() const { return genesisBlock; }
	inline const Block * getLastBlock() const { return lastBlock; }
	inline size_t getBlockCount() const { return blockCount; }
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

/* findPreviousTx()의 반환형이 참조자인 이유:
원래 있던 Tx의 위치를 반환하니까 */

#endif // !BLOCKCHAIN_H




