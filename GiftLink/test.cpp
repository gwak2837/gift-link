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

	vector<UTXO> myUTXO;		// �������� ���� �� ������ ��������(Own + Sale + Spent)
	vector<UTXO> myOwnUTXO;		// �������� ���� �� ������ ���� ����(Own) ��������
	vector<UTXO> mySaleUTXO;	// �������� ���� �� ������ �Ǹ� ����(Sale) ��������
	vector<UTXO> mySpentUTXO;	// �������� ���� �� ������ ��� ��(Spent) ��������

	vector<UTXO> saleUTXO;		// ������ ���� ��� ��������

	Transaction tx;


ISSUE: /* �������� ���� */
	{
		// err = getBroadcastedTransaction(tx);

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool failed...\n";

		Type type("GiftCard LoveShinak", 10000, 9000, time(NULL) + 8640000);		// ����ڷκ��� �Է¹���. ��ȿ�Ⱓ 100��.

		// inputPassword	// ��й�ȣ Ȯ��

		if (bc.issueSecurity(w.getPublicKeyHash(), type))							// ���෮�� �������� ä�� ������ * 100����
			cout << "Security was issued!\n";
		else
			cout << "Issuing security failed...\n";

		// broadcastBlockchain(bc);
	}
	

STATE_TRANSITION: /* �������� ���� ��ȯ(����->�Ǹ�) */
	{
		// err = getBroadcastedBlockchain(bc);
		bc.findMyUTXO(myUTXO, w.getPublicKeyHash());
		w.setMyUTXOTable(myUTXO);
		w.printMyUTXOTable(cout);

		size_t n = 0;													// ������ �������� ��ȣ
		const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();
		Type type = myUTXO[n].output.type;								// ����� ������ ��������
		int64_t value = 99;												// ������ �������� �� �� ���� �Ǹ� ������ ��ȯ���� ����ڷκ��� �Է¹���
		int64_t fee = 1;												// ����ڷκ��� �Է¹ްų� ��õ�� ����
		State state = State::sale;										// ����ڷκ��� � ���·� �ٲ��� �Է¹���

		// inputPassword	// ��й�ȣ Ȯ��

		if (w.createTransactionSwitchState(tx, 1, ownerPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : own to sale"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		// broadcastTransaction(tx);
	}

BUY: /* �������� ���� */
	{
		// err = getBroadcastedBlockchain(bc);
		bc.findSaleUTXOTable(saleUTXO);														// �������� ��ȿ�Ⱓ üũ
		w.setUTXOTable(saleUTXO);
		w.printSaleUTXOTable(cout);

		size_t n = 0;																		// ������ �������� ��ȣ
		const uint8_t * recipientPublicKeyHash = saleUTXO[n].output.recipientPublicKeyHash;	// �Ǹ����� UTXO���� ��� ��
		Type type = saleUTXO[n].output.type;												// �Ǹ��� ������ �������� Type
		int64_t fee = 1;																	// ����ڷκ��� �Է¹ްų� ��õ�� ����

		// inputPassword	// ��й�ȣ Ȯ��

		if (w.createTransactionPurchaseSale(tx, 1, recipientPublicKeyHash, type, 99, fee, "GiftCard LoveShinak : P2P Trade"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		// broadcastTransaction(tx);
	}


USE: /* �������� ��� */
	{
		// err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXO, w.getPublicKeyHash());
		w.setUTXOTable(myUTXO);
		w.printMyUTXOTable(cout);

		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();	// QR�ڵ�(���ڵ�)���� ��� ��
		int64_t fee = 1;														// QR�ڵ�(���ڵ�)���� ��� ��
		Type type("GiftCard LoveShinak", 10000, 9000, time(NULL));				// QR�ڵ�(���ڵ�)���� ��� ��. ��ȿ�Ⱓ Ȯ��.
																				// QR�ڵ�(���ڵ�)�� Type ��ð� ������ ����ڰ� ����
		State state = State::spent;

		// inputPassword	// ��й�ȣ Ȯ��

		if (w.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, 99, fee, state, "GiftCard LoveShinak : own to spent"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		// broadcastTransaction(tx);
	}

SEND: /* �������� ���� */
	{}



PRODUCE: /* ��� ���� */
	{
		// err = getBroadcastedTransaction(tx);

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool fail...\n";

		if (bc.produceBlock(w.getPublicKeyHash()))
			cout << "Block was produced!\n";
		else
			cout << "Block producing fail...\n";

		// broadcastBlockchain(bc);
	}


INFO: /* ���ü�� ���� �ҷ����� */
	{}

	if (bc.isValid())
		cout << "Valid blockchain!\n";
	else
		cout << "Invalid blockchain...\n";

	bc.print(cout);

	cout << "Test complete!\n";
	system("pause");
	return 0;
}