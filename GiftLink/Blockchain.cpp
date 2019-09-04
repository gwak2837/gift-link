#define _CRT_SECURE_NO_WARNINGS	/* localtime() */
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdint>
#include <cassert>
#include <map>
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
	Output output(_recipientPublicKeyHash, COINBASE_REWARD, Type::GLC);	// Coinbase output
	outputs.push_back(output);

	Transaction coinbaseTx(inputs, outputs, version, "GenesisBlock-CoinbaseTransaction");	// Coinbase Transaction
	_genesisBlock->transactions.push_back(coinbaseTx);
	memcpy(_genesisBlock->merkleRoot, coinbaseTx.txHash, SHA256_DIGEST_VALUELEN);
	_genesisBlock->mining();			// Genesis block을 채굴
	
	genesisBlock = _genesisBlock;		// Genesis block 설정
	addBlock(_genesisBlock);			// Last block 설정

	cout << "The blockchain was created successfully!\n\n";

	Block * _waitingBlock = new Block(lastBlock);
	waitingBlock = _waitingBlock;
}

// UTXO를 참조하고 금액이 정확한지(input amount >= output amount), 서명이 유효한지, 입력의 공개키 해시가 참조된 출력의 해시와 같은지
bool Blockchain::isValid(const Transaction & tx) const {
	if (tx.isCoinbase()) {
		if (tx.outputs[0].value != COINBASE_REWARD)
			return false;

		if (tx.outputs[0].type == Type::GLC)
			return false;
	}
	map<Type, int64_t> mapTypeValue;

	for (const Input & input : tx.inputs) {
		const Transaction & previousTx = findPreviousTx(input.blockHeight, input.previousTxHash);
		const Output & previosOutput = previousTx.outputs[input.outputIndex];

		uint8_t senderPublicKeyHash[SHA256_DIGEST_VALUELEN];		// Input의 SHA256(pubKey)가 이전 Output의 pubKeyHash와 같은지
		SHA256_Encrpyt(input.senderPublicKey, sizeof(input.senderPublicKey), senderPublicKeyHash);
		if (!isMemoryEqual(previosOutput.recipientPublicKeyHash, senderPublicKeyHash, sizeof(senderPublicKeyHash)))
			return false;

		const struct uECC_Curve_t * curve = uECC_secp256r1();		// Input의 공개키와 서명이 유효한지
		if (uECC_verify(input.senderPublicKey, tx.txHash, sizeof(tx.txHash), input.signature, curve) == 0)
			return false;

		if (!isUTXO(previousTx, input.outputIndex))			// UTXO를 참조하는지 -->> 아니면 일단 고아 거래 풀에 담김.
			return false;

		auto iter = mapTypeValue.find(previosOutput.type);	// 이전 Output의 Type별 Value 합산 후 저장
		if (iter != mapTypeValue.end())
			iter->second += previosOutput.value;
		else
			mapTypeValue.insert(pair<Type, int64_t>(previosOutput.type, previosOutput.value));
	}

	for (const Output & output : tx.outputs) {		// inputTypeValue와 outputTypeValue의 비교
		auto iter = mapTypeValue.find(output.type);
		if (iter != mapTypeValue.end()) {
			iter->second -= output.value;
			if (iter->second < 0)		// Input의 Value보다 Output의 Value가 더 크면
				return false;
		}
		else	// Output의 Type이 Input에 없으면
			return false;
	}

	return true;
}


/* 유효한 Tx인지 확인(Tx 총 Input >= 총 Output 확인 등)
transaction pool에 transaction을 추가한다. */
void Blockchain::addTransactionToPool(Transaction & _tx) {
	if (isValid(_tx))
		txPool.push(_tx);
	else
		cout << "Invalid Transaction...\n";
}

/* transaction pool에서 transaction을 가져와 채굴한다.
성공적으로 블록을 생성하면 true, 아니면 false를 반환한다.

Coinbase Transaction 생성 과정
1. Input 설정
2. Output 설정
3. Transaction 데이터 해싱 */
bool Blockchain::produceBlock(const uint8_t * _recipientPublicKeyHash) {
	if (txPool.size() < MAX_TRANSACTION_COUNT) {
		cout << "There aren't enough transactions in the memory pool to produce block...\n";
		return false;
	}

	for (int i = 0; i < MAX_TRANSACTION_COUNT; i++) {		// Transaction Pool에서 tx을 가져와서 block에 넣는다.
		assert(isValid(txPool.front()));					// 혹시나 tx가 Invalid하면 delete(메모리 해제)하고 큐에서 뺴기로
		assert(waitingBlock != NULL);
		waitingBlock->transactions.push_back(txPool.front());
		txPool.pop();
	}

	waitingBlock->version = version;		// 현재 blockchain version 입력

	vector<Input> inputs;
	Input input;
	inputs.push_back(input);

	vector<Output> outputs;
	Output output(_recipientPublicKeyHash, COINBASE_REWARD, Type::GLC);	// Coinbase 채굴 보상 Output
	outputs.push_back(output);

	map<Type, int64_t> mapTypeValue;

	for (const Transaction & tx : waitingBlock->transactions) {
		for (const Input & input : tx.inputs) {
			const Transaction & previousTx = findPreviousTx(input.blockHeight, input.previousTxHash);
			const Output & previosOutput = previousTx.outputs[input.outputIndex];

			auto iter = mapTypeValue.find(previosOutput.type);	// 이전 Output의 Type별 Value 합산 후 저장
			if (iter != mapTypeValue.end())
				iter->second += previosOutput.value;
			else
				mapTypeValue.insert(pair<Type, int64_t>(previosOutput.type, previosOutput.value));
		}
	}

	// Type-Value별로 Output 생성
	for (const auto & kv : mapTypeValue) {
		Output output2(_recipientPublicKeyHash, kv.second, kv.first);
		outputs.push_back(output2);
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

/* 개발 중 : Coinbase 거래의 수수료 합산이 유효한지 */
bool Blockchain::isValid() const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return false;
	}

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		for (const Transaction & tx : presentBlock->transactions) {			// 모든 거래(Coinbase + 일반)가 유효한지
			if (!isValid(tx)) {			
				cout << "\nThere is invalid transaction in " << blockCount - i - 1 << "th block...\n";
				cout << "----- Transaction Info -----";
				tx.print(cout);
				return false;
			}
		}
		
		// Coinbase 거래의 수수료 합산이 유효한지

		if (presentBlock->isValid()) {			// 모든 블록이 Block::isValid() 인지 (블록 머클루트 계산 및 채굴이 유효한지)
			cout << "\nThere is invalid " << blockCount - i - 1 << "th block in the blockchain...\n";
			cout << "----- Block Info -----";
			presentBlock->print(cout);
			return false;
		}
	}

	return true;
}

/* 개발 중 : 모든 경로에서 값을 반환하지 않습니다. */
/* 블록 높이와 거래 해시를 입력하면 이전 거래의 위치를 반환한다.
찾지 못하면 NULL을 반환한다. */
const Transaction & Blockchain::findPreviousTx(uint64_t blockHeight, const uint8_t * previousTxHash) const {
	assert(blockHeight < blockCount);

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount - blockHeight - 1; i++, presentBlock = presentBlock->previousBlock) {}

	for (const Transaction & tx : presentBlock->transactions) {
		if (isMemoryEqual(tx.txHash, previousTxHash, sizeof(tx.txHash))) {
			return tx;
		}
	}

	cout << "There is no previous transaction matched with hash...\n";
	assert(false);
}

bool Blockchain::isUTXO(const Transaction & _tx, int outputIndex) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n";
		return false;
	}

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		for (const Transaction & tx : presentBlock->transactions) {
			for (const Input & input : tx.inputs) {
				if (isMemoryEqual(input.previousTxHash, _tx.txHash, sizeof(tx.txHash)) && input.outputIndex == outputIndex)
					return false;
			}
		}
	}

	return true;
}

bool Blockchain::findUTXOTable(std::vector<UTXO>& UTXOTable) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n";
		return false;
	}

	const Block * presentBlock = lastBlock;
	for (size_t i = 0; i < blockCount; i++, presentBlock = presentBlock->previousBlock) {
		for (const Transaction & tx : presentBlock->transactions) {
			for (int i = 0; i < tx.outputs.size(); i++) {
				if (isUTXO(tx, i)) {
					UTXOTable.push_back(UTXO(tx.txHash, tx.outputs[i]));
				}
			}
		}
	}

	return true;
}

bool Blockchain::findMyUTXOTable(vector<UTXO> & myUTXOTable, const std::uint8_t * _recipientPublicKeyHash) const {
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


vector<UTXO *> Blockchain::findUTXOTable() const {
	return vector<UTXO *>();
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
