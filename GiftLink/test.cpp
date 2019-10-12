#include <iostream>
#include <fstream>
#include <cstdlib>
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

int main()
{
	Wallet recipient;
	cout << "Test recipient wallet was created\n\n";

	Wallet w;
	cout << "My wallet was created\n\n";

	Blockchain bc("GiftLink Blockchain", w.getPublicKeyHash());
	cout << bc.getName() << " was created\n\n";

	vector<UTXO> myUTXOTable;		// 참조되지 않은 내 소유의 유가증권(Own + Sale + Spent)
	vector<UTXO> UTXOTable;			// 참조되지 않은 모든 유가증권

	Transaction tx;

	//err = getBroadcastedTransaction(tx)
	//if (bc.addTransactionToPool(tx))
	//	cout << "Transaction was added to Tx Pool!\n";
	//else
	//	cout << "Adding Tx to Tx Pool failed...\n";

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
	}

	
STATE_TRANSITION: // 유가증권 상태 전환(소유<->판매)
	{
		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		// 사용자로부터 입력받음
		size_t n = 0;																			// 선택한 유가증권 번호
		int64_t value = 99;																		// 소유한 유가증권 중 몇 개를 판매 중으로 전환할지 사용자로부터 입력받음
		int64_t fee = 1;																		// 사용자로부터 입력받거나 추천값 적용

		// 코드 스캔
		const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();								// (자신 또는 받는 사람) 코드 스캔

		// 자동 계산
		Type type = myUTXOTable[n].output.type;													// 사용자 소유의 유가증권
		State state = myUTXOTable[n].output.state == State::sale ? State::own : State::sale;	// 선택한 유가증권 번호로 판단

		// inputPassword	// 비밀번호 확인

		if (w.createTransactionSwitchState(tx, 1, ownerPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : own to sale"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool fail...\n";
		//broadcastTransaction(tx);
	}

	/*
BUY: // 유가증권 구매
	{
		//err = getBroadcastedBlockchain(bc);
		bc.findUTXOTable(UTXOTable, State::sale);												// 유가증권 유효기간 체크
		w.UTXOTable = UTXOTable;
		w.printUTXOTable(cout);

		size_t n = 0;																			// 구매할 유가증권 번호
		const uint8_t * recipientPublicKeyHash = UTXOTable[n].output.recipientPublicKeyHash;	// 판매자의 UTXO에서 얻어 옴
		Type type = UTXOTable[n].output.type;													// 판매자 소유의 유가증권 Type
		int64_t fee = 1;																		// 사용자로부터 입력받거나 추천값 적용

		// inputPassword	// 비밀번호 확인

		if (w.createTransaction(tx, 1, recipientPublicKeyHash, type, 99, fee, "GiftCard LoveShinak : P2P Trade"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool fail...\n";
		//broadcastTransaction(tx);
	}
	
	
USE: // 유가증권 사용
	{
		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		// 사용자 직접 입력
		size_t n = 0;																			// 선택한 유가증권 번호
		int64_t value = 99;																		// 얼마나 사용할지 사용자로부터 입력받음

		// 코드 스캔
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// 받는 사람 입력 또는 코드 스캔
		Type type = myUTXOTable[n].output.type;													// 사용자 소유의 유가증권 또는 코드 스캔으로 얻어 옴
		int64_t fee = 1;																		// 코드 스캔으로 얻어 옴

		// 자동 계산
		State state = State::spent;																// 유가증권 사용

		// inputPassword	// 비밀번호 확인

		if (w.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, 99, fee, state, "GiftCard LoveShinak : own to spent"))	// sape to spent는?
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool fail...\n";
		//broadcastTransaction(tx);
	}
	

SEND: // 유가증권 전송
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		//size_t n = 0;																			// 선택한 유가증권 번호
		//const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();
		//Type type = myUTXOTable[n].output.type;													// 사용자 소유의 유가증권
		//int64_t value = 99;																		// 소유한 유가증권 중 몇 개를 판매 중으로 전환할지 사용자로부터 입력받음
		//int64_t fee = 1;																		// 사용자로부터 입력받거나 추천값 적용
		//State state = myUTXOTable[n].output.state == State::sale ? State::own : State::sale;		// 사용자로부터 어떤 상태로 바꿀지 입력받음

		// inputPassword	// 비밀번호 확인

		if (w.createTransactionSwitchState(tx, 1, ownerPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : send"))
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
	}


INFO: // 블록체인 정보 불러오기
	{}

	bc.print(cout);

	if (bc.isValid())
		cout << "Valid blockchain!\n";
	else
		cout << "Invalid blockchain...\n";

	cout << "Test complete!\n";
	system("pause");
	return 0;
}