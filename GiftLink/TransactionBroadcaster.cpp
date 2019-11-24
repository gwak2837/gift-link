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
		// �ٸ� ��忡 ������ �õ��Ѵ�.
		boost::asio::connect(socket, resolver.resolve({ host, port }));

		//������ �����͸� ����
		boost::system::error_code ignored_error;
		boost::asio::write(socket, boost::asio::buffer(ss.str()), ignored_error);
	}
	catch (exception & e) {
		cerr << e.what() << endl;
	}

	tx.print(cout);
}
