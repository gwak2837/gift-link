#include <iostream>
#include <cstdlib>
#include <boost/archive/text_oarchive.hpp>
#include "TransactionBroadcaster.h"
using boost::asio::ip::tcp;
using namespace std;

TransactionBroadcaster::TransactionBroadcaster() : socket(io_service), resolver(io_service) {}

TransactionBroadcaster::~TransactionBroadcaster() {}

void TransactionBroadcaster::broadcast(const Transaction & tx, string host, string port) {
	stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << tx;

	try {
		// 다른 노드에 접속을 시도한다.
		boost::asio::connect(socket, resolver.resolve({ host, port }));

		//서버로 데이터를 전송
		boost::system::error_code ignored_error;
		boost::asio::write(socket, boost::asio::buffer(ss.str()), ignored_error);
	}
	catch (exception & e) {
		cerr << e.what() << endl;
	}

	tx.print(cout);
}
