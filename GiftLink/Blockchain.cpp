#define _CRT_SECURE_NO_WARNINGS	/* localtime() */
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdint>
#include <cassert>
#include <map>
#include <algorithm>
#include <future>
#include "Blockchain.h"
#include "Block.h"
#include "Transaction.h"
#include "Wallet.h"
#include "Utility.h"
#include "BlockBroadcaster.h"
using namespace std;

// Genesis block을 생성한다.
Blockchain::Blockchain(string _name, const uint8_t * _recipientPublicKeyHash) : blockCount(0), name(_name), version(1) {
	cout << "Creating a blockchain...\n";

	Block * _genesisBlock = new Block();
	genesisBlock = _genesisBlock;
	waitingBlock = _genesisBlock;

	vector<Input> inputs;				
	Input coinbaseInput;
	inputs.push_back(coinbaseInput);

	vector<Output> outputs;
	Output output(_recipientPublicKeyHash, COINBASE_REWARD, Type(), State::OWN);	// Coinbase output
	outputs.push_back(output);

	Transaction coinbaseTx(inputs, outputs, version, "GenesisBlock-CoinbaseTransaction");

	if (!addBlock(coinbaseTx))
		cout << "There is an error when creating genesis block...\n";
}

// waiting block을 블록체인에 연결한다.
bool Blockchain::addBlock(Transaction & coinbaseTx) {
	waitingBlock->version = version;		// 현재 blockchain version 입력
	waitingBlock->transactions.insert(waitingBlock->transactions.begin(), coinbaseTx);
	waitingBlock->initializeMerkleRoot();	// Transaction hash로 Merkletree를 만들어 waiting block의 merkleroot 계산
	waitingBlock->mining();					// waiting block을 채굴
	lastBlock = waitingBlock;
	blockCount++;
	waitingBlock->height = blockCount - 1;
	waitingBlock->isMainChain = true;

	Block * newWaitingBlock = new Block(lastBlock);
	waitingBlock = newWaitingBlock;

	return true;
}

/* transaction pool에 transaction을 추가한다. 
참조하는 Output이 없으면 일단 고아 거래 풀에 담김. */
bool Blockchain::addTransactionToPool(Transaction & tx) {
	for (const Input & input : tx.inputs) {
		Transaction previousTx;
		if (!findPreviousTx(previousTx, input.previousTxHash, input.blockHeight)) {		// 이전 Output이 없으면
			orphanTxPool.push_back(tx);
			return false;
		}
	}

	if (!isValid(tx, 0))				// 거래의 내용이 유효한지
		return false;

	txPool.push_back(tx);						
	return true;
}


/* transaction pool에서 transaction을 가져와 채굴한다.
성공적으로 블록을 생성하면 true, 아니면 false를 반환한다.

state : 수수료 보상을 소유할 지 팔 지 선택

Coinbase Transaction 생성 과정
1. Input 설정
2. Output 설정
3. Transaction 데이터 해싱 */
bool Blockchain::produceBlock(const uint8_t * recipientPublicKeyHash, int txCount, State feeState) {
	if (txPool.size() == 0)
		cout << "There aren't enough transactions in the transaction pool to produce block...\n";

	assert(waitingBlock != NULL);

	if (txCount > MAX_TRANSACTION_COUNT)
		txCount = MAX_TRANSACTION_COUNT;

	auto iter = txPool.begin();
	for (size_t count = 0; count < MAX_TRANSACTION_COUNT - 1 && iter != txPool.end(); count++) {
		if (isValid(txPool.front(), 0))
			waitingBlock->transactions.push_back(*iter);
		else
			cout << "produceBlock : Invalid Transaction in Tx Pool...\n";

		iter = txPool.erase(iter);
	}

	vector<Input> inputs;
	Input coinbaseInput;
	inputs.push_back(coinbaseInput);


	vector<Output> outputs;
	Output blockProductionReward(recipientPublicKeyHash, COINBASE_REWARD, Type(), State::OWN);
	outputs.push_back(blockProductionReward);

	map<Type, int64_t> mapTypeValue;
	if (!calculateTotalTransactionFee(waitingBlock, mapTypeValue))
		return false;

	// Type-Value별로 Output 생성
	for (const auto & kv : mapTypeValue) {
		if (kv.second > 0) {
			Output txFeeReward(recipientPublicKeyHash, kv.second, kv.first, feeState);
			outputs.push_back(txFeeReward);
		}
	}

	Transaction coinbaseTx(inputs, outputs, version, "Coinbase Transaction");
	if (!addBlock(coinbaseTx))
		return false;

	cout << "Block was produced successfully!\n";
	broadcastBlock();
	return true;
}

bool Blockchain::issueSecurities(const uint8_t * recipientPublicKeyHash, int txCount, Type type, int64_t issueAmount, const State securitiesState, const State feeState) {
	if (txPool.size() == 0)
		cout << "There aren't enough transactions in the transaction pool to produce block...\n";

	assert(waitingBlock != NULL);

	if (txCount > MAX_TRANSACTION_COUNT)
		txCount = MAX_TRANSACTION_COUNT;

	if (issueAmount > getMaxIssueAmount(type))
		issueAmount = getMaxIssueAmount(type);

	if (issueAmount == 0) {
		cout << "Couldn't issue securities...\n";
		return false;
	}

	auto iter = txPool.begin();
	for (size_t count = 0; count < txCount - 1 && iter != txPool.end(); count++) {
		if (isValid(txPool.front(), 0))						// tx가 Invalid하면 delete(메모리 해제)하고 큐에서 뺴기로
			waitingBlock->transactions.push_back(*iter);
		else
			cout << "issueSecurities : Invalid Transaction in Tx Pool...\n";

		iter = txPool.erase(iter);
	}

	vector<Input> inputs;
	Input coinbaseInput;
	inputs.push_back(coinbaseInput);

	vector<Output> outputs;
	Output blockProductionReward(recipientPublicKeyHash, issueAmount, type, securitiesState);
	outputs.push_back(blockProductionReward);

	map<Type, int64_t> mapTypeValue;
	if (!calculateTotalTransactionFee(waitingBlock, mapTypeValue))
		return false;

	// Type-Value별로 Output 생성
	for (const auto & kv : mapTypeValue) {
		if (kv.second > 0) {
			Output txFeeReward(recipientPublicKeyHash, kv.second, kv.first, feeState);
			outputs.push_back(txFeeReward);
		}
	}

	Transaction coinbaseTx(inputs, outputs, version, "Coinbase Transaction");
	if (!addBlock(coinbaseTx))
		return false;

	cout << "Securities were issued successfully!\n";
	broadcastBlock();
	return true;
}

// 수수료 보상 계산
bool Blockchain::calculateTotalTransactionFee(const Block * block, map<Type, int64_t> & mapTypeValue) const {
	for (const Transaction & tx : block->transactions) {
		if (!tx.isCoinbase()) {
			if (!calculateTotalTransactionFee(tx, mapTypeValue))
				return false;
		}
	}

	return true;
}

bool Blockchain::calculateTotalTransactionFee(const Transaction & tx, map<Type, int64_t> & mapTypeValue) const {
	if(tx.isCoinbase())
		return false;

	for (const Input & input : tx.inputs) {
		Transaction previousTx;
		if (!findPreviousTx(previousTx, input.previousTxHash, input.blockHeight))
			return false;

		const Output & previousOutput = previousTx.outputs[input.outputIndex];
		auto iter = mapTypeValue.find(previousOutput.type);		// 이전 Output의 Type별 Value 합산 후 저장
		if (iter != mapTypeValue.end())
			iter->second += previousOutput.value;
		else
			mapTypeValue.insert(pair<Type, int64_t>(previousOutput.type, previousOutput.value));
	}

	for (const Output & output : tx.outputs) {
		auto iter = mapTypeValue.find(output.type);
		if (iter != mapTypeValue.end()) {
			iter->second -= output.value;
			if (iter->second < 0) {		// Input의 Value보다 Output의 Value가 더 크면
				cout << "Total input value in waiting block < Total output value in waiting block...\n";
				return false;
			}
		}
		else {	// Output의 Type이 Input에 없으면
			cout << "There is new type in output which is not in input...\n";
			return false;
		}
	}

	return true;
}

bool Blockchain::getTxType(TxType & txType, const Transaction & tx) const {
	








	for (const Input & input : tx.inputs) {
		Transaction previousTx;
		if (!findPreviousTx(previousTx, input.previousTxHash, input.blockHeight))		// 이전 Output이 없으면
			return false;


	}

	//for (const Output & output : tx.outputs) {
	//	if(output.)

	//}




	//if (previousTx.outputs[input.outputIndex].state == State::SALE) {
	//	txType = TxType::PURCHASE;
	//}
	//else if (previousTx.outputs[input.outputIndex].state == State::OWN)





	return false;
}

void Blockchain::broadcastBlock() const {
	BlockBroadcaster bb;
	future<void> f2 = async(launch::async, &BlockBroadcaster::broadcast, ref(bb), getLastBlock(), "localhost", "8000");
	cout << "Broadcasting\n";
}

bool Blockchain::isValidCoinbase(const Block * block, const Transaction & coinbaseTx, CoinbaseType coinbaseType) const {
	if (!coinbaseTx.isCoinbase())
		return false;

	if (coinbaseTx.outputs[0].type != Type()) {															// 유가증권 발행 시
		if (coinbaseTx.outputs[0].value > getMaxIssueAmount(coinbaseTx.outputs[0].type))					// 유가증권 최대 발행량 <= 블록 생성 보상 * 100
			return false;
	}
	else if (coinbaseTx.outputs[0].type == Type() && coinbaseTx.outputs[0].state == State::OWN) {		// 블록 생성 시
		if (coinbaseTx.outputs[0].value != COINBASE_REWARD)													// 블록 생성 보상
			return false;
	}
	else if (coinbaseType == CoinbaseType::administratorCoinbase) {										// 관리자 블록의 코인베이스이면
		if (coinbaseTx.outputs[0].value != 0)																// 블록 생성 보상 없음
			return false;

		for (const Output & output : coinbaseTx.outputs) {												// 거래의 모든 output의 주소가				
			if (!isAdministratorAddress(output.recipientPublicKeyHash))										// 관리자인지
				return false;
		}
	}
	else																								// 블록 생성인데 state가 이상한 것
		return false;

	uint8_t blockProducerPublicKeyHash[SHA256_DIGEST_VALUELEN];
	memcpy(blockProducerPublicKeyHash, coinbaseTx.outputs[0].recipientPublicKeyHash, SHA256_DIGEST_VALUELEN);
	for (const Output & output : coinbaseTx.outputs) {
		if (!isMemoryEqual(blockProducerPublicKeyHash, output.recipientPublicKeyHash, SHA256_DIGEST_VALUELEN))		// 블록 생성자가 여러 명이면
			return false;
	}

	map<Type, int64_t> mapTypeValue;
	if (!calculateTotalTransactionFee(block, mapTypeValue))
		return false;

	Transaction copyCoinbaseTx = coinbaseTx;	
	copyCoinbaseTx.outputs.erase(copyCoinbaseTx.outputs.begin());										// 블록 생성 보상(outputs[0]) 제거

	for (const auto & kv : mapTypeValue) {																// 수수료 보상 검증
		if (kv.second > 0) {
			Output output(blockProducerPublicKeyHash, kv.second, kv.first, State::OWN);
			auto iter = find(copyCoinbaseTx.outputs.begin(), copyCoinbaseTx.outputs.end(), output);		// state까지 같을 필요는 없음. operator==에 정의됨
			if ((iter != copyCoinbaseTx.outputs.end())) {													// 수수료 보상 있으면
				copyCoinbaseTx.outputs.erase(iter);
			}
			else
				return false;
		}
	}

	if (copyCoinbaseTx.outputs.size() != 0)		// 이 외에도 수수료 보상이 더 있으면
		return false;

	return true;
}



bool Blockchain::setHeightAndMainChain() {
	for (Block * lastBlock : lastBlocks) {
		







	}

	return false;
}

/* 거래 내역 유효성 검사. coinbase transaction은 항상 false 반환 */
bool Blockchain::isValid(const Transaction & tx, int previousOutputReferenceCount) const {
	if (tx.isCoinbase())	// coinbase는 검사하지 않는다.
		return false;

	Transaction testTx = tx;			// txHash 유효성	
	size_t inputIndex = 0;

	for (const Input & input : tx.inputs) {
		Transaction previousTx;
		if (!findPreviousTx(previousTx, input.previousTxHash, input.blockHeight))		// 이전 Output이 없으면
			return false;

		if (!isTxOutputReferenceCount(previousTx, input.outputIndex, previousOutputReferenceCount))	// 이전 Output 참조 횟수 검사
			return false;

		if (previousTx.outputs[input.outputIndex].state == State::SPENT)				// 사용된 유가증권을 참조하고 있으면
			return false;

		if (previousTx.timestamp >= tx.timestamp)										// 시간 간격
			return false;

		
		
		const Output & previousOutput = previousTx.outputs[input.outputIndex];
		memset(testTx.inputs[inputIndex].signature, 0, sizeof(testTx.inputs[inputIndex].signature));
		memcpy(testTx.inputs[inputIndex].signature, previousOutput.recipientPublicKeyHash, sizeof(previousOutput.recipientPublicKeyHash));

		uint8_t senderPublicKeyHash[SHA256_DIGEST_VALUELEN];
		SHA256_Encrpyt(input.senderPublicKey, sizeof(input.senderPublicKey), senderPublicKeyHash);
		if (!isMemoryEqual(previousOutput.recipientPublicKeyHash, senderPublicKeyHash, sizeof(senderPublicKeyHash)))	// SHA256(input.senderPubKey) == 이전 Output의 pubKeyHash
			return false;

		inputIndex++;
	}



	//TxType txType;
	//if (!getTxType(txType, tx))
	//	return false;

	//if (txType != TxType::PURCHASE) {	// 거래 서명 유효성(txHash + 공개키 + 서명)
	//	if (!tx.isValid())
	//		return false;
	//}
	//else {
	//	
	//}



	


	//switch (txType)
	//{
	//case TxType::USE:
	//	break;
	//case TxType::SEND:
	//	break;
	//case TxType::SALE:
	//	break;
	//case TxType::PURCHASE:
	//	break;
	//default:
	//	break;
	//}




	const uint8_t * testTxData = testTx.createTxData();
	SHA256_Encrpyt(testTxData, (unsigned int)testTx.getTxDataSize(), testTx.txHash);
	delete[] testTxData;

	if (!isMemoryEqual(testTx.txHash, tx.txHash, sizeof(testTx.txHash)))		// txHash 계산 과정 유효성
		return false;

	map<Type, int64_t> mapTypeValue;
	if (!calculateTotalTransactionFee(tx, mapTypeValue))	// input type별 value >= output type별 value 유효성
		return false;

	return true;
}

/* 개발 중 : Coinbase 거래의 수수료 합산이 유효한지 */
bool Blockchain::isValid() const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return false;
	}

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		for (const Transaction & tx : presentBlock->transactions) {		// 모든 거래(Coinbase + 일반)가 유효한
			if (tx.isCoinbase()) {
				if ((blockCount - i - 1) % 30 == 0 && presentBlock->previousBlock != NULL && presentBlock->previousBlock->timestamp + 300 > presentBlock->timestamp) {	// 제네시스 블록 NULL 오류 해결?
					if (!isValidCoinbase(presentBlock, tx, CoinbaseType::administratorCoinbase)) {
						cout << "\nThere is invalid administrator coinbase transaction in " << blockCount - i - 1 << "th block...\n";
						cout << "----- Administrator Coinbase Transaction Info -----\n";
						tx.print(cout);
						return false;
					}
				}
				else {
					if (!isValidCoinbase(presentBlock, tx, CoinbaseType::normalCoinbase)) {
						cout << "\nThere is invalid coinbase transaction in " << blockCount - i - 1 << "th block...\n";
						cout << "----- Coinbase Transaction Info -----\n";
						tx.print(cout);
						return false;
					}
				}
			}
			else {
				if (!isValid(tx, 1)) {
					cout << "\nThere is invalid transaction in " << blockCount - i - 1 << "th block...\n";
					cout << "----- Transaction Info -----\n";
					tx.print(cout);
					return false;
				}
			}
		}

		if (!presentBlock->isValid()) {			// 모든 블록이 유효한지 (블록 머클루트 계산 및 채굴이 유효한지)
			cout << "\nThere is invalid " << blockCount - i - 1 << "th block in the blockchain...\n";
			cout << "----- Block Info -----\n";
			presentBlock->print(cout);
			return false;
		}
	}

	return true;
}

bool Blockchain::isAdministratorAddress(const std::uint8_t * recipientPublicKeyHash) const {
	vector<uint8_t *> administratorAddresses;
	uint8_t administratorAddress[SHA256_DIGEST_VALUELEN] = { 0x39, 0xdc, 0x22, 0x7d, 0x9b, 0x8c, 0x3e, 0xb9, 0xd1, 0xe8, 0x84, 0x20,
	   0xd4, 0xe3, 0x6f, 0x1d, 0x9b, 0xa0, 0x91, 0xa1, 0xe5, 0xef, 0xf8, 0xd4, 0xc7, 0x50, 0xb1, 0x31, 0x5f, 0x61, 0xe4, 0xfd };
	administratorAddresses.push_back(administratorAddress);

	for (const uint8_t * administratorAddress : administratorAddresses) {
		if (isMemoryEqual(recipientPublicKeyHash, administratorAddress, sizeof(administratorAddress)))
			return true;
	}

	return false;
}

/* 블록 높이와 거래 해시를 입력하면 이전 거래를 반환한다. */
bool Blockchain::findPreviousTx(Transaction & previousTx, const uint8_t * previousTxHash, uint64_t blockHeight) const {
	assert(blockHeight < blockCount);

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount - blockHeight - 1; i++, presentBlock = presentBlock->previousBlock) {}

	for (const Transaction & tx : presentBlock->transactions) {
		if (isMemoryEqual(tx.txHash, previousTxHash, sizeof(tx.txHash))) {
			previousTx = tx;
			return true;
		}
	}

	cout << "There is no previous transaction matched with hash...\n";
	return false;
}

bool Blockchain::isTxOutputReferenceCount(const Transaction & tx, size_t outputIndex, uint64_t referenceCount) const {
	//if (tx.isCoinbase())   // 왜 넣었지? 넣으면 안 되는데
	//	return false;

	if (referenceCount != 0 && referenceCount != 1)
		return false;

	if (lastBlock == NULL)
		return false;

	assert(waitingBlock != NULL);
	assert(waitingBlock->previousBlock == lastBlock);

	uint64_t _referenceCount = 0;
	const Block * presentBlock = waitingBlock;
	for (size_t i = 0; i < blockCount + 1; i++, presentBlock = presentBlock->previousBlock) {
		for (const Transaction & _tx : presentBlock->transactions) {
			for (const Input & input : _tx.inputs) {
				if (isMemoryEqual(input.previousTxHash, tx.txHash, sizeof(tx.txHash)) && input.outputIndex == outputIndex) {
					_referenceCount++;
					if(_referenceCount > referenceCount)
						return false;
				}
			}
		}
	}
	
	if (_referenceCount != referenceCount)
		return false;
	return true;
}

bool Blockchain::findUTXOTable(std::vector<UTXO>& UTXOTable) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n";
		return false;
	}

	UTXOTable.clear();
	UTXOTable.shrink_to_fit();

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		for (const Transaction & tx : presentBlock->transactions) {
			int j = 0;
			for (const Output output : tx.outputs) {
				if (isTxOutputReferenceCount(tx, j, 0)) {
					UTXOTable.push_back(UTXO(tx.txHash, tx.outputs[j], j, blockCount - i - 1));
				}
				j++;
			}
		}
	}

	return true;
}

bool Blockchain::findUTXOTable(std::vector<UTXO>& UTXOTable, const State state) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n";
		return false;
	}

	UTXOTable.clear();
	UTXOTable.shrink_to_fit();

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		for (const Transaction & tx : presentBlock->transactions) {
			int j = 0;
			for (const Output output : tx.outputs) {
				if (isTxOutputReferenceCount(tx, j, 0) && output.state == state) {
					UTXOTable.push_back(UTXO(tx.txHash, tx.outputs[j], j, blockCount - i - 1));
				}
				j++;
			}
		}
	}

	return true;
}

bool Blockchain::findMyUTXOTable(vector<UTXO> & myUTXOTable, const std::uint8_t * _recipientPublicKeyHash) const {
	myUTXOTable.clear();
	myUTXOTable.shrink_to_fit();

	vector<UTXO> UTXOTable;
	if (!findUTXOTable(UTXOTable)) {
		return false;
	}

	for (UTXO & utxo : UTXOTable) {
		if (isMemoryEqual(utxo.output.recipientPublicKeyHash, _recipientPublicKeyHash, sizeof(utxo.output.recipientPublicKeyHash))) {
			myUTXOTable.push_back(utxo);
		}
	}

	return true;
}

bool Blockchain::findMyUTXOTable(vector<UTXO> & myUTXOTable, const std::uint8_t * _recipientPublicKeyHash, const State state) const {
	myUTXOTable.clear();
	myUTXOTable.shrink_to_fit();

	vector<UTXO> UTXOTable;
	if (!findUTXOTable(UTXOTable, state)) {
		return false;
	}

	for (UTXO & utxo : UTXOTable) {
		if (isMemoryEqual(utxo.output.recipientPublicKeyHash, _recipientPublicKeyHash, sizeof(utxo.output.recipientPublicKeyHash))) {
			myUTXOTable.push_back(utxo);
		}
	}

	return true;
}

string Blockchain::getFileName() const {
	time_t t = time(NULL);
	struct tm * date = localtime(&t);
	return name + to_string(lastBlock->version) + ' ' + to_string(date->tm_year + 1900) + ". " + to_string(date->tm_mon + 1)
		+ ". " + to_string(date->tm_mday) + "..txt";;
}


void Blockchain::printAllBlockHash() const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return;
	}

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		cout << "Block Hash: " << presentBlock->blockHash << '\n';
	}
}

void Blockchain::printAllMerkleHash() const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return;
	}

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		cout << "Merkle Root: " << presentBlock->merkleRoot << '\n';
	}
}

void Blockchain::print(ostream & o) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return;
	}

	const Block * presentBlock = lastBlock;
	o << "Blockchain Name: " << name << '\n';
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		o << "---------------------------------------------------------------------------\n";
		presentBlock->print(o);
	}
	cout << "\nPrinting was completed!\n";
}

