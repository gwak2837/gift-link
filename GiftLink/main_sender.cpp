#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <queue>
#include <string>
#include <future>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include "Blockchain.h"
#include "BroadcastListener.h"
#include "BlockBroadcaster.h"
using namespace std;


/* 현재 아래 순서를 지켜야 함.
1. UTXO Table 업데이트
2. 거래 생성
3. 채굴

bc.findMyUTXOTable(myUTXO, myWallet.getPublicKeyHash());
myWallet.setUTXOTable(myUTXO);
if (myWallet.createTransaction(tx, 1, recipient.getPublicKeyHash(), Type::GLC, 1, 1, "from myWallet to recipient 1000")) {
	bc.addTransactionToPool(tx);
	cout << "Transaction was created!\n";
}
else
	cout << "No balance....\n";
bc.produceBlock(myWallet.getPublicKeyHash()); */

bool is_number(const string& s);

int main()
{
	const uint8_t recipientPrivateKey[SHA256_DIGEST_VALUELEN] = { 0x00, 0xd2, 0x26, 0x4b, 0x25, 0xbe, 0xd8, 0x25, 0xd0, 0x96, 0x42, 0x1b,
		 0x52, 0x13, 0x0d, 0x26, 0x2c, 0x26, 0x41, 0xa0, 0x7f, 0x06, 0xb0, 0xff, 0x33, 0x75, 0x9f, 0x06, 0x89, 0x96, 0xd2, 0x1a };
	Wallet recipient(recipientPrivateKey);
	cout << "Recipient wallet was created\n\n";

	const uint8_t myPrivateKey[SHA256_DIGEST_VALUELEN] = { 0x67, 0xd2, 0x26, 0x4b, 0x25, 0xbe, 0xd8, 0x25, 0xd0, 0x96, 0x42, 0x1b,
		 0x52, 0x13, 0x0d, 0x26, 0x2c, 0x26, 0x41, 0xa0, 0x7f, 0x06, 0xb0, 0xff, 0x33, 0x75, 0x9f, 0x06, 0x89, 0x96, 0xd2, 0x1a };
	Wallet myWallet(myPrivateKey);
	cout << "My wallet was created\n\n";

	Blockchain bc("GiftLink Blockchain", myWallet.getPublicKeyHash());
	cout << '\n' << bc.getName() << " was created\n\n";
	bc.broadcastBlock();

	vector<UTXO> myUTXOTable;		// 참조되지 않은 내 소유의 UTXO
	vector<UTXO> UTXOTable;			// 참조되지 않은 모든 UTXO

	future<bool> future_block_produce;

	BroadcastListener bbl(8888);
	future<void> f = async(launch::async, &BroadcastListener::listen, ref(bbl), ref(bc));
	//TransactionBroadcastListener tbl(8888);
	//future<void> f = async(launch::async, &TransactionBroadcastListener::listen, ref(tbl));

HOME:
	{
		system("pause");
		system("cls");
		bc.findMyUTXOTable(myUTXOTable, myWallet.getPublicKeyHash());
		myWallet.myUTXOTable = myUTXOTable;
		myWallet.printMyUTXOTable(cout);

		string input;
		cout << "0.Home    1.Issue    2.Sell    3.Buy    4.Use    5.Send    6.Produce    7.Blockchain Infomation    \n";
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
			goto SELL;
		case 3:
			goto BUY;
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

		if (future_block_produce.valid()) {
			if (future_block_produce.wait_for(chrono::nanoseconds(0)) == future_status::timeout) {
				cout << "Still producing the block...\n";
				goto HOME;
			}
			else {
				if (future_block_produce.get())
					cout << "Block producing suceed!\n";
				else
					cout << "Block producing failed...\n";
			}
		}

		// 사용자로부터 입력받음
		cout << "GiftCard Name : ";
		string name;
		cin >> name;

		cout << "Face Value : ";
		int64_t faceValue;
		cin >> faceValue;

		cout << "Market Value : ";
		int64_t marketValue;
		cin >> marketValue;

		cout << "Expiration Date : ";
		time_t expirationDate;
		cin >> expirationDate;

		// 자동 입력
		Type type(name, faceValue, marketValue, time(NULL) + expirationDate, myWallet.getPrivateKey());
		int txCount = 1;
		int64_t issueAmount = bc.getMaxIssueAmount(type);
		State securitiesState = State::OWN;
		State feeState = State::OWN;

		// inputPassword	// 비밀번호 확인

		future_block_produce = async(launch::async, &Blockchain::issueSecurities, ref(bc), myWallet.getPublicKeyHash(), txCount, type, issueAmount, securitiesState, feeState);

		goto HOME;
	}


SELL: // 유가증권 상태 전환(소유<->판매)
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);

		bc.findMyUTXOTable(myUTXOTable, myWallet.getPublicKeyHash());
		vector<UTXO> mySecuritiesUTXOTable;
		for (const UTXO utxo : myUTXOTable) {
			if (utxo.output.type != Type() && (utxo.output.state == State::OWN || utxo.output.state == State::SALE))
				mySecuritiesUTXOTable.push_back(utxo);
		}

		myWallet.myUTXOTable = mySecuritiesUTXOTable;
		myWallet.printMyUTXOTable(cout);

		if (myWallet.myUTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		// 사용자 직접 입력
		cout << "Select : ";
		size_t n;											// 상태를 전환할 유가증권 번호
		cin >> n;

		cout << "Value : ";
		int64_t value;										// 소유한 유가증권 중 몇 개를 판매 중으로 전환할지 사용자로부터 입력받음
		cin >> value;

		cout << "Transaction Fee : ";
		int64_t fee;										// 사용자로부터 입력받거나 추천값 적용
		cin >> fee;


		// 코드 스캔
		const uint8_t * ownerPublicKeyHash = myWallet.getPublicKeyHash();								// (자신 또는 받는 사람) 코드 스캔

		// 자동 입력
		Type type = myWallet.myUTXOTable[n].output.type;												// 사용자 소유의 유가증권
		State state = myWallet.myUTXOTable[n].output.state == State::SALE ? State::OWN : State::SALE;	// 선택한 유가증권 번호로 판단

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (myWallet.createTransactionSwitchState(tx, 1, ownerPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : own to sale")) {
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

	//**************************************유가증권 판매 현황 검사 필요 : '유가증권 전체 판매량 < 유가증권 발행량'이어야 구매 가능
	//**************************************구매할 유가증권은 자기 꺼는 제외하고 
BUY: // 유가증권 구매
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);

		bc.findUTXOTable(UTXOTable, State::SALE);
		vector<UTXO> saleSecuritiesTable;
		for (const UTXO utxo : UTXOTable) {
			if (utxo.output.type.expirationDate > time(NULL) && utxo.output.type != Type())	// 유가증권 유효기간 체크
				saleSecuritiesTable.push_back(utxo);
		}

		bc.findMyUTXOTable(myUTXOTable, myWallet.getPublicKeyHash());
		vector<UTXO> myGLCTable;
		for (const UTXO utxo : myUTXOTable) {
			if (utxo.output.type == Type() && utxo.output.state == State::OWN)					// 내 소유의 GLC
				myGLCTable.push_back(utxo);
		}

		myWallet.UTXOTable = saleSecuritiesTable;
		myWallet.myUTXOTable = myGLCTable;
		myWallet.printUTXOTable(cout);
		myWallet.printMyUTXOTable(cout);

		if (myWallet.UTXOTable.size() == 0) {
			cout << "No sale securities...\n";
			goto HOME;
		}

		if (myWallet.myUTXOTable.size() == 0) {
			cout << "No my GLC...\n";
			goto HOME;
		}

		// 사용자 직접 입력
		cout << "Select : ";
		size_t n;																			// 구매할 유가증권 번호
		cin >> n;

		cout << "Value : ";
		int64_t value;																		// 얼마나 구매할지
		cin >> value;

		cout << "Transaction Fee : ";
		int64_t fee;																		//********************어디서 얻어오나?? 판매자가 정함
		cin >> fee;

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (myWallet.createTransactionPurchaseSale(tx, 1, myWallet.UTXOTable[n], value, fee, "GiftCard LoveShinak : Securities Purchase")) {
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


USE: // 유가증권 사용
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);

		bc.findMyUTXOTable(myUTXOTable, myWallet.getPublicKeyHash(), State::OWN);

		vector<UTXO> mySecuritiesUTXOTable;		// 참조되지 않은 내 소유의 유가증권(Own)
		for (const UTXO utxo : myUTXOTable) {
			if (utxo.output.type != Type())
				mySecuritiesUTXOTable.push_back(utxo);
		}

		myWallet.myUTXOTable = mySecuritiesUTXOTable;
		myWallet.printMyUTXOTable(cout);

		if (myWallet.myUTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		// 사용자 직접 입력
		cout << "Select : ";
		size_t n;																				// 사용할 유가증권 번호
		cin >> n;

		cout << "Value : ";
		int64_t value;																			// 얼마나 사용할지 사용자로부터 입력받음
		cin >> value;


		// 코드 스캔
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// 받는 사람 입력 또는 코드 스캔
		Type type = myWallet.myUTXOTable[n].output.type;												// 사용자 소유의 유가증권 또는 코드 스캔으로 얻어 옴
		int64_t fee = 1;																		// 코드 스캔으로 얻어 옴

		// 자동 입력
		State state = State::SPENT;																// 유가증권 사용

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (myWallet.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : own to spent")) {	// sale to spent는?
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

		bc.findMyUTXOTable(myUTXOTable, myWallet.getPublicKeyHash(), State::OWN);
		myWallet.myUTXOTable = myUTXOTable;
		myWallet.printMyUTXOTable(cout);

		if (myWallet.myUTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}


		// 사용자 직접 입력
		cout << "Select : ";
		size_t n;																				// 선택한 유가증권 번호
		cin >> n;

		cout << "Value : ";
		int64_t value;																			// 소유한 유가증권을 상대방에게 얼마나 전송할 건지
		cin >> value;

		cout << "Transaction Fee : ";
		int64_t fee;																			// 사용자로부터 입력받거나 추천값 적용
		cin >> fee;


		// 자동 입력
		Type type = myWallet.myUTXOTable[n].output.type;												// 사용자 소유의 유가증권
		State state = myWallet.myUTXOTable[n].output.state;											// 어떤 상태로 바꿀지 입력받음

		// 코드 스캔
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// 받는 사람

		// inputPassword	// 비밀번호 확인

		Transaction tx;
		if (myWallet.createTransaction(tx, 1, recipientPublicKeyHash, type, value, fee, "GiftCard LoveShinak : send")) {
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

		if (future_block_produce.valid()) {
			if (future_block_produce.wait_for(chrono::nanoseconds(0)) == future_status::timeout) {
				cout << "Still producing the block...\n";
				goto HOME;
			}
			else {
				if (future_block_produce.get())
					cout << "Block producing suceed!\n";
				else
					cout << "Block producing fail...\n";
			}
		}

		//err = getBroadcastedTransaction(tx);

		// 사용자로부터 입력받음
		int txCount = MAX_TRANSACTION_COUNT;
		State feeState = State::OWN;

		future_block_produce = async(launch::async, &Blockchain::produceBlock, ref(bc), myWallet.getPublicKeyHash(), txCount, feeState);

		goto HOME;
	}


INFO: // 블록체인 정보 불러오기
	{
		bc.print(cout);

		if (bc.isValid())
			cout << "Valid blockchain!\n";
		else
			cout << "Invalid blockchain...\n";

		goto HOME;
	}

	cout << "Test complete!\n";
	system("pause");
	return 0;
}

bool is_number(const string& s) {
	return !s.empty() && find_if(s.begin(),
		s.end(), [](char c) { return !isdigit(c); }) == s.end();
}