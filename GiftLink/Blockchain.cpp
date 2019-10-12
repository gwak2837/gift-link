#define _CRT_SECURE_NO_WARNINGS	/* localtime() */
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdint>
#include <cassert>
#include <map>
#include <algorithm>
#include "Blockchain.h"
#include "Block.h"
#include "Transaction.h"
#include "Wallet.h"
#include "Utility.h"
using namespace std;

// Genesis block을 생성한다.
Blockchain::Blockchain(string _name, const uint8_t * _recipientPublicKeyHash) : blockCount(0), name(_name), version(1) {
	cout << "\nCreating a blockchain...\n";

	Block * _genesisBlock = new Block();
	_genesisBlock->version = version;	// 현재 블록체인 version 입력

	vector<Input> inputs;				
	Input input;						// Coinbase input
	inputs.push_back(input);

	vector<Output> outputs;
	Output output(_recipientPublicKeyHash, COINBASE_REWARD, Type(), State::own);	// Coinbase output
	outputs.push_back(output);

	Transaction coinbaseTx(inputs, outputs, version, "GenesisBlock-CoinbaseTransaction");	// Coinbase Transaction
	_genesisBlock->transactions.push_back(coinbaseTx);
	memcpy(_genesisBlock->merkleRoot, coinbaseTx.txHash, SHA256_DIGEST_VALUELEN);
	_genesisBlock->mining();			// Genesis block을 생성
	
	genesisBlock = _genesisBlock;		// Genesis block 설정
	addBlock(_genesisBlock);			// Last block 설정

	cout << "The blockchain was created successfully!\n\n";

	Block * _waitingBlock = new Block(lastBlock);
	waitingBlock = _waitingBlock;
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
	if (txPool.size() == 0) {
		cout << "There aren't enough transactions in the memory pool to produce block...\n";
		return false;
	}

	if (txCount > MAX_TRANSACTION_COUNT)
		txCount = MAX_TRANSACTION_COUNT;

	auto iter = txPool.begin();
	for (size_t count = 0; count < MAX_TRANSACTION_COUNT - 1 && iter != txPool.end(); count++) {
		assert(isValid(txPool.front(), 0));					// 혹시나 tx가 Invalid하면 delete(메모리 해제)하고 큐에서 뺴기로
		assert(waitingBlock != NULL);
		waitingBlock->transactions.push_back(*iter);
		iter = txPool.erase(iter);
	}

	waitingBlock->version = version;		// 현재 blockchain version 입력

	vector<Input> inputs;
	Input coinbaseInput;
	inputs.push_back(coinbaseInput);

	vector<Output> outputs;
	Output blockProductionReward(recipientPublicKeyHash, COINBASE_REWARD, Type(), State::own);
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

	waitingBlock->transactions.insert(waitingBlock->transactions.begin(), coinbaseTx);
	waitingBlock->initializeMerkleRoot();	// Transaction hash로 Merkletree를 만들어 waiting block의 merkleroot 계산
	waitingBlock->mining();					// waiting block을 채굴
	addBlock(waitingBlock);					// waiting block을 블록체인에 추가
	waitingBlock->height = blockCount - 1;
	waitingBlock->isMainChain = true;
	cout << "Block was produced successfully!\n";

	Block * newWaitingBlock = new Block(lastBlock);
	waitingBlock = newWaitingBlock;
	
	return true;
}

bool Blockchain::issueSecurities(const uint8_t * recipientPublicKeyHash, int txCount, Type & type, int issueAmount, const State securitiesState, const State feeState) {
	//if (txPool.size() == 0) {
	//	cout << "There aren't enough transactions in the memory pool to produce block...\n";
	//	return false;
	//}

	if (txCount > MAX_TRANSACTION_COUNT)
		txCount = MAX_TRANSACTION_COUNT;

	if (issueAmount > COINBASE_REWARD * 100)
		issueAmount = COINBASE_REWARD * 100;

	auto iter = txPool.begin();
	for (size_t count = 0; count < txCount - 1 && iter != txPool.end(); count++) {
		assert(isValid(txPool.front(), 0));					// 혹시나 tx가 Invalid하면 delete(메모리 해제)하고 큐에서 뺴기로
		assert(waitingBlock != NULL);
		waitingBlock->transactions.push_back(*iter);
		iter = txPool.erase(iter);
	}

	waitingBlock->version = version;		// 현재 blockchain version 입력

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

	waitingBlock->transactions.insert(waitingBlock->transactions.begin(), coinbaseTx);
	waitingBlock->initializeMerkleRoot();	// Transaction hash로 Merkletree를 만들어 waiting block의 merkleroot 계산
	waitingBlock->mining();					// waiting block을 채굴
	addBlock(waitingBlock);					// waiting block을 블록체인에 추가
	waitingBlock->height = blockCount - 1;
	waitingBlock->isMainChain = true;
	cout << "Block was produced successfully!\n";

	Block * newWaitingBlock = new Block(lastBlock);
	waitingBlock = newWaitingBlock;

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

bool Blockchain::isValidCoinbase(const Block * block, const Transaction & coinbaseTx, CoinbaseType coinbaseType) const {
	if (!coinbaseTx.isCoinbase())
		return false;

	if (coinbaseTx.outputs[0].type != Type()) {															// 유가증권 발행 시
		if (coinbaseTx.outputs[0].value > COINBASE_REWARD * 100)											// 유가증권 최대 발행량 <= 블록 생성 보상 * 100
			return false;
	}
	else if (coinbaseTx.outputs[0].type == Type() && coinbaseTx.outputs[0].state == State::own) {		// 블록 생성 시
		if (coinbaseTx.outputs[0].value != COINBASE_REWARD)													// 블록 생성 보상
			return false;
	}
	else if (coinbaseType == CoinbaseType::administratorCoinbase) {										// 관리자 블록의 코인베이스이면
		if (coinbaseTx.outputs[0].value != 0)																// 블록 생성 보상 없음
			return false;
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
			Output output(blockProducerPublicKeyHash, kv.second, kv.first, State::own);
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

	if (!tx.isValid())		// 거래 서명 유효성(txHash + 공개키 + 서명)
		return false;

	Transaction testTx = tx;			// txHash 유효성	
	size_t inputIndex = 0;

	for (const Input & input : tx.inputs) {
		Transaction previousTx;
		if (!findPreviousTx(previousTx, input.previousTxHash, input.blockHeight))		// 이전 Output이 없으면
			return false;

		if (!isTxOutputReferenceCount(previousTx, input.outputIndex, previousOutputReferenceCount))	// 이전 Output 참조 횟수 검사
			return false;

		const Output & previosOutput = previousTx.outputs[input.outputIndex];
		memset(testTx.inputs[inputIndex].signature, 0, sizeof(testTx.inputs[inputIndex].signature));
		memcpy(testTx.inputs[inputIndex].signature, previosOutput.recipientPublicKeyHash, sizeof(previosOutput.recipientPublicKeyHash));

		uint8_t senderPublicKeyHash[SHA256_DIGEST_VALUELEN];
		SHA256_Encrpyt(input.senderPublicKey, sizeof(input.senderPublicKey), senderPublicKeyHash);
		if (!isMemoryEqual(previosOutput.recipientPublicKeyHash, senderPublicKeyHash, sizeof(senderPublicKeyHash)))	// SHA256(input.senderPubKey) == 이전 Output의 pubKeyHash
			return false;

		inputIndex++;
	}

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
	//if (tx.isCoinbase())   // 왜 넣었지?
	//	return false;

	if (referenceCount != 0 && referenceCount != 1)
		return false;

	if (lastBlock == NULL)
		return false;

	uint64_t _referenceCount = 0;
	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
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


/*
void Blockchain::printAllTransaction(ostream& o) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return;
	}

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++) {
		if (presentBlock->transactionsAreValid() && presentBlock->isValid()) {		// merkleRoot와 blockHash 유효성 검사
			size_t txSize = presentBlock->transactions.size();
			for (size_t j = 0; j < txSize; j++) {
				o << "\nTransaction #" << j + 1 << '\n';
				presentBlock->transactions[j]->print(o);
			}
			o << '\n';
			presentBlock = presentBlock->previousBlock;
		} else {
			cout << "Digital forgery had occured in " << presentBlock->height << "th block...";
			break;
		}
	}
}





vector<UTXO *> Blockchain::getIssuableGiftcardTable(const BYTE * privateKey) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n";
		return vector<UTXO *>();
	}

	const Block * presentBlock = lastBlock;
	vector<UTXO *> issuableGiftcardList;
	for (uint64_t i = 0; i < blockCount; i++) {
		int j = 0;
		for (Transaction * tx : presentBlock->transactions) {
			int k = 0;
			for (const Output * output : tx->getOutputs()) {
				if (output->getType() == Type::TOKEN) {
					Input input;
					input.generateSignature(privateKey);
					if (input.verifySignature(output->getPublicSignature(), output->getPubSigLength())) {
						UTXO * utxo = new UTXO(tx->getTransactionHash(), i, j, k, output->getValue(), output->getType(), output->getTypeIndex());
						issuableGiftcardList.push_back(utxo);
					}
				}
				k++;
			}
			j++;
		}
		presentBlock = presentBlock->previousBlock;
	}

	return issuableGiftcardList;
}

*/
///* 해당 tx의 모든 input이 참조하는 Output이 1번 참조됐는지(사용됐는지)
//0번 참조된 경우 Unspent Transaction Output(UTXO)이기 때문에 false 반환
//2번 이상 참조된 경우 이중지불에 해당하기 때문에 false 반환 
//참조하는 이전 Tx가 없는 경우 false 반환*/
//bool Blockchain::doesEveryInputReferToSTXO(const Transaction & tx) const {
//	if (tx.isCoinbase()) {
//		if (tx.outputs[0].value != COINBASE_REWARD)
//			return false;
//
//		if (tx.outputs[0].type != Type())
//			return false;
//
//		return true;
//	}
//
//	if (lastBlock == NULL) {
//		cout << "\nThere is no block in the blockchain...\n";
//		return false;
//	}
//
//	for (const Input & input : tx.inputs) {
//		Transaction previousTx;
//		if (!findPreviousTx(previousTx, input.blockHeight, input.previousTxHash))	// 참조하는 Output이 없으면 false 반환.
//			return false;
//
//		int referencedCount = 0;
//		const Block * presentBlock = lastBlock;
//
//		for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
//			for (const Transaction & _tx : presentBlock->transactions) {
//				for (const Input & _input : _tx.inputs) {
//					if (isMemoryEqual(_input.previousTxHash, previousTx.txHash, sizeof(previousTx.txHash)) && _input.outputIndex == input.outputIndex) {
//						referencedCount++;
//						if (referencedCount > 1) {		// 1번을 초과하여 참조되면 false 반환
//							cout << "\nPrevious transaction output is referenced twice or more...\n";
//							cout << "Previous transaction in " << input.blockHeight << "th block\n";
//							cout << "Previous transaction hash : " << input.previousTxHash << '\n';
//							cout << "Previous transaction output index : " << input.outputIndex << "\n\n";
//							return false;
//						}
//					}
//				}
//			}
//		}
//
//		if (referencedCount == 0) {
//			cout << "\nPrevious transaction output is not referenced...\n";
//			cout << "Previous transaction in " << input.blockHeight << "th block\n";
//			cout << "Previous transaction hash : " << input.previousTxHash << '\n';
//			cout << "Previous transaction output index : " << input.outputIndex << "\n\n";
//			return false;
//		}
//	}
//
//	return true;
//}
//
///* 해당 tx의 모든 input이 참조하는 Output이 0번 참조됐는지(사용되지 않았는지)
//1번 이상 참조되면 사용된 경우(2번 이상 참조되면 이중지불)에 해당하므로 false 반환 */
//bool Blockchain::doesEveryInputReferToUTXO(const Transaction & tx) const {
//	if (tx.isCoinbase()) {
//		if (tx.outputs[0].value != COINBASE_REWARD)
//			return false;
//
//		if (tx.outputs[0].type != Type())
//			return false;
//
//		return true;
//	}
//	
//	if (lastBlock == NULL) {
//		cout << "\nThere is no block in the blockchain...\n";
//		return false;
//	}
//
//	for (const Input & input : tx.inputs) {
//		Transaction previousTx;
//		if (!findPreviousTx(previousTx, input.blockHeight, input.previousTxHash)) 	// 참조하는 Output이 없으면 false 반환.
//			return false;
//
//		if (!isUTXO(previousTx, input.outputIndex))
//			return false;
//	}
//
//	return true;
//}