#pragma once
#ifndef TRANSACTION_BROADCASTER_H
#define TRANSACTION_BROADCASTER_H
#include <string>
#include <boost/asio.hpp>
#include "Transaction.h"
#include "TransactionBroadcaster.h"

class TransactionBroadcaster {
	boost::asio::io_context io_service;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ip::tcp::resolver resolver;

public:
	TransactionBroadcaster();
	~TransactionBroadcaster();
	void broadcast(const Transaction & tx, std::string host, std::string port);
};

#endif
