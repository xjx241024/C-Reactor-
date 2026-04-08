all:client echoserver bankclient bankserver

bankclient:bankclient.cpp
	g++ -g -o bankclient bankclient.cpp

bankserver:bankserver.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TCPServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp	BankServer.cpp ThreadPool.cpp Timestamp.cpp
	g++ -g -o bankserver bankserver.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TCPServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp BankServer.cpp ThreadPool.cpp Timestamp.cpp -lpthread

client:client.cpp
	g++ -g -o client client.cpp
echoserver:server.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TCPServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp	EchoServer.cpp ThreadPool.cpp Timestamp.cpp
	g++ -g -o echoserver server.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TCPServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp ThreadPool.cpp Timestamp.cpp -lpthread

clean:
	rm -f client echoserver bankclient bankserver
