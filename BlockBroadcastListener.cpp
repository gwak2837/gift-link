#include <iostream>
#include <cstdlib>
#include "BlockBroadcastListener.h"
using boost::asio::ip::tcp;
using namespace std;

Session::Session(tcp::socket& sock)
//boost 1.66���� (Ubuntu 18.10 ����) ������ ��� io_context�� ���
//session(boost::asio::io_context& io_service)
	: m_socket(sock) {
	start();
}

void Session::start() {
	try {
		while (1) {
			boost::system::error_code error;
			size_t length = m_socket.read_some(boost::asio::buffer(m_data), error);
			if (error == boost::asio::error::eof) {
				//Ŭ���̾�Ʈ�� ���� ������ ��� �޾����Ƿ� �����Ѵ�.
				break;
			}
			else if (error) {
				throw boost::system::system_error(error);
			}
			cout << "Message from client: " << m_data << endl;
			boost::system::error_code ignored_error;
			boost::asio::write(m_socket, boost::asio::buffer(m_data, length + 1), ignored_error);
		}
	}
	catch (exception& e) {
		cerr << "Exception in server: " << e.what() << endl;
	}
}


//BlockBroadcastListener::BlockBroadcastListener(boost::asio::io_context& io_service, short port)
//	: m_io_service(io_service), m_acceptor(io_service, tcp::endpoint(tcp::v4(), port)) {
//	while (1) {
//		//boost 1.66���� (Ubuntu 18.10 ����) ������ ��� io_context�� ���
//		//new session(acceptor_.accept());
//
//		//sever�� ���� ����
//		tcp::socket sock(io_service);
//
//		//client�κ��� ���� ���
//		m_acceptor.accept(sock);
//
//		new Session(sock);
//	}
//}


BlockBroadcastListener::~BlockBroadcastListener() {}
