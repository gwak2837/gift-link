#pragma once
#ifndef BROADCAST_LISTENER_H
#define BROADCAST_LISTENER_H
#include <boost/asio.hpp>
#include "Blockchain.h"

class BroadcastListener {
	boost::asio::io_context m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;

public:
	BroadcastListener(short port);
	~BroadcastListener();
	void listen(Blockchain & bc);
};

#endif

