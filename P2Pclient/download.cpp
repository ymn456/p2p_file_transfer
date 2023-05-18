#include "download.h"
#include <QFile>
#include <QtDebug>

// 客户端向个人服务端下载模块
// 传入tm，连接目标服务端

Download::Download(QObject *parent, meta &tm)
    :QTcpSocket(parent)
{
    status = true;
    port = tm.port + 1;
    ServerIP = new QHostAddress;
    ServerIP->setAddress(tm.ip);
    UserName = tm.owner;
    Dfilepath = tm.filepath;
    Dfilename = tm.filename;
    filesize = tm.size;
}

// 连接个人服务端，连接成功后将目标文件的全路径发送给个人服务端
// 等待服务端把文件传送过来
bool Download::start_as_client(QString dirname){
    if(dirname == ""){
        return false;
    }
    Dirname = dirname;
    connect(this,SIGNAL(connected()),this,SLOT(ClientConnected()));
    connect(this,SIGNAL(disconnected()),this,SLOT(ClientDisconnected()));
    connect(this,SIGNAL(readyRead()),this,SLOT(ClientDataReceived()));

    connectToHost(*ServerIP,port);
    return true;
}

void Download::ClientConnected(){

    QString msg = "#";
    QString slen = QString::number(Dfilepath.length());//文件全路径的长度：字节数
    for(int i = slen.length(); i < 10; ++i){
        slen = "0" + slen;
    }
    msg = msg + slen + "#" + Dfilepath + "#";
    write(msg.toLatin1(), msg.length());
}



///// P2P 客户端下载文件的 核心函数
#define PAK_ERR "PKE"
#define FP_ERR  "FPE"
#define SUCC    "SUC"
void Download::ClientDataReceived(){
    if(bytesAvailable() >= 3){
        char buf[256];

        read(buf, 3);
        QString msg = buf;
        if(msg.mid(0,3) == PAK_ERR){

        }
        else if(msg.mid(0,3) == FP_ERR){

        }
        else if(msg.mid(0,3) == SUCC){
            QFile dfile(Dirname + "/" + Dfilename);
            qDebug() << "filexxx:" << (Dirname + Dfilename);
            qint64 tsz = 0;
            if(dfile.open(QIODevice::ReadWrite)){
                for(int i = 0; i < filesize / 256; ++i){
                    while(bytesAvailable() < 256){ //block
                        if(! (3000)){
                            qDebug() << "timeout";
                            disconnectFromHost();
                            status = false;
                            return;
                        }
                    }
                    read(buf, 256); //将socket中的数据字节流读取到 缓冲区
                    dfile.write(buf, 256); //写到本地文件
                    tsz = tsz + 256;
                    progress = tsz * 100 / filesize;
                    qDebug() << progress;
                }
                while(bytesAvailable() < (filesize % 256)){ //block
                    if(!waitForReadyRead(3000)){
                        qDebug() << "timeout";
                        disconnectFromHost();
                        status = false;
                        return;
                    }
                }
                read(buf, filesize % 256);
                dfile.write(buf, filesize % 256);
                progress = 100;
                qDebug() << progress;
                dfile.flush();
                dfile.close();
            }
            status = true;
            disconnectFromHost();
        }
    }
}

void Download::ClientDisconnected(){

}

Download::~Download(){
    this->close();
}
