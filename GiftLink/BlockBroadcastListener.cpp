#include <iostream>
#include <cstdlib>
#include "BlockBroadcastListener.h"
#include "Blockchain.h"
using boost::asio::ip::tcp;
using namespace std;

BlockBroadcastListener::BlockBroadcastListener(short port)
	: m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), port)) {}

BlockBroadcastListener::~BlockBroadcastListener() {}

void BlockBroadcastListener::listen() {
	while (1) {
		//boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
		//new session(acceptor_.accept());

		//sever의 소켓 생성
		tcp::socket socket(m_io_service);

		//client로부터 접속 대기
		cout << "Waiting...\n";
		m_acceptor.accept(socket);
		cout << "Connected!\n";

		try {
			boost::system::error_code error;
			//boost::array<char, 10000> broadcastedBlock;


			array<char, 1000> stringBuffer{};
			//boost::asio::streambuf stringBuffer;
			while (1) {
				//size_t length = socket.read_some(boost::asio::buffer(stringBuffer), error);
				size_t length = socket.read_some(boost::asio::buffer(stringBuffer), error);
				//size_t length = boost::asio::read(socket, boost::asio::buffer(broadcastedBlock), error);
				
				if (error == boost::asio::error::eof) {
					//클라이언트로 부터 정보를 모두 받았으므로 종료한다.
					break;
					//continue;
				}
				else if (error) {
					throw boost::system::system_error(error);
				}


				std::stringstream ss(stringBuffer.data());
				boost::archive::text_iarchive ia(ss);

				Block broadcastedBlock;
				ia >> broadcastedBlock;
				broadcastedBlock.setAdditionalInfo();
				//if (!broadcastedBlock[0].isValid()) {
				//	cout << "Invalid broadcasted block...\n";
				//	break;
				//}
				cout << "New block from other node: \n";
				broadcastedBlock.print(cout);

				//boost::system::error_code ignored_error;
				//boost::asio::write(socket, boost::asio::buffer("received!\n"), ignored_error);

				

				//if (!broadcastedBlock[0].isValid()) {
				//	cout << "Invalid broadcasted block...\n";
				//	break;
				//}
			}
		}
		catch (exception& e) {
			cerr << "Exception in server: " << e.what() << endl;
		}
	}
}


//int main() {
//	try {
//		boost::asio::io_context io_service;
//		BlockBroadcastListener bbl(io_service, 2222);
//		bbl.listen();
//	}
//	catch (exception& e) {
//		cerr << "Exception: " << e.what() << endl;
//	}
//
//	system("pause");
//	return 0;
//}

