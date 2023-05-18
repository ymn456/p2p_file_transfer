#ifndef DOWNLOAD_H
#define DOWNLOAD_H

// 客户端接收服务端发来的文件 tm目标下载文件信息
// start_as_client 传入保存位置
// 连接到服务端-》发送#长度#文件全路径#
// 收到数据-》

#include <QHostAddress>
#include <QTcpSocket>
#include "meta.h"

class Download : public QTcpSocket
{
    Q_OBJECT

public:
    Download(QObject *parent, meta& tm);
    ~Download();

    qint64 progress;
    bool status;

    bool start_as_client(QString dirname);

private:
    int port;
    QHostAddress *ServerIP;
    QString UserName;
    QString Dirname;
    QString Dfilepath;
    QString Dfilename;
    qint64 filesize;

private slots:
    void ClientConnected();
    void ClientDisconnected();
    void ClientDataReceived();

};


#endif // DOWNLOAD_H
