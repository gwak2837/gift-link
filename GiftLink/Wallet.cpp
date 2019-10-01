#include <vector>
#include <string>
#include <cstring>
#include <algorithm>          // sort 사용 위해 필요
#include "Wallet.h"
#include "Transaction.h"
#include "uECC.h"
#include "Utility.h"
using namespace std;

Wallet::Wallet() {
	if (uECC_make_key(publicKey, privateKey, curve)) {
		SHA256_Encrpyt(publicKey, SECP256R1_NUM_BYTES * 2, publicKeyHash);
		uECC_compress(publicKey, compressedPublicKey, curve);
	}
	else
		cout << "An error occurred when generating the key pair...\n";
}

Wallet::Wallet(uint8_t * _privateKey) {
	memcpy(privateKey, _privateKey, SECP256R1_NUM_BYTES);

	if (uECC_compute_public_key(privateKey, publicKey, curve)) {
		SHA256_Encrpyt(publicKey, SECP256R1_NUM_BYTES * 2, publicKeyHash);
		uECC_compress(publicKey, compressedPublicKey, curve);
	}
	else
		cout << "An error occurred when computing the public key from the private key...\n";
}


/* 거래를 생성함.
추후에 상품권 사는 것과 자신의 GLC를 보내는 것을 동시에 한 거래에 담는 기능을 구현할 예정임.

Transaction 생성 과정
1. Input 설정
2. Output 설정
3. Transaction 데이터 해싱 
4. Transaction의 모든 Input에 서명 */
bool Wallet::createTransaction(Transaction & _tx, int blockchainVersion, const uint8_t * _recipientPublicKeyHash, Type & _type,
	int64_t value, int64_t fee, string memo) {

	if (getMyUTXOAmount(_type) < value + fee) {
		cout << "There are not enough value of mine...\n";
		return false;
	}

	vector<Input> inputs;
	int64_t inputSum = 0;
	// 자신의 UTXO를 Greedy 방법으로 선택
	// input.signature에 참조할 UTXO의 pubKeyHash(32 byte) 입력 
	for (UTXO & utxo : myUTXOTable) {
		if (value + fee > inputSum) {
			Input input(utxo.output.recipientPublicKeyHash, publicKey, utxo.txHash, utxo.outputIndex, utxo.blockHeight);
			inputs.push_back(input);
			inputSum += utxo.output.value;
		}
	}

	vector<Output> outputs;
	Output output(_recipientPublicKeyHash, value, _type);
	outputs.push_back(output);

	if (value + fee != inputSum) {
		Output output2(publicKeyHash, inputSum - value - fee, _type);			// 거스름돈은 자신의 주소로 보냄
		outputs.push_back(output2);
	}
		
	Transaction tx(inputs, outputs, blockchainVersion, memo);	// Tx 데이터 추출 후 Tx의 txHash 계산

	for (Input & input : tx.inputs) { 
		if (uECC_sign(privateKey, tx.txHash, sizeof(tx.txHash), input.signature, curve) == 0)
			cout << "An error occurred when generating the signature...\n";
	}

	_tx = tx;
	return true;
}

int64_t Wallet::getMyUTXOAmount(Type _type) const {
	if (myUTXOTable.size() == 0)
		return 0;
	
	int64_t sum = 0;
	for (const UTXO & utxo : myUTXOTable) {
		if (utxo.output.type == _type) {
			sum += utxo.output.value;
		}
	}

	return sum;
}

void Wallet::setUTXOTable(std::vector<UTXO> & _UTXOTable) {
	for (const UTXO & utxo : _UTXOTable) {
		if (!isMemoryEqual(publicKeyHash, utxo.output.recipientPublicKeyHash, sizeof(publicKeyHash))) {
			UTXOTable = _UTXOTable;
			sort(UTXOTable.begin(), UTXOTable.end());
			break;
		}
	}
	myUTXOTable = _UTXOTable;
	sort(myUTXOTable.begin(), myUTXOTable.end());
}

void Wallet::printMyUTXOTable(std::ostream & o) const {
	o << "----- My UTXO Table -----\n";
	for (const UTXO & utxo : myUTXOTable) {
		utxo.print(o);
		o << '\n';
	}
}
