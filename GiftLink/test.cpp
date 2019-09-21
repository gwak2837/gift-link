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
*/

int main()
{
	Wallet recipient;
	cout << "Test recipient wallet was created\n\n";

	Wallet w;
	cout << "My wallet was created\n\n";

	Blockchain bc("GiftLink Blockchain", w.getPublicKeyHash());
	cout << bc.getName() << " was created\n\n";
	
	vector<UTXO> myUTXO;
	Transaction tx;

	bc.findMyUTXOTable(myUTXO, w.getPublicKeyHash());
	w.setUTXOTable(myUTXO);
	if (w.createTransaction(tx, 1, recipient.getPublicKeyHash(), Type::GLC, 1, 1, "from w to recipient 1000")) {
		bc.addTransactionToPool(tx);
		cout << "Transaction was created!\n";
	}
	else
		cout << "No balance....\n";
	bc.produceBlock(w.getPublicKeyHash());
	
	bc.findMyUTXOTable(myUTXO, w.getPublicKeyHash());
	w.setUTXOTable(myUTXO);
	if (w.createTransaction(tx, 1, recipient.getPublicKeyHash(), Type::GLC, 2, 2, "from w to recipient 1000")) {
		bc.addTransactionToPool(tx);
		cout << "Transaction was created!\n";
	}
	else
		cout << "No balance....\n";
	bc.produceBlock(w.getPublicKeyHash());
	
	bc.findMyUTXOTable(myUTXO, w.getPublicKeyHash());
	w.setUTXOTable(myUTXO);
	if (w.createTransaction(tx, 1, recipient.getPublicKeyHash(), Type::GLC, 3, 3, "from w to recipient 1000")) {
		bc.addTransactionToPool(tx);
		cout << "Transaction was created!\n";
	}
	else
		cout << "No balance....\n";
	bc.produceBlock(w.getPublicKeyHash());



	if (bc.isValid())
		cout << "Valid blockchain!\n";
	else
		cout << "Invalid blockchain...\n";

	bc.print(cout);

	cout << "Test complete!\n";
	system("pause");
	return 0;
}