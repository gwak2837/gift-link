#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>          // sort ��� ���� �ʿ�
#include "Wallet.h"
#include "Transaction.h"
#include "TransactionBroadcaster.h"
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

Wallet::Wallet(const uint8_t * _privateKey) {
	memcpy(privateKey, _privateKey, SECP256R1_NUM_BYTES);

	if (uECC_compute_public_key(privateKey, publicKey, curve)) {
		SHA256_Encrpyt(publicKey, SECP256R1_NUM_BYTES * 2, publicKeyHash);
		uECC_compress(publicKey, compressedPublicKey, curve);
	}
	else
		cout << "An error occurred when computing the public key from the private key...\n";
}


/* �ŷ��� ������.
���Ŀ� ��ǰ�� ��� �Ͱ� �ڽ��� GLC�� ������ ���� ���ÿ� �� �ŷ��� ��� ����� ������ ������.

Transaction ���� ����
1. Input ����
2. Output ����
3. Transaction ������ �ؽ� 
4. Transaction�� ��� Input�� ���� */
bool Wallet::createTransaction(Transaction & tx, int blockchainVersion, const uint8_t * _recipientPublicKeyHash, Type & type,
	int64_t value, int64_t fee, string memo) {

	if (getMyUTXOAmount(type) < value + fee) {
		cout << "There are not enough value of my securities : " << type << '\n';
		return false;
	}

	vector<Input> inputs;
	int64_t inputSum = 0;
	for (UTXO & utxo : myUTXOTable) {	// �ڽ��� UTXO�� Greedy ������� ����
		if (value + fee > inputSum) {
			Input input(utxo.output.recipientPublicKeyHash, publicKey, utxo.txHash, utxo.outputIndex, utxo.blockHeight);	// input.signature�� ������ UTXO�� pubKeyHash(32 byte) �Է� 
			inputs.push_back(input);
			inputSum += utxo.output.value;
		}
	}

	vector<Output> outputs;
	Output output(_recipientPublicKeyHash, value, type, State::OWN);
	outputs.push_back(output);

	if (value + fee != inputSum) {
		assert(value + fee < inputSum);
		Output output2(publicKeyHash, inputSum - value - fee, type, State::OWN);	// �Ž������� �ڽ��� �ּҷ� ����
		outputs.push_back(output2);
	}
		
	Transaction tempTx(inputs, outputs, blockchainVersion, memo);	// Tx ������ ���� �� Tx�� txHash ���

	if (!signOnTx(tempTx))
		return false;

	tx = tempTx;
	broadcastTransaction(tempTx);
	return true;
}

//*************************createTransaction �̶� ��ġ�� �κ�(=ó�� ���� ����) ��ġ��
bool Wallet::createTransactionSwitchState(Transaction & tx, int blockchainVersion, const std::uint8_t * recipientPublicKeyHash, 
	Type & type, std::int64_t value, std::int64_t fee, const State state, std::string memo) {
	
	State currentState = state == State::OWN ? State::SALE : State::OWN;

	if (getMyUTXOAmount(type, currentState) < value + fee) {
		cout << "There are not enough value of mine...\n";
		return false;
	}

	vector<Input> inputs;
	int64_t inputSum = 0;
	for (UTXO & utxo : myUTXOTable) {	// �ڽ��� UTXO�� Greedy ������� ����
		if (value + fee > inputSum) {
			Input input(utxo.output.recipientPublicKeyHash, publicKey, utxo.txHash, utxo.outputIndex, utxo.blockHeight);	// input.signature�� ������ UTXO�� pubKeyHash(32 byte) �Է� 
			inputs.push_back(input);
			inputSum += utxo.output.value;
		}
	}

	vector<Output> outputs;
	Output output(recipientPublicKeyHash, value, type, state);
	outputs.push_back(output);

	if (value + fee != inputSum) {
		assert(value + fee < inputSum);
		Output output2(publicKeyHash, inputSum - value - fee, type, currentState);	// �Ž������� �ڽ��� �ּҷ� ����
		outputs.push_back(output2);
	}

	Transaction tempTx(inputs, outputs, blockchainVersion, memo);	// Tx ������ ���� �� Tx�� txHash ���

	if (!signOnTx(tempTx))
		return false;

	tx = tempTx;
	broadcastTransaction(tempTx);
	return true;
}

//*************************createTransaction �̶� ��ġ�� �κ� ��ġ��
bool Wallet::createTransactionPurchaseSale(Transaction & tx, int blockchainVersion, UTXO & saleSecurities,
	std::int64_t value, std::int64_t fee, std::string memo) {

	if (getMyUTXOAmount(Type()) < value * saleSecurities.output.type.marketValue) {
		cout << "There are not enough value of my GLC...\n";
		return false;
	}

	vector<Input> inputs;
	int64_t inputSum = 0;
	for (UTXO & utxo : myUTXOTable) {	// �ڽ��� UTXO�� Greedy ������� ����
		if (value * saleSecurities.output.type.marketValue > inputSum) {
			Input input(utxo.output.recipientPublicKeyHash, publicKey, utxo.txHash, utxo.outputIndex, utxo.blockHeight);	// input.signature�� ������ UTXO�� pubKeyHash(32 byte) �Է� 
			inputs.push_back(input);
			inputSum += utxo.output.value;
		}
	}

	Input input(saleSecurities.output.recipientPublicKeyHash, publicKey, saleSecurities.txHash, saleSecurities.outputIndex, saleSecurities.blockHeight);	// �Ǹ� ���� �������� ����
	inputs.push_back(input);

	vector<Output> outputs;
	Output output(saleSecurities.output.recipientPublicKeyHash, value * saleSecurities.output.type.marketValue, Type(), State::OWN);				// �Ǹ��ڿ��� �������� ���(GLC) ����
	outputs.push_back(output);

	Output output2(publicKeyHash, value, saleSecurities.output.type, State::OWN);											// ������ �������� ������ ����
	outputs.push_back(output2);

	if (value * saleSecurities.output.type.marketValue != inputSum) {
		assert(value * saleSecurities.output.type.marketValue < inputSum);
		Output output3(publicKeyHash, inputSum - value * saleSecurities.output.type.marketValue, Type(), State::OWN);		// GLC �Ž������� �� �ּҷ� ����
		outputs.push_back(output3);
	}

	Transaction tempTx(inputs, outputs, blockchainVersion, memo);	// Tx ������ ���� �� Tx�� txHash ���
	
	if (!signOnTx(tempTx))
		return false;

	tx = tempTx;
	broadcastTransaction(tempTx);
	return true;
}

bool Wallet::signOnTx(Transaction & tx) {
	for (Input & input : tx.inputs) {
		if (uECC_sign(privateKey, tx.txHash, sizeof(tx.txHash), input.signature, curve) == 0) {			// ��� input�� �ڽ��� ���� ����
			cout << "An error occurred when generating the signature...\n";								// �Ǹ����� �Ǹ� ���� �������� ����
			return false;
		}
	}

	return true;
}

void Wallet::broadcastTransaction(Transaction tx) const {
	TransactionBroadcaster tb;
	future<void> f2 = async(launch::async, &TransactionBroadcaster::broadcast, ref(tb), tx, "localhost", "8000");
	cout << "Broadcasting transaction...\n";
}



int64_t Wallet::getMyUTXOAmount(const Type & type) const {
	if (myUTXOTable.size() == 0)
		return 0;

	int64_t sum = 0;
	for (const UTXO & utxo : myUTXOTable) {
		if (utxo.output.type == type) {
			sum += utxo.output.value;
		}
	}

	return sum;
}

int64_t Wallet::getMyUTXOAmount(const Type & type, const State state) const {
	if (myUTXOTable.size() == 0)
		return 0;
	
	int64_t sum = 0;
	for (const UTXO & utxo : myUTXOTable) {
		if (utxo.output.type == type && utxo.output.state == state) {
			sum += utxo.output.value;
		}
	}

	return sum;
}

//void Wallet::setUTXOTable(std::vector<UTXO> & _UTXOTable) {
//	for (const UTXO & utxo : _UTXOTable) {
//		if (!isMemoryEqual(publicKeyHash, utxo.output.recipientPublicKeyHash, sizeof(publicKeyHash))) {
//			UTXOTable = _UTXOTable;
//			sort(UTXOTable.begin(), UTXOTable.end());
//			break;
//		}
//	}
//	myUTXOTable = _UTXOTable;
//	sort(myUTXOTable.begin(), myUTXOTable.end());
//}

void Wallet::printUTXOTable(std::ostream & o) const {
	o << "----- UTXO Table -----\n";
	size_t index = 0;
	for (const UTXO & utxo : UTXOTable) {
		cout << '#' << index++ << '\n';
		utxo.print(o);
		o << '\n';
	}
}

void Wallet::printMyUTXOTable(std::ostream & o) const {
	o << "----- My UTXO Table -----\n";
	size_t index = 0;
	for (const UTXO & utxo : myUTXOTable) {
		cout << '#' << index++ << '\n';
		utxo.print(o);
		o << '\n';
	}
}
