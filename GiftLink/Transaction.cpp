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


Type::Type() : name("GiftLink Coin"), faceValue(0), marketValue(0), expirationDate((time_t)pow(2, sizeof(time_t) * 8 - 1) - 1) {
	memset(issuerSignature, 0, sizeof(issuerSignature));
}

Type::Type(std::string _n, std::int64_t _f, std::int64_t _m, time_t _e, const std::uint8_t * issuerPrivateKey)
	: name(_n), faceValue(_f), marketValue(_m), expirationDate(_e) {		// 유가증권 이름, 액면가, 시장가, 유효기간 설정

	//std::uint8_t _issuerSignature[SECP256R1_NUM_BYTES * 2];

	//********이름 + 액면가 + 유효기간으로 해시 만들고
	//********해시 + 개인키로 디지털 서명 만들고

	//memcpy(issuerSignature, _issuerSignature, sizeof(issuerSignature));
}

void Type::print(ostream & o) const {
	if (*this == Type())
		o << this->name;
	else
		o << this->name << ", " << this->faceValue << ", " << this->marketValue << ", " << timeToString(this->expirationDate);
	o << '\n';
}



Output::Output(const std::uint8_t * _recipientPublicKeyHash, std::int64_t _value, Type _type, State _state) : value(_value), type(_type), state(_state) {
	memcpy(recipientPublicKeyHash, _recipientPublicKeyHash, sizeof(recipientPublicKeyHash));
}

void Output::print(ostream & o) const {
	o << "Recipient Public Key Hash: " << recipientPublicKeyHash << '\n';
	o << "Value: " << value << '\n';
	o << "Type:  " << type << '\n';
	o << "State: " << state << '\n';
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
	memset(signature, 0, sizeof(signature));
	memcpy(signature, previousOutputPublicKeyHash, SHA256_DIGEST_VALUELEN);
	memcpy(senderPublicKey, _senderPublicKey, sizeof(senderPublicKey));
	memcpy(previousTxHash, _previousTxHash, sizeof(previousTxHash));
}



Transaction::Transaction(vector<Input> & _inputs, vector<Output> & _outputs, int _version, string _memo)
	: inputs(_inputs), outputs(_outputs), version(_version), memo(_memo) {
	timestamp = time(NULL);

	
	const uint8_t * txData = createTxData();
	//unique_ptr<const uint8_t> txData(createTxData());	// 적절한 변환 함수가 없습니다..



	//DEBUG
	//int i = 0;
	//cout << memo << '\n';
	//cout << "0x";
	//for (int j = 0; j < 4; i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)txData[i];
	//}
	//cout << "\nInput : \n";
	//for (int j = 0; j < inputs.size() * (sizeof(inputs[0].previousTxHash) + sizeof(inputs[0].outputIndex) + sizeof(outputs[0].recipientPublicKeyHash)); i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)txData[i];
	//}
	//cout << "\nOutput : \n";
	//for (int j = 0; j < outputs.size() * (sizeof(outputs[0].value) + sizeof(outputs[0].type) + sizeof(outputs[0].state) + sizeof(outputs[0].recipientPublicKeyHash)); i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)txData[i];
	//}
	//cout << "\nTimestamp + Hashcode + memo : \n";
	//for (int j = 0; j < sizeof(timestamp) + 4 + memo.length(); i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)txData[i];
	//}
	//cout << dec;
	//cout << '\n';


	





	//DEBUG
	//int i = 0;
	//cout << testTx.memo << '\n';
	//cout << "0x";
	//for (int j = 0; j < 4; i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)testTxData[i];
	//}
	//cout << "\nInput : \n";
	//for (int j = 0; j < testTx.inputs.size() * (sizeof(testTx.inputs[0].previousTxHash) + sizeof(testTx.inputs[0].outputIndex) + sizeof(testTx.outputs[0].recipientPublicKeyHash)); i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)testTxData[i];
	//}
	//cout << "\nOutput : \n";
	//for (int j = 0; j < testTx.outputs.size() * (sizeof(testTx.outputs[0].value) + sizeof(testTx.outputs[0].type) + sizeof(testTx.outputs[0].state) + sizeof(testTx.outputs[0].recipientPublicKeyHash)); i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)testTxData[i];
	//}
	//cout << "\nTimestamp + Hashcode + memo : \n";
	//for (int j = 0; j < sizeof(testTx.timestamp) + 4 + testTx.memo.length(); i++, j++) {
	//	cout.width(2);
	//	cout.fill('0');
	//	cout << hex << (int)testTxData[i];
	//}
	//cout << dec;
	//cout << '\n';



	SHA256_Encrpyt(txData, (unsigned int)getTxDataSize(), txHash);
	delete[] txData;
}

uint8_t * Transaction::createTxData() const {
	size_t i = 0;									// txData index
	uint8_t * txData = new BYTE[getTxDataSize()];	// Assertion: getTxSize와 i의 최종 값은 동일해야 한다.
	int sigHashCode = 1;

	memcpy(txData + i, &version, sizeof(version));
	i += sizeof(version);

	for (const Input input : inputs) {
		memcpy(txData + i, input.previousTxHash, sizeof(input.previousTxHash));
		i += sizeof(input.previousTxHash);

		memcpy(txData + i, &input.outputIndex, sizeof(input.outputIndex));
		i += sizeof(input.outputIndex);

		memcpy(txData + i, input.signature, sizeof(outputs[0].recipientPublicKeyHash));	// 참조할 UTXO의 PubKey Hash(32 byte)
		i += sizeof(outputs[0].recipientPublicKeyHash);
	}

	for (const Output output : outputs) {
		memcpy(txData + i, &output.value, sizeof(output.value));
		i += sizeof(output.value);

		memcpy(txData + i, output.type.name.c_str(), output.type.name.length());
		i += output.type.name.length();

		memcpy(txData + i, &output.type.faceValue, sizeof(output.type.faceValue));
		i += sizeof(output.type.faceValue);

		memcpy(txData + i, &output.type.marketValue, sizeof(output.type.marketValue));
		i += sizeof(output.type.marketValue);

		memcpy(txData + i, &output.type.expirationDate, sizeof(output.type.expirationDate));
		i += sizeof(output.type.expirationDate);

		memcpy(txData + i, output.type.issuerSignature, sizeof(output.type.issuerSignature));
		i += sizeof(output.type.issuerSignature);

		memcpy(txData + i, &output.state, sizeof(output.state));
		i += sizeof(output.state);

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

size_t Transaction::getTxDataSize() const {
	assert(inputs.size() > 0);
	assert(outputs.size() > 0);
	size_t txDataSize = inputs.size() * (sizeof(inputs[0].previousTxHash) + sizeof(inputs[0].outputIndex) + sizeof(outputs[0].recipientPublicKeyHash))
		+ sizeof(version) + sizeof(timestamp) + 4 + memo.length();

	for (const Output output : outputs) {
		txDataSize += sizeof(output.value) + output.type.name.length() + sizeof(output.type.faceValue) + sizeof(output.type.marketValue)
			+ sizeof(output.type.expirationDate) + sizeof(output.type.issuerSignature) + sizeof(output.state) + sizeof(output.recipientPublicKeyHash);
	}

	return txDataSize;
}

// input 형태 검사로 코인베이스 거래 판별
bool Transaction::isCoinbase() const {
	if (inputs.size() != 1 || outputs.size() == 0)					// input 1개, output 1개 이상
		return false;

	if (inputs[0].outputIndex != -1)								// outputIndex는 -1
		return false;

	for (int i = 0; i < sizeof(inputs[0].previousTxHash); i++) {	// previousTxHash 전부 0
		if (inputs[0].previousTxHash[i] != 0)
			return false;
	}

	for (int i = 0; i < sizeof(inputs[0].senderPublicKey); i++) {	// senderPublicKey 전부 0
		if (inputs[0].senderPublicKey[i] != 0)
			return false;
	}

	for (int i = 0; i < sizeof(inputs[0].signature); i++) {			// signature 전부 0
		if (inputs[0].signature[i] != 0)
			return false;
	}

	return true;
}

// 서명+해시+공개키 유효성 검사
bool Transaction::isValid() const {
	if (inputs.size() == 0)
		return false;

	if (!isCoinbase()) {
		for (const Input & input : inputs) {
			if (uECC_verify(input.senderPublicKey, txHash, SHA256_DIGEST_VALUELEN, input.signature, uECC_secp256r1()) == 0)
				return false;
		}
	}

	return true;
}



void Transaction::print(ostream & o) const {
	o << "Transaction Hash: " << txHash << '\n';
	o << "Version:          " << version << '\n';
	o << "Timestamp:        " << timeToString(timestamp) << '\n';
	o << "Memo:             " << memo << "\n\n";

	o << "Transaction Input\n";
	for (Input input : inputs) {
		o << "Sender Signature:         ";
		printInHex(o, input.signature, sizeof(input.signature));
		o << "Sender Public Key:        ";
		printInHex(o, input.senderPublicKey, sizeof(input.senderPublicKey));
		o << "Previous Tx Hash:         " << input.previousTxHash << '\n';
		o << "Previous Tx Output Index: " << input.outputIndex << "\n\n";
	}

	o << "Transaction Output\n";
	for (Output output : outputs) {
		o << "Recipient Public Key Hash: " << output.recipientPublicKeyHash << '\n';
		o << "Type:                      ";
		output.type.print(o);
		o << "State:                     " << output.state << '\n';
		o << "Value:                     " << output.value << "\n\n";
	}
}



UTXO::UTXO(const std::uint8_t * _txHash, Output _output, int _outputIndex, std::uint64_t _blockHeight) : output(_output), 
	outputIndex(_outputIndex), blockHeight(_blockHeight) {
	memcpy(txHash, _txHash, sizeof(txHash));
}

void UTXO::print(std::ostream & o) const {
	o << "Unspent Transaction Hash:  " << txHash << '\n';
	output.print(o);
}



bool operator==(const Type & obj, const Type &obj2) {
	if (obj.name != obj2.name)
		return false;

	if (obj.faceValue != obj2.faceValue)
		return false;

	//if (obj.marketValue != obj2.marketValue)
	//	return false;

	if (obj.expirationDate != obj2.expirationDate)
		return false;

	if (!isMemoryEqual(obj.issuerSignature, obj2.issuerSignature, sizeof(obj.issuerSignature)))
		return false;

	return true;
}

bool operator!=(const Type & obj, const Type &obj2) {
	return !(obj == obj2);
}

bool operator<(const Type & type, const Type & type2) {
	return type.expirationDate < type2.expirationDate;
}

bool operator==(const Output & obj, const Output & obj2) {
	if (!isMemoryEqual(obj.recipientPublicKeyHash, obj2.recipientPublicKeyHash, sizeof(obj.recipientPublicKeyHash)))
		return false;

	if (obj.value != obj2.value)
		return false;

	if (obj.type != obj2.type)
		return false;

	if (obj.type == Type() && obj2.type == Type()) {
		if (obj.state != State::OWN || obj2.state != State::OWN)
			return false;
	}

	//if (state != obj.state)	find 함수에서 블록 생성 시 상태를 마음대로 설정할 수 있기 때문에
	//	return false;

	return true;
}

bool operator!=(const Output & obj, const Output & obj2) {
	return !(obj == obj2);
}

bool operator<(const UTXO & utxo1, const UTXO & utxo2) {
	return utxo1.output.value > utxo2.output.value;
}


