#pragma once
#ifndef BLOCK_BROADCAST_LISTENER_H
#define BLOCK_BROADCAST_LISTENER_H
#include <boost/asio.hpp>
#include "Block.h"

class BlockBroadcastListener {
	boost::asio::io_context m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;

public:
	BlockBroadcastListener(short port);
	~BlockBroadcastListener();
	void listen();
};

#endif

