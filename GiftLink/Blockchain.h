#pragma once
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <iostream>
#include <cstdint>
#include <list>
#include <map>
#include <ctime>
#include <vector>
#include <string>
#include "KISA_SHA256.h"
#include "Utility.h"
#include "Transaction.h"
#include "Block.h"
#include "Wallet.h"

enum class CoinbaseType {
	normalCoinbase, administratorCoinbase
};

class Blockchain {
public:
	const Block * genesisBlock;					// 첫번째 블록
	Block * lastBlock;							// 주체인의 마지막 블록
	Block * waitingBlock;						// 채굴을 기다리는 블록
	std::list<Transaction> txPool;				// 검증을 기다리는 거래(아직 검증되지 않은 블록)
	std::list<Transaction> orphanTxPool;		// 고아 거래 풀
	std::list<Block *> lastBlocks;				// 여러 분기
	size_t blockCount;							// 블록의 총 개수
	std::string name;							// 블록체인 이름
	int version;								// 블록체인 현재 버전

	Blockchain() {};
	Blockchain(std::string _name, const std::uint8_t * _recipientPublicKeyHash);

	bool addTransactionToPool(Transaction & _tx);
	bool produceBlock(const std::uint8_t * recipientPublicKeyHash, int txCount, const State feeState = State::OWN);
	bool issueSecurities(const std::uint8_t * recipientPublicKeyHash, int txCount, Type type, int64_t issueAmount, const State securitiesState, const State feeState = State::OWN);
	void broadcastBlock() const;

	bool findUTXOTable(std::vector<UTXO> & UTXOTable) const;
	bool findUTXOTable(std::vector<UTXO> & UTXOTable, const State state) const;
	bool findMyUTXOTable(std::vector<UTXO> & myUTXOTable, const std::uint8_t * _recipientPublicKeyHash) const;	// -> test 필요함
	bool findMyUTXOTable(std::vector<UTXO> & myUTXOTable, const std::uint8_t * _recipientPublicKeyHash, const State state) const;	// -> test 필요함

	bool getBlockchain();								// -> 개발 중	// isValid와 비슷
	bool setHeightAndMainChain();						// -> 개발 중

	bool isValidCoinbase(const Block * block, const Transaction & coinbaseTx, CoinbaseType coinbaseType) const;
	bool isValid(const Transaction & tx, int previousOutputReferenceCount) const;	// 블록체인 단위 거래 유효성 검사
	bool isValid() const;															// 블록체인 유효성 검사
	bool isAdministratorAddress(const std::uint8_t * recipientPublicKeyHash) const;
	
	// getter method
	std::string getFileName() const;
	inline const Block * getGenesisBlock() const { return genesisBlock; }
	inline const Block * getLastBlock() const { return lastBlock; }
	inline size_t getBlockCount() const { return blockCount; }
	inline std::string getName() const { return name; }
	inline std::int64_t getMaxIssueAmount(const Type & type) const { return COINBASE_REWARD * 100 / type.faceValue; }

	//for debug
	void print(std::ostream & o) const;
	void printAllBlockHash() const;
	void printAllMerkleHash() const;
	void printAllTransaction(std::ostream & o) const;
	void printWaitingBlock() const;						// -> 개발 중


	// tx 내용 검색 메소드								// -> 개발 중

private:
	bool addBlock(Transaction & coinbaseTx);
	bool findPreviousTx(Transaction & previousTx, const std::uint8_t * previousTxHash, std::uint64_t blockHeight) const;
	bool isTxOutputReferenceCount(const Transaction & tx, size_t txOutputIndex, std::uint64_t referenceCount) const;
	bool calculateTotalTransactionFee(const Block * block, std::map<Type, int64_t> & mapTypeValue) const;
	bool calculateTotalTransactionFee(const Transaction & tx, std::map<Type, int64_t> & mapTypeValue) const;
	bool getTxType(TxType & txType, const Transaction & tx) const;
};


/* findPreviousTx()의 반환형이 참조자인 이유:
원래 있던 Tx의 위치를 반환하니까 */

#endif // !BLOCKCHAIN_H




