#include "server.h"
#include <QFile>
#include <QFileDialog>


// 个人服务端接受个人客户端连接、
// 收到信息时，信息的格式为 #长度#msg#
// 发送回应码
// 如信息的格式正确，则将对方要求的文件发送过去
server::server(QObject *parent, int port)
    :QTcpServer(parent){

    tcp_client_socket = NULL;
    listen(QHostAddress::Any, port);
}

void server::incomingConnection(int socketDescriptor){
   tcp_client_socket = new QTcpSocket(this);
   connect(tcp_client_socket, SIGNAL(readyRead()), this, SLOT(checkdata()));
   connect(tcp_client_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
   tcp_client_socket->setSocketDescriptor(socketDescriptor);
}

//#len#filepath#
#define PAK_ERR "PKE"
#define FP_ERR  "FPE"
#define SUCC    "SUC"
void server::checkdata(){

    if(tcp_client_socket->bytesAvailable() >= 11){

        //tcp:字节流，不知道对方发送了多少字节数？怎么办？一般：消息头部给出字节数
        char buf[1024];
        tcp_client_socket->read(buf,11);///读取11个字节
        QString msg;

        if(buf[0] != '#'){
            msg = PAK_ERR;
            tcp_client_socket->write(msg.toLatin1(), msg.length());
            return;
        }

        msg = buf;
        long len = msg.mid(1, 10).toLong();
        tcp_client_socket->read(buf,len + 2);

        msg = buf;
        QString filepath = msg.mid(1, len);///文件的全路径
        transfer(filepath);
    }

}


/// P2P文件服务器 给 客户端 发送文件
void server::transfer(QString& filepath){///传输文件
    QFile file(filepath);///先打开本地文件
    QFileInfo info(file);
    qint64 filesize = info.size();
    QString msg;

    if(file.open(QIODevice::ReadOnly)){
        char buf[257];
        msg = SUCC;

        ///发送给了“对端”
        tcp_client_socket->write(msg.toLatin1(), msg.length());
        for(int i = 0; i < filesize / 256; ++i) {
            file.read(buf, 256);
            qDebug() << tcp_client_socket->write(buf, 256);
            qDebug() << tcp_client_socket->waitForBytesWritten(3000);
        }
        file.read(buf, filesize % 256);
        qDebug() << tcp_client_socket->write(buf, filesize % 256);
        qDebug() <<  tcp_client_socket->waitForBytesWritten(3000);
    }
    else{
        msg = FP_ERR;
        tcp_client_socket->write(msg.toLatin1(), msg.length());
    }
}

void server::slotDisconnected(){

}

server::~server(){
    if(tcp_client_socket != NULL)
        delete tcp_client_socket;
}
