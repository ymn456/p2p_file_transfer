#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QTreeWidgetItem>
#include <QObject>
#include <QMessageBox>
#include "P2Pserver.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slotCreateServer();
    void UpdateServer(QString, int, P2Pserver::MsgKind);

private:
    QWidget *myWidget;
    QListWidget *ContentListWidget;
    QLabel *PortLabel;
    QLineEdit *PortLineEdit;
    QPushButton *CreateBtn;
    QTreeWidget *resourceTree;
    QGridLayout *mainLayout;
    P2Pserver *server;
    int port;
};
#endif // MAINWINDOW_H
