#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include "Blockchain.h"
using namespace std;


/* 현재 아래 순서를 지켜야 함.
1. UTXO Table 업데이트
2. 거래 생성
3. 채굴

bc.findMyUTXOTable(myUTXO, w.getPublicKeyHash());
w.setUTXOTable(myUTXO);
if (w.createTransaction(tx, 1, recipient.getPublicKeyHash(), Type::GLC, 1, 1, "from w to recipient 1000")) {
	bc.addTransactionToPool(tx);
	cout << "Transaction was created!\n";
}
else
	cout << "No balance....\n";
bc.produceBlock(w.getPublicKeyHash()); */

bool is_number(const string& s);

int main()
{
	Wallet recipient;
	cout << "Test recipient wallet was created\n\n";

	const uint8_t privateKey[SHA256_DIGEST_VALUELEN] = { 0x67, 0xd2, 0x26, 0x4b, 0x25, 0xbe, 0xd8, 0x25, 0xd0, 0x96, 0x42, 0x1b,
		 0x52, 0x13, 0x0d, 0x26, 0x2c, 0x26, 0x41, 0xa0, 0x7f, 0x06, 0xb0, 0xff, 0x33, 0x75, 0x9f, 0x06, 0x89, 0x96, 0xd2, 0x1a };
	Wallet w(privateKey);
	cout << "My wallet was created\n\n";

	Blockchain bc("GiftLink Blockchain", w.getPublicKeyHash());
	cout << bc.getName() << " was created\n\n";

	vector<UTXO> myUTXOTable;		// 참조되지 않은 내 소유의 UTXO
	vector<UTXO> UTXOTable;			// 참조되지 않은 모든 UTXO

	//err = getBroadcastedTransaction(tx)
	//if (bc.addTransactionToPool(tx))
	//	cout << "Transaction was added to Tx Pool!\n";
	//else
	//	cout << "Adding Tx to Tx Pool failed...\n";

HOME:
	{
		system("pause");
		system("cls");
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		string input;
		cout << "0.Home    1.Issue    2.State Transition    3.Buy    4.Use    5.Send    6.Produce    7.Infomation    \n";
		cin >> input;

		if (!is_number(input)) {
			cout << "Invalid number...\n";
			goto HOME;
		}

		int select;
		select = atoi(input.c_str());

		switch (select) {
		case 0:
			goto HOME;
		case 1:
			goto ISSUE;
		case 2:
			goto STATE_TRANSITION;
		case 3:
			//goto BUY;
		case 4:
			goto USE;
		case 5:
			goto SEND;
		case 6:
			goto PRODUCE;
		case 7:
			goto INFO;
		default:
			cout << "Invalid number...\n";
			goto HOME;
		}
	}

ISSUE: // 유가증권 발행
	{
		system("pause");
		system("cls");

		// 사용자로부터 입력받음
		Type type("GiftCard LoveShinak", 10000, 9000, time(NULL) + 8640000);
		int txCount = 1;
		int issueAmount = COINBASE_REWARD * 100;
		State securitiesState = State::own;
		State feeState = State::own;

		// inputPassword	// 비밀번호 확인

		if (bc.issueSecurities(w.getPublicKeyHash(), txCount, type, issueAmount, securitiesState, feeState))
			cout << "Security was issued!\n";
		else
			cout << "Issuing security failed...\n";

		// broadcastBlockchain(bc);

		goto HOME;
	}


STATE_TRANSITION: // 유가증권 상태 전환(소유<->판매)
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		vector<UTXO> mySecuritiesUTXOTable;
		for (const UTXO utxo : myUTXOTable) {
			if (utxo.output.type != Type() && (utxo.output.state == State::own || utxo.output.state == State::sale))
				mySecuritiesUTXOTable.push_back(utxo);
		}

		w.myUTXOTable = mySecuritiesUTXOTable;
		w.printMyUTXOTable(cout);

		if (w.myUTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		// 사용자로부터 입력받음
		size_t n = 0;																			// 선택한 유가증권 번호
		int64_t value = 10;																		// 소유한 유가증권 중 몇 개를 판매 중으로 전환할지 사용자로부터 입력받음
		int64_t fee = 1;																		// 사용자로부터 입력받거나 추천값 적용

		// 코드 스캔
		const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();								// (자신 또는 받는 사람) 코드 스캔

		// 자동 입력
		Type type = w.myUTXOTable[n].output.type;												// 사용자 소유의 유가증권
		State state = w.myUTXOTable[n].output.state == State::sale ? State::own : State::sale;	// 선택한 유가증권 번호로 판단

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (w.createTransactionSwitchState(tx, 1, ownerPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : own to sale")) {
			cout << "Transaction was created!\n";

			if (bc.addTransactionToPool(tx))
				cout << "Transaction was added to Tx Pool!\n";
			else
				cout << "Adding Tx to Tx Pool fail...\n";
		}
		else
			cout << "No balance....\n";

		//broadcastTransaction(tx);

		goto HOME;
	}

	/*
BUY: // 유가증권 구매
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findUTXOTable(UTXOTable, State::sale);												
		vector<UTXO> saleSecuritiesTable;
		for (const UTXO utxo : UTXOTable) {
			if (utxo.output.type.expirataionDate > time(NULL) && utxo.output.type != Type())	// 유가증권 유효기간 체크
				saleSecuritiesTable.push_back(utxo);
		}

		w.UTXOTable = saleSecuritiesTable;
		w.printUTXOTable(cout);

		if (w.UTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		size_t n = 0;																			// 구매할 유가증권 번호
		const uint8_t * recipientPublicKeyHash = w.UTXOTable[n].output.recipientPublicKeyHash;	// 판매자의 UTXO에서 얻어 옴
		Type type = w.UTXOTable[n].output.type;													// 판매자 소유의 유가증권 Type
		int64_t fee = 1;																		// 사용자로부터 입력받거나 추천값 적용

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (w.createTransactionPurchaseSale(tx, 1, recipientPublicKeyHash, type, 99, fee, "GiftCard LoveShinak : P2P Trade"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool fail...\n";
		//broadcastTransaction(tx);
	}
	*/

USE: // 유가증권 사용
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash(), State::own);

		vector<UTXO> mySecuritiesUTXOTable;		// 참조되지 않은 내 소유의 유가증권(Own)
		for (const UTXO utxo : myUTXOTable) {
			if (utxo.output.type != Type())
				mySecuritiesUTXOTable.push_back(utxo);
		}

		w.myUTXOTable = mySecuritiesUTXOTable;
		w.printMyUTXOTable(cout);

		if (w.myUTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		// 사용자 직접 입력
		size_t n = 0;																			// 선택한 유가증권 번호
		int64_t value = 99;																		// 얼마나 사용할지 사용자로부터 입력받음

		// 코드 스캔
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// 받는 사람 입력 또는 코드 스캔
		Type type = w.myUTXOTable[n].output.type;												// 사용자 소유의 유가증권 또는 코드 스캔으로 얻어 옴
		int64_t fee = 1;																		// 코드 스캔으로 얻어 옴

		// 자동 입력
		State state = State::spent;																// 유가증권 사용

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (w.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, 99, fee, state, "GiftCard LoveShinak : own to spent")) {	// sale to spent는?
			cout << "Transaction was created!\n";

			if (bc.addTransactionToPool(tx))
				cout << "Transaction was added to Tx Pool!\n";
			else
				cout << "Adding Tx to Tx Pool fail...\n";
		}
		else
			cout << "No balance....\n";

		//broadcastTransaction(tx);

		goto HOME;
	}


SEND: // 유가증권, 코인 전송
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash(), State::own);
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		if (w.myUTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		// 사용자 직접 입력
		size_t n = 0;																			// 선택한 유가증권 번호
		Type type = w.myUTXOTable[n].output.type;												// 사용자 소유의 유가증권
		int64_t value = 10;																		// 소유한 유가증권을 상대방에게 얼마나 전송할 건지
		int64_t fee = 1;																		// 사용자로부터 입력받거나 추천값 적용
		State state = w.myUTXOTable[n].output.state;											// 어떤 상태로 바꿀지 입력받음

		// 코드 스캔
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// 받는 사람

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (w.createTransaction(tx, 1, recipientPublicKeyHash, type, value, fee, "GiftCard LoveShinak : send")) {
			cout << "Transaction was created!\n";

			if (bc.addTransactionToPool(tx))
				cout << "Transaction was added to Tx Pool!\n";
			else
				cout << "Adding Tx to Tx Pool fail...\n";
		}
		else
			cout << "No balance....\n";

		//broadcastTransaction(tx);

		goto HOME;
	}


PRODUCE: // 블록 생성
	{
		system("pause");
		system("cls");

		//err = getBroadcastedTransaction(tx);

		// 사용자로부터 입력받음
		int txCount = MAX_TRANSACTION_COUNT;
		State feeState = State::own;

		if (bc.produceBlock(w.getPublicKeyHash(), txCount, feeState))
			cout << "Block was produced!\n";
		else
			cout << "Block producing fail...\n";

		//broadcastBlockchain(bc);

		goto HOME;
	}


INFO: // 블록체인 정보 불러오기
	{
		bc.print(cout);

		if (bc.isValid())
			cout << "Valid blockchain!\n";
		else
			cout << "Invalid blockchain...\n";

		cout << "Test complete!\n";

		goto HOME;
	}

	system("pause");
	return 0;
}

bool is_number(const string& s) {
	return !s.empty() && find_if(s.begin(),
		s.end(), [](char c) { return !isdigit(c); }) == s.end();
}