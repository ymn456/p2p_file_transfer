#include "P2P_socket.h"
#include <QDebug>


// 连接客户端的套接字
// 收到信息 -》 发送信号让main 或者 server 处理
// 用户端断开，发信号

Client_Socket::Client_Socket(QObject *parent):QTcpSocket(parent){
    connect(this, SIGNAL(readyRead()), this, SLOT(DataReceived()));
    connect(this, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
}

void Client_Socket::DataReceived(){

    if(bytesAvailable() > 0) {
        char buf[1024];
        read(buf, 8); //先读取8个字节

        QString head = buf;
        long msglen = 0;
        QString kind;
        if(head == "error|||"){
            return;
        }
        else{
            msglen = head.mid(2,6).toLong();
            kind = head.mid(0,2);
            read(buf,msglen);
            buf[msglen] = 0;
        }

        //解析消息类型，，发射相关的信号，即可
        //谁来处理这些信号呢？    肯定是中心服务器在统一处理
        /////接受　新的客户端连接
        /////void Server::incomingConnection(int socketDescriptor){
        QString msg = buf;
        if(kind == "NM"){ //name messsage， 有用户登录
            QString logininfo = msg;
            emit UpdateUserName(logininfo);
        }
        else if(kind == "MM"){ //meta message 有用户上传共享文件，涉及数据库
            emit UpdateMeta(msg);
         }
        else if(kind == "SM"){ //return share file message 回传该用户的共享文件列表
            QString username = msg;
            emit ReturnMeta(username);

        }
        else if(kind == "DM"){ //delete share file message 删除用户共享文件
            emit DeleteMeta(msg);
        }
        else if(kind == "SR"){ //search share file message 传回用户搜索的文件信息
            emit SearchMeta(msg);
        }
        else if(kind == "GM"){ //sign message  用户注册
            emit UpdateUserInfo(msg);
        }
        else if(kind == "OT"){ //normal message 用户登出
            emit UpdateClients(msg, msg.length());
        }
    }
}

void Client_Socket::slotDisconnected(){
    emit Disconnected(this->socketDescriptor());
}

Client_Socket::~Client_Socket(){}
