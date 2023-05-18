#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
class server:public QTcpServer{
    Q_OBJECT

public:
    server(QObject *parent = 0 , int port = 0);
    ~server();

private slots:
    void checkdata();
    void slotDisconnected();

protected:
    void incomingConnection(int socketDescriptor);
private:
    QTcpSocket *tcp_client_socket;      /// 代表一个真实的 另外的客户端
    void transfer(QString& filepath); ///传输文件

};

#endif // SERVER_H
