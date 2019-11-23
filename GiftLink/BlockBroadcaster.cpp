#include <iostream>
#include <cstdlib>
#include <boost/archive/text_oarchive.hpp>
#include "BlockBroadcaster.h"
using boost::asio::ip::tcp;
using namespace std;

BlockBroadcaster::BlockBroadcaster() : socket(io_service), resolver(io_service) {}

BlockBroadcaster::~BlockBroadcaster() {}

void BlockBroadcaster::broadcast(const Block * _block, string host, string port) {
	//boost::asio::streambuf block;
	//boost::archive::text_oarchive oa(cout);
	//oa << *_block;

	stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << *_block;

	//boost::asio::streambuf block;
	//std::ostream os(&block);

	//os << *_block;

	// serialize into `os`

	try {
		//서버에 접속을 시도한다.
		boost::asio::connect(socket, resolver.resolve({ host, port }));

		//서버로 데이터를 전송
		//boost::asio::write(socket, boost::asio::buffer(block));
		//boost::asio::write(socket, boost::asio::buffer(&oa, sizeof(oa)));
		boost::system::error_code ignored_error;
		boost::asio::write(socket, boost::asio::buffer(ss.str()), ignored_error);

		//boost::array<char, 10> reply;
		//boost::system::error_code error;
		//
		////응답 대기
		//size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, 10), error);

		//cout << "Reply is: ";
		//cout.write(reply.data(), reply_length);
		//if (strcmp(reply.data(), "received!\n") == 0)
		//	return;
		//else if (error)
		//	throw boost::system::system_error(error);
	}
	catch (exception & e) {
		cerr << e.what() << endl;
	}

	_block->print(cout);
}
