#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QGridLayout>
#include <QString>
#include <QTableWidget>
#include <QTcpSocket>
#include <QFileDialog>
#include <QtNetwork/QHostAddress>
#include <QTimer>
#include <QMessageBox>
#include "meta.h"
#include "server.h"
#include "download.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    QWidget *myWidget;

    QListWidget *ContentListWidget;

    QLabel *UserNameLabel;
    QLineEdit *UserNameLineEdit;

    QLabel *UserPasswdLabel;
    QLineEdit *UserPasswdLineEdit;

    QLabel *ServerIPLabel;
    QLineEdit *ServerIPLineEdit;

    QLabel *PortLabel;
    QLineEdit *PortLineEdit;

    QPushButton *EnterBtn;
    QPushButton *SignBtn;

    QTableWidget *ShareTableWidget;
    QPushButton *FlushBtn;
    QPushButton *DisableBtn;
    QPushButton *ShareBtn;

    QTableWidget *ResultTableWidget;
    QLineEdit *SearchLineEdit;
    QPushButton *SearchBtn;
    QProgressBar *ProgressBar;
    QPushButton *DownloadBtn;
    QTimer *timer;

    QLineEdit *DirLineEdit;
    QPushButton *SelectDirBtn;

    QGridLayout *layout1;
    QGridLayout *layout2;
    QGridLayout *layout3;
    QGridLayout *mainLayout;

    QTcpSocket *tcpSocket;
    int port;
    QHostAddress *ServerIP;
    QString UserName;

    server *server;
    bool status;
    QList<Download*> tasks;

    enum MsgKind{
        NAMEMSG = 0,
        METAMSG = 1,
        DISCONNECT = 2,
        SHAREMSG = 3,
        DELETEMSG = 4,
        SEARCHMSG = 5,
        SIGNMSG = 6,
        OTHERMSG = 7
    };
    void FormatMsg(QString &msg, MsgKind kind);

public slots:
    void slotEnter();
    void slotConnected();
    void slotSendMeta();
    void slotSearch();
    void DataReceived();
    void slotFlushShare();
    void slotDeleteShare();
    void slotSelectDire();
    void slotDownload();
    void slotSignUp();
    void timeout();
    void slotDisconnected();
private:
    void UpdateResultTableWidget(QString &);
    void UpdateShareTableWidget(QString );
    //void slotDisconnected();

};
#endif // MAINWINDOW_H
