#ifndef P2P_SOCKET_H
#define P2P_SOCKET_H

#include <QTcpSocket>
class Client_Socket:public QTcpSocket{
    Q_OBJECT
public:
    Client_Socket(QObject *parent = 0);
    ~Client_Socket();
signals:
    void UpdateClients(QString, int);
    void UpdateUserName(QString);
    void UpdateMeta(QString);
    void Disconnected(int);
    void ReturnMeta(QString);
    void DeleteMeta(QString);
    void SearchMeta(QString);
    void UpdateUserInfo(QString);
private slots:
    void DataReceived();
    void slotDisconnected();
};


#endif // P2P_SOCKET_H
