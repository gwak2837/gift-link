#include <iostream>
#include <cstdlib>
#include "BlockBroadcastListener.h"
using boost::asio::ip::tcp;
using namespace std;

Session::Session(tcp::socket& sock)
//boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
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
				//클라이언트로 부터 정보를 모두 받았으므로 종료한다.
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
//		//boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
//		//new session(acceptor_.accept());
//
//		//sever의 소켓 생성
//		tcp::socket sock(io_service);
//
//		//client로부터 접속 대기
//		m_acceptor.accept(sock);
//
//		new Session(sock);
//	}
//}


BlockBroadcastListener::~BlockBroadcastListener() {}
