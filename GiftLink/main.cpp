#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>		// isdigit()
#include "Blockchain.h"
using namespace std;

bool is_number(const string& s);


int main()
{
	
	Wallet recipient;
	cout << "Test recipient wallet was created\n\n";

	cout << "Input password: ";
	string password;
	cin >> password;
	Wallet w;
	cout << "My wallet was created\n\n";

	Blockchain bc("GiftLink", w.getPublicKeyHash());
	cout << bc.getName() << " was created\n\n";
	

	
HOME://complete
	{
		system("pause");
		system("cls");

		string input;
		cout << "0.Home    1.Buy    2.Mining    3.Blockchain    4.My Wallet    \n";
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
			goto BUY;
		case 2:
			goto MINING;
		case 3:
			goto BLOCKCHAIN;
		case 4:
			goto MY_WALLET;
		default:
			cout << "Invalid number...\n";
			goto HOME;
		}
	}

BUY:
	{
		system("pause");
		system("cls");

		cout << "0.Back    List    \n";


		string input;
		cin >> input;

		if (!is_number(input)) {
			cout << "Invalid number...\n";
			goto BUY;
		}

		unsigned int select;
		select = atoi(input.c_str());

		if (select == 0) {
			goto HOME;
		}
		
		else if (select > giftcardList.size()) {
			cout << "invalid number...\n";
			goto BUY;
		}

		Transaction * tx = recipient.createTransaction(w.getPublicKey(), giftcardList[select-1]->getName(), giftcardList[select-1]->getFaceValue(), 0, "Buy");
		Transaction * tx2 = w.createTransaction(recipient.getPublicKey(), "coin", giftcardList[select-1]->getMarketValue(), 0, "Buy");

		if (tx == NULL) {
			cout << "Seller have no value of the giftcard you select...\n";
		}
		if (tx2 == NULL) {
			cout << "You have no balance of the coin for paying...\n";
		}
		
		if (tx->isValid(w.getPrivateKey()) && bc.isUTXO(tx)) {
			if (tx2->isValid(w.getPrivateKey()) && bc.isUTXO(tx2)) {
				bc.addTransactionToPool(tx);
				bc.addTransactionToPool(tx2);
				cout << "Valid transaction!\n";
			}
		}
		else {
			delete tx;
			delete tx2;
			cout << "Invalid transaction...\n";
		}
		
		goto HOME;
	}

MINING://complete
	{
		system("pause");
		system("cls");

		bc.produceBlock(&w);
		goto HOME;
	}

BLOCKCHAIN://complete
	{
		system("pause");
		system("cls");

		string input;
		cout << "0.Back    1.Print Blockchain    2.Save Blockchain    3.Load Blockchain    \n";
		cin >> input;

		if (!is_number(input)) {
			cout << "Invalid number...\n";
			goto BLOCKCHAIN;
		}

		int select;
		select = atoi(input.c_str());

		switch (select) {
		case 0:
			goto HOME;
		case 1:
			goto PRINT;
		case 2:
			goto SAVE;
		case 3:
			goto LOAD;
		default:
			cout << "Invalid number...\n";
			goto BLOCKCHAIN;
		}

		goto HOME;
	}

PRINT://complete
	{
		system("pause");
		system("cls");

		bc.print(cout);
		goto BLOCKCHAIN;
	}

SAVE://complete
	{
		system("pause");
		system("cls");

		string input;
		cout << "0.Back    1.Default path    2.User defined path    \n";
		cin >> input;

		if (!is_number(input)) {
			cout << "Invalid number...\n";
			goto SAVE;
		}

		int select;
		select = atoi(input.c_str());

		ofstream fout;
		string path;

		switch (select) {
		case 0:
			goto BLOCKCHAIN;
		case 1:
			fout.open(bc.getFileName());
			if (!fout.is_open()) {
				cout << "File I/O error...\n";
				break;
			}
			bc.print(fout);
			cout << "File writing was completed!";
			break;
		case 2:
			cout << "File path: ";
			cin >> path;
			fout.open(path);
			if (!fout.is_open()) {
				cout << "File I/O error...\n";
				break;
			}
			bc.print(fout);
			cout << "File writing was completed!";
			break;
		default:
			cout << "Invalid number...\n";
			goto SAVE;
		}
		goto BLOCKCHAIN;
	}

LOAD:
	{
		system("pause");
		system("cls");
		
		cout << "Is developing\n";

		goto BLOCKCHAIN;
	}

MY_WALLET://complete
	{
		system("pause");
		system("cls");

		string input;
		cout << "0.Back    1.Send    2.Sell    3.Issue    4.My UTXO\n";
		cin >> input;

		if (!is_number(input)) {
			cout << "Invalid number...\n";
			goto MY_WALLET;
		}

		int select;
		select = atoi(input.c_str());

		switch (select) {
		case 0:
			goto HOME;
		case 1:
			goto SEND;
		case 2:
			goto SELL;
		case 3:
			goto ISSUE;
		case 4:
			goto MY_UTXO;
		default:
			cout << "Invalid number...\n";
			goto MY_WALLET;
		}
	}

SEND:
	{
		system("pause");
		system("cls");


		
		string propertyType, memo;
		uint64_t sendingAmount, fee;
		cout << "Which property(string): ";
		cin >> propertyType;

		vector<UTXO> utxo = bc.findMyUTXOTable(recipient.getPrivateKey(), propertyType);
		recipient.setMyUTXOTable(utxo);

		cout << "Your property: " << recipient.getMyUTXOAmount(propertyType) << '\n';
		cout << "Receiver's public key: " << recipient.getPublicKey() << '\n';
		cout << "Sending value(integer): ";
		cin >> sendingAmount;
		cout << "Transaction fee(integer): ";
		cin >> fee;
		cout << "Memo(string): ";
		cin >> memo;

		Transaction * tx_ = w.createTransaction(recipient.getPublicKey(), propertyType, sendingAmount, fee, memo);

		if (tx_ == NULL) {
			cout << "You have no property of the propertyType you input...\n";
		}
		else if (tx_->isValid(w.getPrivateKey()) && bc.isUTXO(tx_)) {
			bc.addTransactionToPool(tx_);
			cout << "Valid transaction!\n";
		}
		else {
			delete tx_;
			cout << "Invalid transaction...\n";
		}
		
		goto MY_WALLET;
	}

SELL:
	{

	}

ISSUE:
	{
		system("pause");
		system("cls");

		w.myIssuableGiftcardTable = bc.getIssuableGiftcardTable(w.getPrivateKey());

		cout << "Issuable Gift Card List    0.Back    \n";
		int i = 1;
		for (UTXO * utxo : w.myIssuableGiftcardTable) {
			cout << i << ".";
			utxo->print(cout);
			i++;
		}
		string input;
		cin >> input;

		if (!is_number(input)) {
			cout << "Invalid number...\n";
			goto ISSUE;
		}

		unsigned int select;
		select = atoi(input.c_str());

		if (select == 0) {
			goto MY_WALLET;
		}
		else if (select > w.myIssuableGiftcardTable.size()) {
			cout << "invalid number...\n";
			goto ISSUE;
		}


	}

MY_UTXO:
	{
		system("pause");
		system("cls");


		
		vector<UTXO> myUTXOTable = bc.findMyUTXOTable(w.getPrivateKey());
		w.setMyUTXOTable(myUTXOTable);
		w.printMyUTXOTable();
		
		goto MY_WALLET;
	}

	
	bc.setVersion("Giftcard Blockchain 1.1");

	bc.printAllBlockHash();
	bc.printAllMerkleHash();
	bc.printAllTransaction(cout);
	bc.printWaitingBlock();

	bc.print(cout);				// Blockchain을 txt 파일로 저장(출력)
	bc.loadBlockchain();



	//---------------------- 발행된 상품권 조회 ----------------------
	/* 사용자가 블록체인 노드에게 UTXO Table 요청 */
	vector<UTXO> utxoTable = bc.findUTXOTable();
	recipient.setUTXOTable(utxoTable);
	recipient.printUTXOTable();
	

	//---------------------- 상품권 발행 및 판매 ----------------------

	/* 판매자의 첫 거래 생성 */
	Transaction * tx = w2.createCoinbaseTransaction(new Giftcard("Giftcard Name"), 100, 0, "Memo"); // 상품권 종류, 보낼 금액, 수수료, 메모

	/* 판매자가 블록체인 노드에 거래와 개인키 전송 */
	if (tx->isValidCoinbase()) {
		bc.push(tx);

		/* 판매자에게 메시지 전송 */
		cout << "상품권 발행 성공!\n";		
	}
	else {
		/* 판매자에게 메시지 전송 */
		cout << "Invalid transaction data...\n";
	}
	

	//---------------------- 상품권 거래 ----------------------
	/* 상품권 거래 생성 */
	Transaction * tx2 = w2.createTransaction(recipient.getPublicKey(), new Giftcard("Giftcard Name"), 1, 0, "Memo"); // 받는 사람, 상품권 종류, 보낼 금액, 수수료, 메모 

	/* 블록체인 노드에 거래 전송 */
	if (tx2->isValid(w2.getPrivateKey()), bc.isUTXO(tx2)) {
		bc.push(tx2);

		/* w2에게 메시지 전송 */
		cout << "판매 성공!\n";

		/* recipient에게 메시지 전송 */
		cout << "구매 성공!\n";
	}
	else {
		/* 판매자에게 메시지 전송 */
		cout << "Invalid transaction data...\n";
	}

	serve_forever("4000");

	system("pause");
	return 0;
} 



bool is_number(const string& s)
{
	return !s.empty() && find_if(s.begin(),
		s.end(), [](char c) { return !isdigit(c); }) == s.end();
}

//void route()
//{
//    ROUTE_START()
//
//    ROUTE_GET("/")
//    {
//        printf("HTTP/1.1 200 OK\r\n\r\n");
//        printf("Hello! You are using %s", request_header("User-Agent"));
//    }
//
//    ROUTE_POST("/")
//    {
//        printf("HTTP/1.1 200 OK\r\n\r\n");
//        printf("Wow, seems that you POSTed %d bytes. \r\n", payload_size);
//        printf("Fetch the data using `payload` variable.");
//    }
//
//    ROUTE_END()
//}
