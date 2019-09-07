#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include "Blockchain.h"
using namespace std;

int main()
{
	Wallet recipient;
	cout << "Test recipient wallet was created\n\n";

	Wallet w;
	cout << "My wallet was created\n\n";

	Blockchain bc("GiftLink Blockchain", w.getPublicKeyHash());
	cout << bc.getName() << " was created\n\n";
	
	vector<UTXO> myUTXO;
	bc.findMyUTXOTable(myUTXO, w.getPublicKeyHash());
	w.setUTXOTable(myUTXO);
	
	
	Transaction tx;
	if (w.createTransaction(tx, 1, recipient.getPublicKeyHash(), Type::GLC, 10, 1, "from w to recipient 1000")) {
		cout << "Transaction was created!\n";
	}
	else {
		cout << "No balance....\n";
	}
	
	bc.addTransactionToPool(tx);
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