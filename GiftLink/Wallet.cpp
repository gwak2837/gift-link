#include <vector>
#include <string>
#include <cstring>
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


/* �ŷ��� ������.
���Ŀ� ��ǰ�� ��� �Ͱ� �ڽ��� GLC�� ������ ���� ���ÿ� �� �ŷ��� ��� ����� ������ ������.

Transaction ���� ����
1. Input ����
2. Output ����
3. Transaction ������ �ؽ� 
4. Transaction�� ��� Input�� ���� */
Transaction Wallet::createTransaction(int blockchainVersion, const uint8_t * _recipientPublicKeyHash, Type _type, 
	int64_t value, int64_t fee, string memo) {

	vector<Input> inputs;

	// �ڽ��� UTXO�� Greedy ������� ����
	// input.signature�� ������ UTXO�� pubKeyHash(32 byte) �Է� 

	/* ���� ���� */
	Input input(publicKeyHash, publicKey, publicKeyHash, 12, 12);	// previousOutputPubKeyHash, senderPublicKey, previousTxHash
	Input input2(publicKeyHash, publicKey, publicKeyHash, 34, 34);
	inputs.push_back(input);
	inputs.push_back(input2);
	/* ���� �� */

	vector<Output> outputs;
	Output output(_recipientPublicKeyHash, value, _type);
	outputs.push_back(output);

	//if (�Ž������� �ʿ��ϸ�) {
		Output output2(publicKey, value, _type);			// �Ž������� �ڽ��� �ּҷ� ����
		outputs.push_back(output2);
	//}
		
	Transaction tx(inputs, outputs, blockchainVersion, memo);	// Tx ������ ���� �� Tx�� txHash ���

	for (Input & input : tx.inputs) { 
		if (uECC_sign(privateKey, tx.txHash, sizeof(tx.txHash), input.signature, curve) == 0)
			cout << "An error occurred when generating the signature...\n";
	}

	return tx;
}




int64_t Wallet::getMyUTXOAmount(Type _type) const {
	return 0;
}

void Wallet::printMyUTXOTable() const {
}

void Wallet::printUTXOTable() const {
}
