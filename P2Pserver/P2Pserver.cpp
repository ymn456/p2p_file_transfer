#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "P2Pserver.h"
#include "P2P_socket.h"
#include "meta.h"
#include "format_packet.h"

// 监听套接字 包含客户端连接的套接字队列，有新连接时初始化连接套接字
// 收到相应信号后，对meta增删改查
// 收到updateusername信号后，发信号updateserver
// 收到disconnected信号后，从连接套接字列表中删除该客户，并发出updateserver信号 ，main处理


P2Pserver::P2Pserver(QObject *parent, int port):QTcpServer(parent){
    this->listen(QHostAddress::Any, port);

}

P2Pserver::~P2Pserver(){}

void P2Pserver::incomingConnection(int socketDescriptor){
    Client_Socket *Client_Socket = new class Client_Socket(this);
    connect(Client_Socket,SIGNAL(Disconnected(int)),this,SLOT(slotDisconnected(int)));
    connect(Client_Socket,SIGNAL(UpdateClients(QString,int)),this,SLOT(UpdateClients(QString,int)));
    connect(Client_Socket,SIGNAL(UpdateUserInfo(QString)),this,SLOT(UpdateUserInfo(QString)));
    connect(Client_Socket,SIGNAL(UpdateUserName(QString)),this,SLOT(UpdateUserName(QString)));
    connect(Client_Socket,SIGNAL(UpdateMeta(QString)),this,SLOT(UpdateMeta(QString)));
    connect(Client_Socket,SIGNAL(SearchMeta(QString)),this,SLOT(SearchMeta(QString)));
    connect(Client_Socket, SIGNAL(ReturnMeta(QString)), this, SLOT(ReturnMeta(QString)));
    connect(Client_Socket, SIGNAL(DeleteMeta(QString)), this, SLOT(DeleteMeta(QString)));
    Client_Socket->setSocketDescriptor(socketDescriptor);
    QString name = "Unknown";
    QPair<class Client_Socket*, QString> pair(Client_Socket, name);
    Client_Socket_list.append(pair);

}

QTcpSocket* P2Pserver::find_socket(QString username){
    int i = 0;
    for(i = 0; i < Client_Socket_list.count(); ++i){
        if(Client_Socket_list.at(i).second == username){
            return Client_Socket_list.at(i).first;
        }
    }
    return NULL;
}

void P2Pserver::UpdateUserInfo(QString msg){
    int i = 0;
    int pos = i;
    while(msg.at(i++) != ';');
    QString username = msg.mid(pos, i - 1);
    pos = i;
    while(msg.at(i++) != ';');
    QString password = msg.mid(pos, i - pos - 1);
    QSqlQuery query(db);
    QTcpSocket *item = Client_Socket_list.last().first;

    qDebug() << "enter";
    query.exec("select * from login where Username='" + username + "'");
    if(query.next()){
        msg = "Server: sign error, user name exists.";
        format_packet fmsg(msg, SIGNFAILKIND);
        item->write(fmsg.fmpak.toLatin1(),fmsg.fmpak.length());
    }
    else{
        if(!query.exec("insert into login (Username, Password) values( '" + username +
                   "', '" + password + "')")){
            msg = "Server: sign error";
            format_packet fmsg(msg, SIGNFAILKIND);
            item->write(fmsg.fmpak.toLatin1(),fmsg.fmpak.length());
        }
        else {
            msg = "Server: sign success";
            format_packet fmsg(msg, SIGNSCKIND);
            item->write(fmsg.fmpak.toLatin1(),fmsg.fmpak.length());
        }
    }
}

// 发updateserver信号，发送OT msg, 有客户端下线了
//将消息广播给所有人
void P2Pserver::UpdateClients(QString msg, int length){
    emit UpdateServer(msg, length, UpdateMsg);
    for(int i = 0; i < Client_Socket_list.count(); ++i){
        QTcpSocket *item =Client_Socket_list.at(i).first;
        format_packet fmsg(msg, OTHERKIND);
        if(item->write(fmsg.fmpak.toLatin1(),length + 8) != length){
            continue;
        }
    }
}

void P2Pserver::UpdateUserName(QString msg){
    int i = 0;
    int pos = i;
    while(msg.at(i++) != ';');
    QString username = msg.mid(pos, i - 1);
    pos = i;
    while(msg.at(i++) != ';');
    QString password = msg.mid(pos, i - pos - 1);
    QSqlQuery query(db);
    QTcpSocket *item =Client_Socket_list.last().first;
    query.exec("select * from login where Username='" + username + "'");

    if(!query.next()){
        msg = "Server: login error, unknown user name.";
        format_packet fmsg(msg, ERRDCKIND);
        item->write(fmsg.fmpak.toLatin1(),fmsg.fmpak.length());
    }
    else{
        if(query.value(2).toString() != password){
            msg = "Server: login error, password is wrong.";
            format_packet fmsg(msg, ERRDCKIND);
            item->write(fmsg.fmpak.toLatin1(),fmsg.fmpak.length());
        }
        else{
            Client_Socket_list.last().second = username;
            emit UpdateServer(username, username.length(), UpdateName);
        }
    }
}

void P2Pserver::ReturnMeta(QString username){
    bool flag = false;
    QSqlQuery query(db);
    QTcpSocket *item = find_socket(username);
    if(item == NULL){
        return;
    }

    query.exec("select * from Resource where Owner='" + username + "'");
    while(query.next())
    {
        meta mt;
        mt.filename = query.value(1).toString();
        mt.size = query.value(2).toString().toLong();
        mt.filepath = query.value(3).toString();
        mt.owner = query.value(4).toString();
        mt.ip = query.value(5).toString();
        mt.port = query.value(6).toString().toLong();
        QString msg = mt.toString();
        format_packet fmsg(msg, RESKIND);
        while(item->write(fmsg.fmpak.toLatin1(),msg.length() + 8) != msg.length() + 8);
        flag = true;

    }
    if(flag == false){
        QString msg = "";
        format_packet fmsg(msg, RESKIND);
        while(item->write(fmsg.fmpak.toLatin1(),msg.length() + 8) != msg.length() + 8);
    }
}

void P2Pserver::UpdateMeta(QString mt){
    QSqlQuery query(db);
    meta mt_t(mt);
    QString query_str = "insert into Resource(Filename,Size,Filepath,Owner,IP,Port) values( ";

    query_str = query_str + "'" + mt_t.filename + "', ";
    query_str = query_str + QString::number(mt_t.size) + ", ";
    query_str = query_str + "'" + mt_t.filepath + "', ";
    query_str = query_str + "'" + mt_t.owner + "', ";
    query_str = query_str + "'" + mt_t.ip + "', ";
    query_str = query_str + QString::number(mt_t.port) + ")";
    query.exec(query_str);
}

void P2Pserver::SearchMeta(QString msg){

    int i = 0;
    int pos = i;
    while(msg.at(i++) != ';');
    QString username = msg.mid(pos, i - 1);
    pos = i;
    while(msg.at(i++) != ';');
    QString filename = msg.mid(pos, i - pos - 1);

    bool flag = false;
    QSqlQuery query(db);
    QTcpSocket *item = find_socket(username);
    if(item == NULL){
        return;
    }

    /// 严格匹配：如果希望模糊匹配
    /////query.exec("select * from Resource where FileName='" + filename + "'");

    ///如果希望模糊匹配
    query.exec("select * from Resource where FileName like '%"+ filename +"%'"  );
    while(query.next())
    {
        meta mt;
        mt.filename = query.value(1).toString();
        mt.size = query.value(2).toString().toLong();
        mt.filepath = query.value(3).toString();
        mt.owner = query.value(4).toString();
        mt.ip = query.value(5).toString();
        mt.port = query.value(6).toString().toLong();
        if(find_socket(mt.owner) != NULL){
            mt.online = "on";
        }
        else{
            mt.online = "off";
        }
        QString msg = mt.toString();
        format_packet fmsg(msg, SRHKIND);
        while(item->write(fmsg.fmpak.toLatin1(),msg.length() + 8) != msg.length() + 8);
        flag = true;
    }
    if(flag == false){
        QString msg = "";
        format_packet fmsg(msg, SRHKIND);
        while(item->write(fmsg.fmpak.toLatin1(),msg.length() + 8) != msg.length() + 8);
    }
}


void P2Pserver::DeleteMeta(QString msg){
    int i = 0;
    int pos = i;
    QSqlQuery query(db);

    while(msg.at(i++) != ';');
    QString username = msg.mid(pos, i - 1);
    qDebug() << msg;
    for(pos = i; i < msg.length(); ++i){
        if(msg.at(i) == ';'){
            QString filename = msg.mid(pos, i - pos);
            query.exec("delete from Resource where FileName='" + filename
                       + "' AND Owner='" + username + "'");
            pos = i + 1;
        }
    }
}

// 将该连接从套接字队列中删除
void P2Pserver::slotDisconnected(int descriptor){
    int i = 0;
    for(i = 0; i < Client_Socket_list.count(); ++i){
        QTcpSocket *item = Client_Socket_list.at(i).first;
        if(item->socketDescriptor() == descriptor){
            if(Client_Socket_list.at(i).second != "Unknown")
                emit UpdateServer("", i, RemoveName);
            Client_Socket_list.removeAt(i);
            break;
        }
    }
}

