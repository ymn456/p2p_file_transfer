#ifndef P2PSERVER_H
#define P2PSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QPair>
#include <QList>

#include "P2P_socket.h"
// 中心服务器
class P2Pserver: public QTcpServer{
    Q_OBJECT
public:
   P2Pserver(QObject *parent = 0,int port = 0);
   ~P2Pserver();
   QList<QPair<Client_Socket*, QString> > Client_Socket_list;
   QSqlDatabase db;

   enum MsgKind{
       UpdateName = 0,
       UPDATEMETA = 1,
       UpdateMsg = 2,
       RemoveName = 3,
   };
private:
   QTcpSocket* find_socket(QString username);
private slots:
   void UpdateUserName(QString);
   void UpdateMeta(QString);
   void SearchMeta(QString);
   void ReturnMeta(QString);
   void DeleteMeta(QString);
   void slotDisconnected(int);
   void UpdateClients(QString,int);
   void UpdateUserInfo(QString);
signals:
    void UpdateServer(QString,int, P2Pserver::MsgKind);///自定义的信号：更新中心服务器);
    void Disconnected(int);
protected:
    void incomingConnection(int socketDescriptor);
};

#endif // P2PSERVER_H
