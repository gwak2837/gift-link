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


/* ���� �Ʒ� ������ ���Ѿ� ��.
1. UTXO Table ������Ʈ
2. �ŷ� ����
3. ä��

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

	vector<UTXO> myUTXOTable;		// �������� ���� �� ������ UTXO
	vector<UTXO> UTXOTable;			// �������� ���� ��� UTXO

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

ISSUE: // �������� ����
	{
		system("pause");
		system("cls");

		// ����ڷκ��� �Է¹���
		Type type("GiftCard LoveShinak", 10000, 9000, time(NULL) + 8640000);
		int txCount = 1;
		int issueAmount = COINBASE_REWARD * 100;
		State securitiesState = State::own;
		State feeState = State::own;

		// inputPassword	// ��й�ȣ Ȯ��

		if (bc.issueSecurities(w.getPublicKeyHash(), txCount, type, issueAmount, securitiesState, feeState))
			cout << "Security was issued!\n";
		else
			cout << "Issuing security failed...\n";

		// broadcastBlockchain(bc);

		goto HOME;
	}


STATE_TRANSITION: // �������� ���� ��ȯ(����<->�Ǹ�)
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

		// ����ڷκ��� �Է¹���
		size_t n = 0;																			// ������ �������� ��ȣ
		int64_t value = 10;																		// ������ �������� �� �� ���� �Ǹ� ������ ��ȯ���� ����ڷκ��� �Է¹���
		int64_t fee = 1;																		// ����ڷκ��� �Է¹ްų� ��õ�� ����

		// �ڵ� ��ĵ
		const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();								// (�ڽ� �Ǵ� �޴� ���) �ڵ� ��ĵ

		// �ڵ� �Է�
		Type type = w.myUTXOTable[n].output.type;												// ����� ������ ��������
		State state = w.myUTXOTable[n].output.state == State::sale ? State::own : State::sale;	// ������ �������� ��ȣ�� �Ǵ�

		// inputPassword	// ��й�ȣ Ȯ��

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
BUY: // �������� ����
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findUTXOTable(UTXOTable, State::sale);												
		vector<UTXO> saleSecuritiesTable;
		for (const UTXO utxo : UTXOTable) {
			if (utxo.output.type.expirataionDate > time(NULL) && utxo.output.type != Type())	// �������� ��ȿ�Ⱓ üũ
				saleSecuritiesTable.push_back(utxo);
		}

		w.UTXOTable = saleSecuritiesTable;
		w.printUTXOTable(cout);

		if (w.UTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		size_t n = 0;																			// ������ �������� ��ȣ
		const uint8_t * recipientPublicKeyHash = w.UTXOTable[n].output.recipientPublicKeyHash;	// �Ǹ����� UTXO���� ��� ��
		Type type = w.UTXOTable[n].output.type;													// �Ǹ��� ������ �������� Type
		int64_t fee = 1;																		// ����ڷκ��� �Է¹ްų� ��õ�� ����

		// inputPassword	// ��й�ȣ Ȯ��

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

USE: // �������� ���
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash(), State::own);

		vector<UTXO> mySecuritiesUTXOTable;		// �������� ���� �� ������ ��������(Own)
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

		// ����� ���� �Է�
		size_t n = 0;																			// ������ �������� ��ȣ
		int64_t value = 99;																		// �󸶳� ������� ����ڷκ��� �Է¹���

		// �ڵ� ��ĵ
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// �޴� ��� �Է� �Ǵ� �ڵ� ��ĵ
		Type type = w.myUTXOTable[n].output.type;												// ����� ������ �������� �Ǵ� �ڵ� ��ĵ���� ��� ��
		int64_t fee = 1;																		// �ڵ� ��ĵ���� ��� ��

		// �ڵ� �Է�
		State state = State::spent;																// �������� ���

		// inputPassword	// ��й�ȣ Ȯ��

		Transaction tx;
		if (w.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, 99, fee, state, "GiftCard LoveShinak : own to spent")) {	// sale to spent��?
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
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash(), State::own);
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		if (w.myUTXOTable.size() == 0) {
			cout << "No balance...\n";
			goto HOME;
		}

		// ����� ���� �Է�
		size_t n = 0;																			// ������ �������� ��ȣ
		Type type = w.myUTXOTable[n].output.type;												// ����� ������ ��������
		int64_t value = 10;																		// ������ ���������� ���濡�� �󸶳� ������ ����
		int64_t fee = 1;																		// ����ڷκ��� �Է¹ްų� ��õ�� ����
		State state = w.myUTXOTable[n].output.state;											// � ���·� �ٲ��� �Է¹���

		// �ڵ� ��ĵ
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// �޴� ���

		// inputPassword	// ��й�ȣ Ȯ��

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


PRODUCE: // ��� ����
	{
		system("pause");
		system("cls");

		//err = getBroadcastedTransaction(tx);

		// ����ڷκ��� �Է¹���
		int txCount = MAX_TRANSACTION_COUNT;
		State feeState = State::own;

		if (bc.produceBlock(w.getPublicKeyHash(), txCount, feeState))
			cout << "Block was produced!\n";
		else
			cout << "Block producing fail...\n";

		//broadcastBlockchain(bc);

		goto HOME;
	}


INFO: // ���ü�� ���� �ҷ�����
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