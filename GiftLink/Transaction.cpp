#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cassert>
#include "Transaction.h"
#include "Utility.h"
using namespace std;

std::ostream& operator<<(std::ostream & o, Type & type);

Output::Output(const std::uint8_t * _recipientPublicKeyHash, std::int64_t _value, Type _type) : value(_value), type(_type) {
	memcpy(recipientPublicKeyHash, _recipientPublicKeyHash, sizeof(recipientPublicKeyHash));
}

void Output::print(std::ostream & o) const {
	o << "Recipient Public Key Hash: " << recipientPublicKeyHash << '\n';
	o << "Value: " << value << '\n';
}



Input::Input() {
	memset(signature, 0, sizeof(signature));
	memset(senderPublicKey, 0, sizeof(senderPublicKey));
	memset(previousTxHash, 0, sizeof(previousTxHash));
	outputIndex = -1;
	blockHeight = UINT64_MAX;
}

Input::Input(const uint8_t * previousOutputPublicKeyHash, const uint8_t * _senderPublicKey,
	const uint8_t * _previousTxHash, int _outputIndex, uint64_t _blockHeight)
	: outputIndex(_outputIndex), blockHeight(_blockHeight) {
	memcpy(signature, previousOutputPublicKeyHash, SHA256_DIGEST_VALUELEN);
	memcpy(senderPublicKey, _senderPublicKey, sizeof(senderPublicKey));
	memcpy(previousTxHash, _previousTxHash, sizeof(previousTxHash));
}



Transaction::Transaction(vector<Input> & _inputs, vector<Output> & _outputs, int _version, string _memo)
	: inputs(_inputs), outputs(_outputs), version(_version), memo(_memo) {
	timestamp = time(NULL);

	uint8_t * txData = createTxData();

	SHA256_Encrpyt(txData, (unsigned int)getTxDataSize(), txHash);
	delete[] txData;
}


/* 기획 의도에 맞게 수정 필요 */
uint8_t * Transaction::createTxData() const {
	size_t i = 0;									// txData index
	uint8_t * txData = new BYTE[getTxDataSize()];	// Assertion: getTxSize와 i의 최종 값은 동일해야 한다.
	int sigHashCode = 1;

	memcpy(txData + i, &version, sizeof(version));
	i += sizeof(version);

	for (Input input : inputs) {
		memcpy(txData + i, input.previousTxHash, sizeof(input.previousTxHash));
		i += sizeof(input.previousTxHash);

		memcpy(txData + i, &input.outputIndex, sizeof(input.outputIndex));
		i += sizeof(input.outputIndex);

		memcpy(txData + i, input.signature, sizeof(outputs[0].recipientPublicKeyHash));	// 참조할 UTXO의 PubKey Hash(32 byte)
		i += sizeof(outputs[0].recipientPublicKeyHash);
	}

	for (Output output : outputs) {
		memcpy(txData + i, &output.value, sizeof(output.value));
		i += sizeof(output.value);

		memcpy(txData + i, &output.type, sizeof(output.type));
		i += sizeof(output.type);

		memcpy(txData + i, output.recipientPublicKeyHash, sizeof(output.recipientPublicKeyHash));
		i += sizeof(output.recipientPublicKeyHash);
	}

	memcpy(txData + i, &timestamp, sizeof(timestamp));
	i += sizeof(timestamp);

	memcpy(txData + i, &sigHashCode, sizeof(sigHashCode));
	i += sizeof(sigHashCode);

	memcpy(txData + i, memo.c_str(), memo.length());
	i += memo.length();

	assert(i == getTxDataSize());

	return txData;
}

bool Transaction::isCoinbase() const {
	if (inputs.size() != 1)
		return false;

	if (inputs[0].outputIndex != -1 || inputs[0].blockHeight != -1)
		return false;

	for (int i = 0; i < sizeof(inputs[0].previousTxHash); i++) {
		if (inputs[0].previousTxHash[i] != 0)
			return false;
	}

	for (int i = 0; i < sizeof(inputs[0].senderPublicKey); i++) {
		if (inputs[0].senderPublicKey[i] != 0)
			return false;
	}

	for (int i = 0; i < sizeof(inputs[0].signature); i++) {
		if (inputs[0].signature[i] != 0)
			return false;
	}

	return true;
}

void Transaction::print(ostream & o) const {
	o << "Transaction Hash: " << txHash << '\n';
	o << "Version:          " << version << '\n';
	o << "Timestamp:        " << timeToString(timestamp) << '\n';
	o << "Memo:             " << memo << "\n\n";

	o << "-Transaction Input-\n";
	for (Input input : inputs) {
		o << "Sender Signature:         ";
		printInHex(o, input.signature, sizeof(input.signature));
		o << "Sender Public Key:        ";
		printInHex(o, input.senderPublicKey, sizeof(input.senderPublicKey));
		o << "Previous Tx Hash:         " << input.previousTxHash << '\n';
		o << "Previous Tx Output Index: " << input.outputIndex << "\n\n";
	}

	o << "-Transaction Output-\n";
	for (Output output : outputs) {
		o << "Recipient Public Key Hash: " << output.recipientPublicKeyHash << '\n';
		o << "Type:                      " << output.type << '\n';
		o << "Value:                     " << output.value << "\n\n";
	}
}

ostream & operator<<(ostream & o, Type & type) {
	switch (type) {
	case Type::A:
		return o << "GiftCard A";
	case Type::GLC:
		return o << "GiftLink Coin";
	default:
		return o << "Undefined";
	}
}

UTXO::UTXO(const std::uint8_t * _txHash, Output _output, int _outputIndex, std::uint64_t _blockHeight) : output(_output), 
	outputIndex(_outputIndex), blockHeight(_blockHeight) {
	memcpy(txHash, _txHash, sizeof(txHash));
}

void UTXO::print(std::ostream & o) const {
	o << "Unspent Transaction Hash: " << txHash << '\n';
	output.print(o);
}

bool operator<(const UTXO & utxo1, const UTXO & utxo2) {
	return utxo1.output.value > utxo2.output.value;
}
