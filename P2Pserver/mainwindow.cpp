#include "mainwindow.h"
#include "P2Pserver.h"

// 初始化界面，
// 登录后，监听套接字初始化并连接数据库
// 响应updateserver信号

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("P2P Server");
    ContentListWidget = new QListWidget;

    myWidget = new QWidget;
    PortLabel = new QLabel("端口:");
    PortLineEdit = new QLineEdit;
    CreateBtn = new QPushButton("打开服务器");

    resourceTree = new QTreeWidget();
    resourceTree->setHeaderLabel("在线用户");
    resourceTree->clear();

    mainLayout = new QGridLayout();

    mainLayout->addWidget(ContentListWidget,0,0,1,2);
    mainLayout->addWidget(PortLabel,1,0);
    mainLayout->addWidget(PortLineEdit,1,1);
    mainLayout->addWidget(CreateBtn,2,0,1,2);
    mainLayout->addWidget(resourceTree,0,2,3,1);
    myWidget->setLayout(mainLayout);
    setCentralWidget(myWidget);

    port = 8010;
    PortLineEdit->setText(QString::number(port));

    connect(CreateBtn, SIGNAL(clicked()),this, SLOT(slotCreateServer()));
}

void MainWindow::slotCreateServer(){
    server = new P2Pserver(this, port);

    server->db = QSqlDatabase::addDatabase("QMYSQL"); // add mysql engine
    server->db.setHostName("localhost");
    server->db.setDatabaseName("p2p");
    server->db.setUserName("root");
    server->db.setPassword("rootroot");
    if (!server->db.open()) {
        QMessageBox::critical(0, QObject::tr("无法打开数据库"),
        "无法创建数据库连接！ ", QMessageBox::Cancel);
        return;
    }

//    QSqlQuery query(server->db);

//    QString query_str = "insert into login(id, Username, Password) values(2,3,4)";

//    query.exec(query_str);
    connect(server,SIGNAL(UpdateServer(QString,int,P2Pserver::MsgKind)),this,SLOT(UpdateServer(QString,int,P2Pserver::MsgKind)));
    CreateBtn->setEnabled(false);

}

/// 更新  中心服务器：根据消息类型
void MainWindow::UpdateServer(QString msg, int length, P2Pserver::MsgKind flag){
    switch (flag) {
    case P2Pserver::UpdateMsg:
    {
        ContentListWidget->addItem(msg.left(length));
        break;
    }
    case P2Pserver::UPDATEMETA:  // 客户端更新共享文件（增加或减少
    {
        ContentListWidget->addItem(msg + ": update meta");
        break;
    }
    case P2Pserver::UpdateName: // 有用户上线了
    {
        QTreeWidgetItem *item_name = new QTreeWidgetItem(resourceTree);
        item_name->setText(0,msg);
        break;
    }
    case P2Pserver::RemoveName: // 有用户下线了
    {
        QPoint p(length,0);
        resourceTree->removeItemWidget(resourceTree->itemAt(p),0);
        delete resourceTree->itemAt(p);
        break;
    }
    }
}



MainWindow::~MainWindow()
{
    delete PortLabel;
    delete PortLineEdit;
    delete CreateBtn;
    delete ContentListWidget;
    resourceTree->clear();
    delete resourceTree;
    delete mainLayout;
    delete myWidget;
}

