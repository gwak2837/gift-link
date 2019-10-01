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

	vector<UTXO> myUTXO;		// 참조되지 않은 내 소유의 유가증권(Own + Sale + Spent)
	vector<UTXO> myOwnUTXO;		// 참조되지 않은 내 소유의 소유 중인(Own) 유가증권
	vector<UTXO> mySaleUTXO;	// 참조되지 않은 내 소유의 판매 중인(Sale) 유가증권
	vector<UTXO> mySpentUTXO;	// 참조되지 않은 내 소유의 사용 된(Spent) 유가증권

	vector<UTXO> saleUTXO;		// 사용되지 않은 모든 유가증권

	Transaction tx;


ISSUE: /* 유가증권 발행 */
	{
		// err = getBroadcastedTransaction(tx);

		if (bc.addTransactionToPool(tx))
			cout << "Transaction was added to Tx Pool!\n";
		else
			cout << "Adding Tx to Tx Pool failed...\n";

		Type type("GiftCard LoveShinak", 10000, 9000, time(NULL) + 8640000);		// 사용자로부터 입력받음. 유효기간 100일.

		// inputPassword	// 비밀번호 확인

		if (bc.issueSecurity(w.getPublicKeyHash(), type))							// 발행량을 기준으로 채굴 수수료 * 100까지
			cout << "Security was issued!\n";
		else
			cout << "Issuing security failed...\n";

		// broadcastBlockchain(bc);
	}
	

STATE_TRANSITION: /* 유가증권 상태 전환(소유->판매) */
	{
		// err = getBroadcastedBlockchain(bc);
		bc.findMyUTXO(myUTXO, w.getPublicKeyHash());
		w.setMyUTXOTable(myUTXO);
		w.printMyUTXOTable(cout);

		size_t n = 0;													// 선택한 유가증권 번호
		const uint8_t * ownerPublicKeyHash = w.getPublicKeyHash();
		Type type = myUTXO[n].output.type;								// 사용자 소유의 유가증권
		int64_t value = 99;												// 소유한 유가증권 중 몇 개를 판매 중으로 전환할지 사용자로부터 입력받음
		int64_t fee = 1;												// 사용자로부터 입력받거나 추천값 적용
		State state = State::sale;										// 사용자로부터 어떤 상태로 바꿀지 입력받음

		// inputPassword	// 비밀번호 확인

		if (w.createTransactionSwitchState(tx, 1, ownerPublicKeyHash, type, value, fee, state, "GiftCard LoveShinak : own to sale"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		// broadcastTransaction(tx);
	}

BUY: /* 유가증권 구매 */
	{
		// err = getBroadcastedBlockchain(bc);
		bc.findSaleUTXOTable(saleUTXO);														// 유가증권 유효기간 체크
		w.setUTXOTable(saleUTXO);
		w.printSaleUTXOTable(cout);

		size_t n = 0;																		// 구매할 유가증권 번호
		const uint8_t * recipientPublicKeyHash = saleUTXO[n].output.recipientPublicKeyHash;	// 판매자의 UTXO에서 얻어 옴
		Type type = saleUTXO[n].output.type;												// 판매자 소유의 유가증권 Type
		int64_t fee = 1;																	// 사용자로부터 입력받거나 추천값 적용

		// inputPassword	// 비밀번호 확인

		if (w.createTransactionPurchaseSale(tx, 1, recipientPublicKeyHash, type, 99, fee, "GiftCard LoveShinak : P2P Trade"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		// broadcastTransaction(tx);
	}


USE: /* 유가증권 사용 */
	{
		// err = getBroadcastedBlockchain(bc);
		bc.findMyUTXOTable(myUTXO, w.getPublicKeyHash());
		w.setUTXOTable(myUTXO);
		w.printMyUTXOTable(cout);

		const uint8_t * recipientPublicKeyHash = recipient.getPublicKeyHash();	// QR코드(바코드)에서 얻어 옴
		int64_t fee = 1;														// QR코드(바코드)에서 얻어 옴
		Type type("GiftCard LoveShinak", 10000, 9000, time(NULL));				// QR코드(바코드)에서 얻어 옴. 유효기간 확인.
																				// QR코드(바코드)에 Type 명시가 없으면 사용자가 선택
		State state = State::spent;

		// inputPassword	// 비밀번호 확인

		if (w.createTransactionSwitchState(tx, 1, recipientPublicKeyHash, type, 99, fee, state, "GiftCard LoveShinak : own to spent"))
			cout << "Transaction was created!\n";
		else
			cout << "No balance....\n";

		// broadcastTransaction(tx);
	}

SEND: /* 유가증권 전송 */
	{}



PRODUCE: /* 블록 생성 */
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


INFO: /* 블록체인 정보 불러오기 */
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