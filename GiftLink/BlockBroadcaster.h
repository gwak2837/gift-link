#pragma once
#ifndef BLOCK_BROADCASTER_H
#define BLOCK_BROADCASTER_H
#include <string>
#include <boost/asio.hpp>
#include "Block.h"

class BlockBroadcaster {
	boost::asio::io_context io_service;
	boost::asio::ip::tcp::socket socket;
	boost::asio::ip::tcp::resolver resolver;

public:
	BlockBroadcaster();
	~BlockBroadcaster();
	void broadcast(const Block * block, std::string host, std::string port);
};


#endif

