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

	Block * _waitingBlock = new Block(lastBlock);
	waitingBlock = _waitingBlock;
}


bool Blockchain::isValid() const {

	// 모든 블록이 Block::isValid() 인지 (블록 머클루트 계산 및 채굴이 유효한지)
	// Coinbase 거래의 수수료 합산이 유효한지
	// 모든 거래(Coinbase + 일반)가 Blockchain::isValid(tx) 인지



	return false;
}

// UTXO를 참조하고 금액이 정확한지(input amount >= output amount), 서명이 유효한지, 입력의 공개키 해시가 참조된 출력의 해시와 같은지
bool Blockchain::isValid(const Transaction * tx) const {
	if (tx->isCoinbase()) {
		if (tx->outputs[0].value != COINBASE_REWARD)
			return false;

		if (tx->outputs[0].type == Type::GLC)
			return false;
	}
	else {
		map<Type, int64_t> mapTypeValue;

		for (const Input & input : tx->inputs) {
			Output & output = findPreviousOutput(input.blockHeight, input.previousTxHash, input.outputIndex);

			uint8_t senderPublicKeyHash[SHA256_DIGEST_VALUELEN];		// Input의 SHA256(pubKey)가 이전 Output의 pubKeyHash와 같은지
			SHA256_Encrpyt(input.senderPublicKey, sizeof(input.senderPublicKey), senderPublicKeyHash);
			if (!isMemoryEqual(output.recipientPublicKeyHash, senderPublicKeyHash, sizeof(senderPublicKeyHash)))
				return false;

			const struct uECC_Curve_t * curve = uECC_secp256r1();		// Input의 공개키와 서명이 유효한지
			if (uECC_verify(input.senderPublicKey, tx->txHash, sizeof(tx->txHash), input.signature, curve) == 0)
				return false;

			if (!isUTXO(output))	// UTXO를 참조하는지 -->> 고아 거래 풀에 담김.
				return false;

			map<Type, int64_t>::iterator iter = mapTypeValue.find(output.type);	// 이전 Output의 Type별 Value 합산 후 저장
			if (iter != mapTypeValue.end())
				iter->second += output.value;
			else
				mapTypeValue.insert(pair<Type, int64_t>(output.type, output.value));
		}

		// inputTypeValue와 outputTypeValue의 비교
		for (const Output & output : tx->outputs) {
			auto iter = mapTypeValue.find(output.type);
			if (iter != mapTypeValue.end()) {
				iter->second -= output.value;
				if (iter->second < 0)		// Input의 Value보다 Output의 Value가 더 크면
					return false;
			}
			else	// Output의 Type이 Input에 없으면
				return false;
		}
	}

	return true;
}




/* 개발 중 : 유효한 Tx인지 확인(Tx 총 Input >= 총 Output 확인 등)
transaction pool에 transaction을 추가한다. */
void Blockchain::addTransactionToPool(Transaction * _tx) {
	if (isValid(_tx))
		transactionPool.push(_tx);
	else
		cout << "Unvalid Transaction";
}

/* transaction pool에서 transaction을 가져와 채굴한다.

Coinbase Transaction 생성 과정
1. Input 설정
2. Output 설정
3. Transaction 데이터 해싱 */
void Blockchain::produceBlock(const uint8_t * _recipientPublicKeyHash) {
	if (transactionPool.size() >= MAX_TRANSACTION_COUNT) {
		waitingBlock->addTransactionsFrom(transactionPool);
		waitingBlock->version = version;		// 현재 blockchain version 입력

		vector<Input> inputs;
		Input input;
		inputs.push_back(input);

		vector<Output> outputs;
		Output output(_recipientPublicKeyHash, COINBASE_REWARD, Type::GLC);	// Coinbase 채굴 보상 Output
		outputs.push_back(output);

		int64_t reward = 0;
		for (Transaction & tx : waitingBlock->transactions) {
			// 블록에 있는 거래가 참조하는 UTXO를 확인해서 그 거래의 총 Input 계산
			for (Input & input : tx.inputs) {
				assert(lastBlock != NULL);

				Output & utxo = findPreviousOutput(input.blockHeight, input.previousTxHash, input.outputIndex);
				(int)utxo.type;
				utxo.value;

				



			}

			// 만약 Type이 다르면 따로 Output 생성
			

			reward += 10000 - tx.getTotalCoinOutputs(); // Tx의 총 Input과 총 Output의 차이 계산
		}
		Output output2(_recipientPublicKeyHash, reward, Type::GLC);
		outputs.push_back(output2);

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
	}
	else {
		cout << "There aren't enough transactions in the memory pool to produce block...\n";
	}
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
	for (uint64_t i = 0; i < blockCount; i++) {
		cout << "Block Hash: " << presentBlock->blockHash << '\n';
		presentBlock = presentBlock->previousBlock;
	}
}

void Blockchain::printAllMerkleHash() const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return;
	}

	const Block * presentBlock = lastBlock;
	for (uint64_t i = 0; i < blockCount; i++) {
		cout << "Merkle Root: " << presentBlock->merkleRoot << '\n';
		presentBlock = presentBlock->previousBlock;
	}
}


void Blockchain::print(ostream & o) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n\n";
		return;
	}

	const Block * presentBlock = lastBlock;
	o << "Blockchain Name: " << name << '\n';
	for (uint64_t i = 0; i < blockCount; i++) {

		// block.print(o) 로 변경

		o << "---------------------------------------------------------------------------\n";
		o << "Block #" << presentBlock->height << '\n';
		o << "Block Hash:  " << presentBlock->blockHash << '\n';
		o << "Version:     " << presentBlock->version << '\n';
		o << "Previous \nBlock Hash:  " << presentBlock->previousBlockHash << '\n';
		o << "Merkle Hash: " << presentBlock->merkleRoot << '\n';
		o << "Timestamp:   " << timeToString(presentBlock->timestamp) << '\n';
		o << "Bits:        " << (int)presentBlock->bits << '\n';
		o << "Nonce:       " << presentBlock->nonce << "\n\n";

		int j = 0;
		for (const Transaction & tx : presentBlock->transactions) {
			o << "Transaction #" << j << '\n';
			tx.print(o);
			j++;
		}
		
		presentBlock = presentBlock->previousBlock;
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
	for (uint64_t i = 0; i < blockCount; i++) {
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


vector<UTXO *> Blockchain::getUTXOTable() const {
	return vector<UTXO *>();
}

// Private Key를 받아 해당 key의 사용되지 않은 거래를 찾아서 반환한다.
vector<UTXO *> Blockchain::getMyUTXOTable(const BYTE * privateKey) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n";
		return vector<UTXO *>();
	}
	
	vector<UTXO *> myTXOs;
	const Block * presentBlock = lastBlock;
	// 블록체인에서 자신의 Transaction Output을 찾는다.
	for (uint64_t i = 0; i < blockCount; i++) {
		int j = 0;
		for (Transaction * tx : presentBlock->transactions) {
			int k = 0;
			for(Output * output : tx->getOutputs()) {
				Input input;
				input.generateSignature(privateKey);
				if (input.verifySignature(output->getPublicSignature(), output->getPubSigLength())) {




					UTXO * txo = new UTXO(tx->getTransactionHash(), i, j, k, output->getValue(), output->getType(), output->getTypeIndex());
					myTXOs.push_back(txo);
				}
				k++;
			}
			j++;
		}
		presentBlock = presentBlock->previousBlock;
	}

	// 모든 Transaction에 대해 참조되지 않은 Transaction만 찾는다. O(n^2), n = myTransactions.size()
	vector<UTXO *> myUTXOs;
	for (const Transaction * myTx : myTransactions) {
		for (const Transaction * myTx2 : myTransactions) {
			for(const Input * input : myTx2->getInputs()) {
				if (Block::isMemoryEqual(myTx->getTransactionHash(), input->getPreviousTxHash(), SHA256_DIGEST_VALUELEN))
					goto REFERENCED;
			}
		}

		// 참조되지 않은 Transaction의 output만 myUTXOTable에 넣는다.
		for (const Output * output : myTx->getOutputs()) {
			if (Block::isMemoryEqual(publicKey, output->getPublicSignature(), SHA256_DIGEST_VALUELEN)) {
				UTXO * myUTXO = new UTXO(myTx->getTransactionHash(), 0, 0, 0, output->getValue(), output->getType(), output->getTypeIndex());
				myUTXOTable.push_back(myUTXO);
			}
		}
		
		// 참조된 Transaction output은 Table에 넣지 않는다.
		REFERENCED:
		continue;
	}
	
	return myUTXOTable;
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

bool Blockchain::isUTXO(const Transaction * _tx) const {
	if (lastBlock == NULL) {
		cout << "\nThere is no block in the blockchain...\n";
		return false;
	}

	const Block * presentBlock = lastBlock;
	for (uint64_t i = 0; i < blockCount - 1; i++) {
		for (const Transaction * tx : presentBlock->transactions) {
			for (const Input * input : tx->getInputs()) {
				if (Block::isMemoryEqual(_tx->getTransactionHash(), input->getPreviousTxHash(), SHA256_DIGEST_VALUELEN))
					return false;
			}
		}
		presentBlock = presentBlock->previousBlock;
	}

	for (const Transaction * tx : presentBlock->transactions) {
		for (const Input * input : tx->getInputs()) {
			if (Block::isMemoryEqual(_tx->getTransactionHash(), input->getPreviousTxHash(), SHA256_DIGEST_VALUELEN))
				return false;
		}
	}
	return true;
}



*/
