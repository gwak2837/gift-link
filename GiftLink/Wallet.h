#pragma once
#ifndef WALLET_H
#define WALLET_H
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>			// memcpy
#include "Transaction.h"	/* 헤더 include 충돌? */
#include "uECC.h"

class Wallet {
	std::uint8_t privateKey[SECP256R1_NUM_BYTES];
	std::uint8_t publicKey[SECP256R1_NUM_BYTES * 2];
	std::uint8_t publicKeyHash[SHA256_DIGEST_VALUELEN];		// = SHA256(publicKey)
	std::uint8_t compressedPublicKey[SECP256R1_NUM_BYTES + 1];

	const struct uECC_Curve_t * curve = uECC_secp256r1();

public:
	std::vector<Output *> UTXOTable;
	std::vector<Output *> myUTXOTable;

	Wallet();
	Wallet(std::uint8_t * _privateKey);

	Transaction createTransaction(int blockchainVersion, const std::uint8_t * _recipientPublicKeyHash, Type _type,
		std::int64_t value, std::int64_t fee, std::string memo = "");

	// getter method
	inline const std::uint8_t * getPrivateKey() const { return privateKey; }
	inline const std::uint8_t * getPublicKey() const { return publicKey; }
	inline const std::uint8_t * getPublicKeyHash() const { return publicKeyHash; }
	std::int64_t getMyUTXOAmount(Type _type) const;

	void printTransactionHistory() const;
	void printMyUTXOTable() const;
	void printUTXOTable() const;
};



#endif