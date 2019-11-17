#pragma once
#ifndef BLOCK_BROADCAST_LISTENER_H
#define BLOCK_BROADCAST_LISTENER_H
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "Block.h"

class Session {
	boost::asio::ip::tcp::socket& m_socket;
	enum { max_length = 1024 };
	char m_data[max_length];

public:
	Session(boost::asio::ip::tcp::socket& sock);
	void start();
	inline boost::asio::ip::tcp::socket& socket() { return m_socket; }
};

class BlockBroadcastListener {
	boost::asio::io_context& m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;

public:
	BlockBroadcastListener(boost::asio::io_context& io_service, short port);
	~BlockBroadcastListener();
};

#endif

