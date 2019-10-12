#include <iostream>
#include <fstream>
#include <cstdlib>
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

int main()
{
	Wallet recipient;
	cout << "Test recipient wallet was created\n\n";

	Wallet w;
	cout << "My wallet was created\n\n";

	Blockchain bc("GiftLink Blockchain", w.getPublicKeyHash());
	cout << bc.getName() << " was created\n\n";

	vector<UTXO> myUTXOTable;		// �������� ���� �� ������ ��������(Own + Sale + Spent)
	vector<UTXO> UTXOTable;			// �������� ���� ��� ��������

	Transaction tx;

	//err = getBroadcastedTransaction(tx)
	//if (bc.addTransactionToPool(tx))
	//	cout << "Transaction was added to Tx Pool!\n";
	//else
	//	cout << "Adding Tx to Tx Pool failed...\n";

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
	}

	
STATE_TRANSITION: // �������� ���� ��ȯ(����<->�Ǹ�)
	{
		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		// ����ڷκ��� �Է¹���
		size_t n = 0;																			// ������ �������� ��ȣ
		int64_t value = 99;																		// ������ �������� �� �� ���� �Ǹ� ������ ��ȯ���� ����ڷκ��� �Է¹���
		int64_t fee = 1;																		// ����ڷκ��� �Է¹ްų� ��õ�� ����

		// �ڵ� ��ĵ
		const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();								// (�ڽ� �Ǵ� �޴� ���) �ڵ� ��ĵ

		// �ڵ� ���
		Type type = myUTXOTable[n].output.type;													// ����� ������ ��������
		State state = myUTXOTable[n].output.state == State::sale ? State::own : State::sale;	// ������ �������� ��ȣ�� �Ǵ�

		// inputPassword	// ��й�ȣ Ȯ��

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
BUY: // �������� ����
	{
		//err = getBroadcastedBlockchain(bc);
		bc.findUTXOTable(UTXOTable, State::sale);												// �������� ��ȿ�Ⱓ üũ
		w.UTXOTable = UTXOTable;
		w.printUTXOTable(cout);

		size_t n = 0;																			// ������ �������� ��ȣ
		const uint8_t * recipientPublicKeyHash = UTXOTable[n].output.recipientPublicKeyHash;	// �Ǹ����� UTXO���� ��� ��
		Type type = UTXOTable[n].output.type;													// �Ǹ��� ������ �������� Type
		int64_t fee = 1;																		// ����ڷκ��� �Է¹ްų� ��õ�� ����

		// inputPassword	// ��й�ȣ Ȯ��

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
	
	
USE: // �������� ���
	{
		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		// ����� ���� �Է�
		size_t n = 0;																			// ������ �������� ��ȣ
		int64_t value = 99;																		// �󸶳� ������� ����ڷκ��� �Է¹���

		// �ڵ� ��ĵ
		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();					// �޴� ��� �Է� �Ǵ� �ڵ� ��ĵ
		Type type = myUTXOTable[n].output.type;													// ����� ������ �������� �Ǵ� �ڵ� ��ĵ���� ��� ��
		int64_t fee = 1;																		// �ڵ� ��ĵ���� ��� ��

		// �ڵ� ���
		State state = State::spent;																// �������� ���

		// inputPassword	// ��й�ȣ Ȯ��

		if (w.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, 99, fee, state, "GiftCard LoveShinak : own to spent"))	// sape to spent��?
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool fail...\n";
		//broadcastTransaction(tx);
	}
	

SEND: // �������� ����
	{
		system("pause");
		system("cls");

		//err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXOTable, w.getPublicKeyHash());
		w.myUTXOTable = myUTXOTable;
		w.printMyUTXOTable(cout);

		//size_t n = 0;																			// ������ �������� ��ȣ
		//const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();
		//Type type = myUTXOTable[n].output.type;													// ����� ������ ��������
		//int64_t value = 99;																		// ������ �������� �� �� ���� �Ǹ� ������ ��ȯ���� ����ڷκ��� �Է¹���
		//int64_t fee = 1;																		// ����ڷκ��� �Է¹ްų� ��õ�� ����
		//State state = myUTXOTable[n].output.state == State::sale ? State::own : State::sale;		// ����ڷκ��� � ���·� �ٲ��� �Է¹���

		// inputPassword	// ��й�ȣ Ȯ��

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
	}


INFO: // ���ü�� ���� �ҷ�����
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