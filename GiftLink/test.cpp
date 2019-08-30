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
	bc.print(cout);

	vector<Transaction> vtx;
	queue<Transaction> qtx;


	{
		Transaction tx = w.createTransaction(1, recipient.getPublicKeyHash(), Type::GLC, 1000, 1, "from w to recipient 1000");
		tx.print(cout);
		qtx.push(tx);

	}

	vtx.push_back(qtx.front());
	vtx[0].print(cout);


	cout << "Test complete!\n";
	system("pause");
	return 0;
}