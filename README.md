# GiftLink
 블록체인 기반 상품권 거래 플랫폼


## 필요성(들어가는 말)
흩어진 상품권, 포인트, 쿠폰, 스탬프... 어떻게 관리합니까..? 관리하기 매우 귀찮다. 심지어 사용하지 못하고 잊어버리기도 쉽상. 잔액 환불받기도 귀찮아.

- 여러 곳에 흩어져 있는 유가증권 관리하기 어렵다. 사용하지 못하고 잊어버리기 쉽다.
- 잔액을 확인하거나 환불받기 번거롭다.
- 모바일 상품권은 발행 수수료(액면가액의 10%)가 높다.
- 지류 상품권은 발행 비용이 높다.



## 특징
간단한 발행 절차

상품권 물리적 발행 비용 없음

상품권 위변조 불가능

내 마음대로 정하는 발행 수수료

상품권 거래 내역 투명성

채굴로 코인 획득

## 의도
누구나 거래 내역을 볼 수 있고,

누구나 거래 내역을 검증할 수 있고,

누구나 상품권을 발행할 수 있다.


## 활용 예시
회원카드(얼마 충전하면 얼마 보너스)
포인트
스탬프(쿠폰, 출석체크 등)
금액권
상품 교환권



## 시나리오
### 상품권 판매자:
상품권을 발행하려면 

직접 상품권 거래를 채굴하거나
- 코인베이스 보상을 상품권으로 설정

상품권 거래를 생성해서 전파. 
- 수수료를 코인/상품권으로 설정 가능



1. 본인인증 및 사업자인증(세부 절차는 구상 중) 후 특정 상품권에 대한 발행 권한(GLT) 획득
- 매일 사업자 등록 여부 확인
- 상품권 발행 시, 상품권 구매 시에도 확인
- 만약 인증 실패 시 판매자의 상품권 발행 중지, 이의신청 기간 (얼마나) 부여, 구매자 상품권에도 주의 문구 표시

2. 발행 권한이 있는 상품권 발행
- 발행 수수료, 액면가, 실거래가, 개수, 상품권 유효기간 설정
- 모바일 상품권 액면가 3만원 초과 시 인지세 부과
- 유효기간 만료 상품권 잔액의 90% x (100%-할인율)를 구매자에게 자동 환불
- 상품권 총 발행 금액을 설정한 후 Coinbase 거래 Output에 넣음

3. 발행한 상품권 판매
- 판매 중인 상품권은 판매자의 서명이 담겨 있어 구매자의 서명만 있으면 거래 가능? 판매자 서명 탈취 공격은? 유효한가?

### 상품권 구매자
1. 판매 중인 상품권 목록 확인
2. 구매할 상품권 선택 및 GLC로 결제
3. 구매한 상품권 사용


### 상품권 개인 거래
1. 판매자는 판매할 자신 소유의 상품권 선택(실거래가, 개수) 및 판매
2. 구매자는 구매할 상품권 선택 및 GLC로 결제
3. 상품권 소유권 이전


### 상품권 거래 블록 생산
1. 거래 풀에 있는 거래 내역을 블록에 기록하고 블록을 채굴함(채굴 쓰레드)
2. 블록 채굴에 성공하면 해당 채굴자는 GLC를 획득하고 여러 노드에게 채굴된 블록을 전파(전파 쓰레드)



## 오픈소스 활용
ECDSA
SHA256


거래 구조
	Giftcard	GLC
Input	판매자(이전 거래의 Output)	구매자(이전 거래의 Output)
Output	구매자	판매자



과연 이 플랫폼이 판매자에게 이득을 줄까?
발행 권한 탈취 공격은?

판매자가 채굴을 통해 상품권 발행도 가능.
.판매자가 자신의 상품권을 한번에 많이 발행하고 더 이상 플랫폼에 기여를 안 하면?
..코인 보상이 정해져 있듯이 발행 액수를 제한?
...그럼 어떻게 제한?
....블록 높이에 따라서?
.....그럼 늦게 참여한 판매자는 발행액이 적겠네
...보상액수를 제한해도 그만큼 상품가격을 낮추면 많이 발행한 효과를 봄.(원/상품권 = 10으로 한다던가)
....그럼 원/상품권 = 1을 강제? 어떻게?



Type 클래스(열거형)를 Giftcard 클래스로 교체







## 용어
GLT(Gift Link Token) - 상품권 발행 권한
GLC(Gift Link Coin) - 상품권 결제 수단


### 기획 중
채굴보상을 특정 가게 상품권으로?
상품권 가맹점


### 진행 중
1.
생성자 만들 때 bits를 다른 값으로 초기화 가능하게
	bits(난이도)가 블록 채굴 시간에 따라 적절히 변하도록
채굴보상이 반감기를 가지도록
상품권 유효기간
상품권 별로 거래를 판단해야 함
상품권은 해당 가게 주인만 발행할 수 있도록


상품권 발행하려면 채굴해야 한다.
채굴하면 코인을 얻는다.

상품권이랑 코인이랑 한번에?


2.
loadBlockchain 개발 중
Blockchain::printWaitingBlock 개발 중

coinbase blockindex 추가


3.
채굴쓰레드, transaction 생성 쓰레드 분리 -> producer-consumer synchronize


### 완료
채굴 확인할 때 hash값을 byte 단위가 아니라 bit 단위로 확인할 수 있게
transactionPool -> waitingBlock을 transactionPool로 바꾸고 queue로 구현
block 유효성 검사 함수 -> timestamp가 시간 순인지
blockIndex 값 추가
blockchain version 변경 가능
nonce -1 되면 채굴 시간 바꾸기
채굴 시 논스를 다 사용했으면 시간을 현재로 갱신해서 다시 논스 계산


### 정책
- 개별 tx 해싱은 wallet 클래스에서 한다. 블록체인으로 전파되면 블록체인 노드가 그 tx 유효성을 검증한다.
- Output에 Receiver는 1개씩만 있어야 한다. -> Output = {A-30원, A-40원, B-...} 이면 안 된다. -> 둘을 합쳐서 Output = {A-70원, B-...} 으로 변경해야 한다.
- Input의 Receiver는 무조건 1종류
- previous block index가 block height이면 coinbase이다.


### 정리 중
판매자 publicKey
구매자는 해당 가게 주인의 public key와 상품권에 표시된 public key를 보고 위조 여부를 판단한다.
Input, Output은 각각 1명이다.
노드는 가장 긴 체인을 받아간다. -> 다른 컴퓨터로 전파된다.
포크가 돼도 10분 뒤 가장 긴 체인이 정해진다. 그리고 가장 긴 체인이 유효하다.


### 상대방 예상질문
채굴자의 수가 적으면 해킹 공격에 취약한데 해결책은? - 10분마다 블록 1개 생성된다.

TPS가 느리지 않냐 -


##### 개발 메모

uint64_t -> 블록높이, 블록개수, 금액

발행 수수료는 코인 또는 자신이 발행한 상품권으로 낼 수 있다.

Tx Input/Output 정적 할당, Tx 정적 할당, 블록부터 동적 할당




## Main Function
#### 상품권 발행 및 판매
발행권한이 있는 상품권을 발행할 수 있고, 이를 판매할 수 있다.
#### 상품권 구매
누구나 상품권 거래를 채굴할 수 있고, 이를 통해 수익을 얻을 수 있다.
#### 상품권 사용
상품권 거래와 비슷하게 판매자에게 자신의 상품권을 전송하면 된다.
#### 상품권 채굴
누구나 상품권 거래를 채굴할 수 있고, 이를 통해 수익을 얻을 수 있다.
#### 상품권 P2P 거래
개인 간에도 상품권을 온라인으로 거래할 수 있고, 해당 거래 내역이  블록체인에 기록된다.


## What's this project?
블록체인 프로젝트 `Gift Link`는 상품권의 낙전 수입 문제와 불합리함을 해결하기 위해 만들어졌다. 블록체인으로 추적이 가능해 상품권 깡, 상품권 사기, 환불 분쟁 등을 해결할 수 있고 불필요한 자원(인쇄, 유통, 발신, 환불)을 줄일 수 있다. 또 누구나 손쉽게 자신의 상품권을 발행할 수 있다.


## How to solve the problem?
본 프로젝트는 2가지 형태의 코인을 사용한다.

`GLT(Gift Link Token)` : 블록체인 생태계 참여자격을 의미하는 토큰으로서 이 토큰을 보유하고 있어야 특정 상품권을 발행할 수 있는 권한을 가진다.

`GLC(Gift Link Coin)` : 스마트 컨트랙트 기반 상품권 역할을 하는 코인으로서 상품권 발행자 또는 에스크로 은행이 가맹점에 대금을 지급하거나 서비스 혹은 상품을 제공하면 자동으로 소멸된다. 구매한 상품권(GLC)은 블록체인에 등록돼 추적이 가능하고, 이 기록으로 상품권을 반품하거나 사기 피해를 예방할 수 있다.


## Strength
상품권 제작에 드는 수수료가 적은 것이 본 프로젝트의 강점이다. 카카오톡 '선물하기'는 수수료가 15%에 달해 지나치게 높다는 불만이 계속되고 있다. 수수료가 적은 곳도 5%에 달한다. 반면 본 프로젝트의 상품권 수수료는 1%에 불과하다. 이 1%의 수입도 일정량 쌓이게 되면 발행한 코인 소각에 사용해 코인의 가치를 지속적으로 높일 계획이다.
