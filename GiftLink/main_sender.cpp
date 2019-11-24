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


/* ���� �Ʒ� ������ ���Ѿ� ��.
1. UTXO Table ������Ʈ
2. �ŷ� ����
3. ä��

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

	vector<UTXO> myUTXOTable;		// �������� ���� �� ������ UTXO
	vector<UTXO> UTXOTable;			// �������� ���� ��� UTXO

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

ISSUE: // �������� ����
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

		// ����ڷκ��� �Է¹���
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

		// �ڵ� �Է�
		Type type(name, faceValue, marketValue, time(NULL) + expirationDate, myWallet.getPrivateKey());
		int txCount = 1;
		int64_t issueAmount = bc.getMaxIssueAmount(type);
		State securitiesState = State::OWN;
		State feeState = State::OWN;

		// inputPassword	// ��й�ȣ Ȯ��

		future_block_produce = async(launch::async, &Blockchain::issueSecurities, ref(bc), myWallet.getPublicKeyHash(), txCount, type, issueAmount, securitiesState, feeState);

		goto HOME;
	}


SELL: // �������� ���� ��ȯ(����<->�Ǹ�)
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

		// ����� ���� �Է�
		cout << "Select : ";
		size_t n;											// ���¸� ��ȯ�� �������� ��ȣ
		cin >> n;

		cout << "Value : ";
		int64_t value;										// ������ �������� �� �� ���� �Ǹ� ������ ��ȯ���� ����ڷκ��� �Է¹���
		cin >> value;

		cout << "Transaction Fee : ";
		int64_t fee;										// ����ڷκ��� �Է¹ްų� ��õ�� ����
		cin >> fee;


		// �ڵ� ��ĵ
		const uint8_t * ownerPublicKeyHash = myWallet.getPublicKeyHash();								// (�ڽ� �Ǵ� �޴� ���) �ڵ� ��ĵ

		// �ڵ� �Է�
		Type type = myWallet.myUTXOTable[n].output.type;												// ����� ������ ��������
		State state = myWallet.myUTXOTable[n].output.state == State::SALE ? State::OWN : State::SALE;	// ������ �������� ��ȣ�� �Ǵ�

		// inputPassword	// ��й�ȣ Ȯ��

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

	//**************************************�������� �Ǹ� ��Ȳ �˻� �ʿ� : '�������� ��ü �Ǹŷ� < �������� ���෮'�̾�� ���� ����
	//**************************************������ ���������� �ڱ� ���� �����ϰ� 
BUY: // �������� ����
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);

		bc.findUTXOTable(UTXOTable, State::SALE);
		vector<UTXO> saleSecuritiesTable;
		for (const UTXO utxo : UTXOTable) {
			if (utxo.output.type.expirationDate > time(NULL) && utxo.output.type != Type())	// �������� ��ȿ�Ⱓ üũ
				saleSecuritiesTable.push_back(utxo);
		}

		bc.findMyUTXOTable(myUTXOTable, myWallet.getPublicKeyHash());
		vector<UTXO> myGLCTable;
		for (const UTXO utxo : myUTXOTable) {
			if (utxo.output.type == Type() && utxo.output.state == State::OWN)					// �� ������ GLC
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

		// ����� ���� �Է�
		cout << "Select : ";
		size_t n;																			// ������ �������� ��ȣ
		cin >> n;

		cout << "Value : ";
		int64_t value;																		// �󸶳� ��������
		cin >> value;

		cout << "Transaction Fee : ";
		int64_t fee;																		//********************��� ������?? �Ǹ��ڰ� ����
		cin >> fee;

		// inputPassword	// ��й�ȣ Ȯ��

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


USE: // �������� ���
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);

		bc.findMyUTXOTable(myUTXOTable, myWallet.getPublicKeyHash(), State::OWN);

		vector<UTXO> mySecuritiesUTXOTable;		// �������� ���� �� ������ ��������(Own)
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

		// ����� ���� �Է�
		cout << "Select : ";
		size_t n;																				// ����� �������� ��ȣ
		cin >> n;

		cout << "Value : ";
		int64_t value;																			// �󸶳� ������� ����ڷκ��� �Է¹���
		cin >> value;


		// �ڵ� ��ĵ
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// �޴� ��� �Է� �Ǵ� �ڵ� ��ĵ
		Type type = myWallet.myUTXOTable[n].output.type;												// ����� ������ �������� �Ǵ� �ڵ� ��ĵ���� ��� ��
		int64_t fee = 1;																		// �ڵ� ��ĵ���� ��� ��

		// �ڵ� �Է�
		State state = State::SPENT;																// �������� ���

		// inputPassword	// ��й�ȣ Ȯ��

		Transaction tx;
		if (myWallet.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : own to spent")) {	// sale to spent��?
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


SEND: // ��������, ���� ����
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


		// ����� ���� �Է�
		cout << "Select : ";
		size_t n;																				// ������ �������� ��ȣ
		cin >> n;

		cout << "Value : ";
		int64_t value;																			// ������ ���������� ���濡�� �󸶳� ������ ����
		cin >> value;

		cout << "Transaction Fee : ";
		int64_t fee;																			// ����ڷκ��� �Է¹ްų� ��õ�� ����
		cin >> fee;


		// �ڵ� �Է�
		Type type = myWallet.myUTXOTable[n].output.type;												// ����� ������ ��������
		State state = myWallet.myUTXOTable[n].output.state;											// � ���·� �ٲ��� �Է¹���

		// �ڵ� ��ĵ
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// �޴� ���

		// inputPassword	// ��й�ȣ Ȯ��

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


PRODUCE: // ��� ����
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

		// ����ڷκ��� �Է¹���
		int txCount = MAX_TRANSACTION_COUNT;
		State feeState = State::OWN;

		future_block_produce = async(launch::async, &Blockchain::produceBlock, ref(bc), myWallet.getPublicKeyHash(), txCount, feeState);

		goto HOME;
	}


INFO: // ���ü�� ���� �ҷ�����
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