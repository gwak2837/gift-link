#include <iostream>
#include <cstdlib>
#include <array>
#include <boost/archive/text_iarchive.hpp>
#include "BroadcastListener.h"
using boost::asio::ip::tcp;
using namespace std;

BroadcastListener::BroadcastListener(short port)
	: m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), port)) {}

BroadcastListener::~BroadcastListener() {}

void BroadcastListener::listen(Blockchain & bc) {
	while (1) {
		//boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
		//new session(acceptor_.accept());

		//sever의 소켓 생성
		tcp::socket socket(m_io_service);

		//client로부터 접속 대기
		cout << "Waiting...\n";
		m_acceptor.accept(socket);
		cout << "Connected!\n";

		//try {
		boost::system::error_code error;
		array<char, 10000> stringBuffer{};
		while (1) {
			size_t length = socket.read_some(boost::asio::buffer(stringBuffer), error);

			if (error == boost::asio::error::eof)		// 클라이언트로부터 정보를 모두 받으면 종료
				break;
			else if (error)
				throw boost::system::system_error(error);

			// 블록 또는 Tx를 다운로드 받는다.
			try {
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

				// 전파받은 블록의 유효성을 판단하고 블록체인에 연결
				//if (isMemoryEqual(b->previousBlockHash, bc.lastBlock->blockHash, SHA256_DIGEST_VALUELEN)) {
				if (bc.lastBlock != NULL)
					b->height = bc.lastBlock->height + 1;
				else {
					b->height = 0;
					bc.genesisBlock = b;
				}
				b->previousBlock = bc.lastBlock;
				bc.blockCount++;
				bc.lastBlock = b;

				if (bc.waitingBlock != NULL)
					delete bc.waitingBlock;					// produceBlock() 함수와 충돌 가능
				bc.waitingBlock = new Block(bc.lastBlock);
				//}

				cout << "New block from other node: \n";		//DEBUG
				b->print(cout);									//DEBUG
			}
			catch (exception & e) {
				try {
					std::stringstream ss(stringBuffer.data());
					boost::archive::text_iarchive ia(ss);

					Transaction broadcastedTx;
					ia >> broadcastedTx;
					cout << "New transaction from other node: \n";	//DEBUG
					broadcastedTx.print(cout);						//DEBUG

					// 전파받은 거래의 유효성을 판단하고 거래 풀에 넣기
					if (bc.addTransactionToPool(broadcastedTx))
						cout << "Transaction was added in TxPool\n";
					else
						cout << "Broadcasted transaction is invalid...\n";

				}
				catch (exception & e) {
					cerr << "Exception in server: " << e.what() << endl;
				}
			}
		}
		//}
		//catch (exception & e) {
		//	cerr << "Exception in server: " << e.what() << endl;
		//}
	}
}

