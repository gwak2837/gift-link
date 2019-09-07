#pragma once
#ifndef WALLET_H
#define WALLET_H
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>			// memcpy
#include "Transaction.h"	/* ��� include �浹? */
#include "uECC.h"

class Wallet {
	std::uint8_t privateKey[SECP256R1_NUM_BYTES];
	std::uint8_t publicKey[SECP256R1_NUM_BYTES * 2];
	std::uint8_t publicKeyHash[SHA256_DIGEST_VALUELEN];		// = SHA256(publicKey)
	std::uint8_t compressedPublicKey[SECP256R1_NUM_BYTES + 1];

	const struct uECC_Curve_t * curve = uECC_secp256r1();

	std::vector<UTXO> UTXOTable;
	std::vector<UTXO> myUTXOTable;

public:
	Wallet();
	Wallet(std::uint8_t * _privateKey);

	bool createTransaction(Transaction & tx, int blockchainVersion, const std::uint8_t * _recipientPublicKeyHash, Type _type,
		std::int64_t value, std::int64_t fee, std::string memo = "");

	// getter method
	inline const std::uint8_t * getPrivateKey() const { return privateKey; }
	inline const std::uint8_t * getPublicKey() const { return publicKey; }
	inline const std::uint8_t * getPublicKeyHash() const { return publicKeyHash; }
	std::int64_t getMyUTXOAmount(Type _type) const;
	inline std::vector<UTXO> getUTXOTable() const { return UTXOTable; }
	inline std::vector<UTXO> getMyUTXOTable() const { return myUTXOTable; }

	void setUTXOTable(std::vector<UTXO> & UTXOTable);		// ���������� UTXOTable�� myUTXOTable�� �����ؼ� ����

	void printUTXOTable(std::ostream & o) const;
	void printMyUTXOTable(std::ostream & o) const;
	void printTransactionHistory(std::ostream & o) const;
};

/* createTransaction�� ��ȯ���� Transaction�� ����:
�Լ� ���ο��� �������� Transaction(���� ����� �ӽú��� ��ȯ)�� ������ ��ȯ�ϴ� ���¶�
��ȯ���� Transaction &(��ü �� ��ü�� ��ȯ) �̸� �Ҹ�� ���������� ��ȯ�ϱ� ������ ���� �ʴ�. */

#endif