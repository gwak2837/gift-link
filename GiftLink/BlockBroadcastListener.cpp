#include <iostream>
#include <cstdlib>
#include <array>
#include <boost/archive/text_iarchive.hpp>
#include "BlockBroadcastListener.h"
using boost::asio::ip::tcp;
using namespace std;

BlockBroadcastListener::BlockBroadcastListener(short port)
	: m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), port)) {}

BlockBroadcastListener::~BlockBroadcastListener() {}

void BlockBroadcastListener::listen(Blockchain & bc) {
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
			array<char, 10000> stringBuffer{};
			//boost::asio::streambuf stringBuffer;
			while (1) {
				//size_t length = socket.read_some(boost::asio::buffer(stringBuffer), error);
				size_t length = socket.read_some(boost::asio::buffer(stringBuffer), error);
				//size_t length = boost::asio::read(socket, boost::asio::buffer(broadcastedBlock), error);
				
				if (error == boost::asio::error::eof)		// 클라이언트로부터 정보를 모두 받으면 종료
					break;
				else if (error)
					throw boost::system::system_error(error);

				std::stringstream ss(stringBuffer.data());
				boost::archive::text_iarchive ia(ss);

				Block broadcastedBlock;
				ia >> broadcastedBlock;

				Block * b = new Block(broadcastedBlock);
				b->setAdditionalInfo();
				if (!b->isValid()) {
					cout << "Invalid broadcasted block...\n";	//DEBUG
					break;
				}

				//if (isMemoryEqual(b->previousBlockHash, bc.lastBlock->blockHash, SHA256_DIGEST_VALUELEN)) {
					b->height = bc.lastBlock->height + 1;
					b->previousBlock = bc.lastBlock;
					bc.blockCount++;
					bc.lastBlock = b;

					if (bc.waitingBlock != NULL) {
						delete bc.waitingBlock;
						bc.waitingBlock = new Block(bc.lastBlock);
					}
				//}


				//for (size_t i = 0; i < bc.getBlockCount(); i++, presentBlock = presentBlock->previousBlock) {
				//	if (isMemoryEqual(broadcastedBlock.previousBlockHash, presentBlock->blockHash, SHA256_DIGEST_VALUELEN)) {
				//		bc.lastBlock = waitingBlock;
				//		blockCount++;
				//		waitingBlock->height = blockCount - 1;
				//		waitingBlock->isMainChain = true;

				//		Block * newWaitingBlock = new Block(lastBlock);
				//		waitingBlock = newWaitingBlock;

				//	}
				//}

				
				//****************************전파된 블록을 블록체인에 연결

				cout << "New block from other node: \n";		//DEBUG
				b->print(cout);					//DEBUG

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

