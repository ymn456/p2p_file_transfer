#include "mainwindow.h"


// 客户端 初始化界面，各按钮连接的槽函数声明
// 点击登录，就连接服务器，连接成功即发送namemsg即账号密码
// 收到服务端的信息则更新界面，
// 客户端发送：共享文件的mata、username和filename的searchmsg，deletemsg、username的sharemsg
// 客户端收到：resource messsage（RM）对应sharemsg，search messsage对应searchmsg

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("P2P Client");
    setGeometry(250,200,850,500);

    ContentListWidget = new QListWidget;

    myWidget = new QWidget;
    UserNameLabel = new QLabel("用户名:");
    UserNameLineEdit = new QLineEdit;
    UserPasswdLabel = new QLabel("密码:");
    UserPasswdLineEdit = new QLineEdit;
    ServerIPLabel = new QLabel("服务器IP:");
    ServerIPLineEdit = new QLineEdit("127.0.0.1");
    PortLabel = new QLabel("端口:");
    PortLineEdit = new QLineEdit;
    EnterBtn = new QPushButton("登陆");
    SignBtn = new QPushButton("注册");

    ShareTableWidget = new QTableWidget;
    ShareTableWidget->setColumnCount(6);
    QStringList tableHead;
    tableHead << "文件名" << "大小" << "文件路径" << "IP" << "端口" << "选择";
    ShareTableWidget->setHorizontalHeaderLabels(tableHead);
    FlushBtn = new QPushButton("我的共享文件");
    DisableBtn = new QPushButton("取消选中项共享");
    ShareBtn = new QPushButton("上传我的文件");

    ResultTableWidget = new QTableWidget;
    ResultTableWidget->setColumnCount(7);
    tableHead.clear();
    tableHead << "文件名" << "大小" << "文件路径" << "拥有者" << "状态" << "IP" << "选择";
    ResultTableWidget->setHorizontalHeaderLabels(tableHead);
    SearchBtn = new QPushButton("搜索");
    SearchLineEdit = new QLineEdit;
    ProgressBar = new QProgressBar;
    ProgressBar->setRange(0,100);
    DownloadBtn = new QPushButton("下载选中项");

    DirLineEdit = new QLineEdit;
    SelectDirBtn = new QPushButton("选择保存位置");

    layout1 = new QGridLayout();
    layout2 = new QGridLayout();
    layout3 = new QGridLayout();
    mainLayout = new QGridLayout();


    layout1->addWidget(ContentListWidget,0,0,1,2);
    layout1->addWidget(UserNameLabel,1,0);
    layout1->addWidget(UserNameLineEdit,1,1);
    layout1->addWidget(UserPasswdLabel,2,0);
    layout1->addWidget(UserPasswdLineEdit,2,1);
    layout1->addWidget(ServerIPLabel,3,0);
    layout1->addWidget(ServerIPLineEdit,3,1);
    layout1->addWidget(PortLabel,4,0);
    layout1->addWidget(PortLineEdit,4,1);
    layout1->addWidget(EnterBtn,5,0,1,2);
    layout1->addWidget(SignBtn,6,0,1,2);
    layout2->addWidget(ShareTableWidget,0,0,1,3);
    layout2->addWidget(FlushBtn,1,0);
    layout2->addWidget(DisableBtn,1,1);
    layout2->addWidget(ShareBtn,1,2);
    layout3->addWidget(ResultTableWidget,0,0,1,2);
    layout3->addWidget(SearchLineEdit,1,0);
    layout3->addWidget(SearchBtn,1,1);
    layout3->addWidget(ProgressBar,2,0,1,2);
    layout3->addWidget(DirLineEdit,3,0);
    layout3->addWidget(SelectDirBtn,3,1);
    layout3->addWidget(DownloadBtn,4,0,1,2);


    mainLayout->addLayout(layout1,0,0,2,2);
    mainLayout->addLayout(layout2,0,2,1,1);
    mainLayout->addLayout(layout3,1,2,1,1);
    myWidget->setLayout(mainLayout);

    timer = new QTimer();

    ServerIP = new QHostAddress;
    port = 8010;
    status = false;
    PortLineEdit->setText(QString::number(port));


    setCentralWidget(myWidget);

    connect(EnterBtn, SIGNAL(clicked()), this, SLOT(slotEnter()));
    connect(SignBtn,SIGNAL(clicked()),this,SLOT(slotSignUp()));

    connect(ShareBtn,SIGNAL(clicked()),this,SLOT(slotSendMeta()));
    connect(FlushBtn, SIGNAL(clicked()), this, SLOT(slotFlushShare()));
    connect(DisableBtn, SIGNAL(clicked()), this, SLOT(slotDeleteShare()));
    connect(SearchBtn,SIGNAL(clicked()),this,SLOT(slotSearch()));
    connect(SelectDirBtn,SIGNAL(clicked()),this,SLOT(slotSelectDire()));
    connect(DownloadBtn,SIGNAL(clicked()),this,SLOT(slotDownload()));

    connect(timer,SIGNAL(timeout()),this,SLOT(timeout()));

    ShareBtn->setEnabled(false);
    FlushBtn->setEnabled(false);
    DisableBtn->setEnabled(false);
    SearchBtn->setEnabled(false);
    DownloadBtn->setEnabled(false);

    tcpSocket = NULL;
    server = NULL;
}

/// 定时器负责 进度条
void MainWindow::timeout(){
    if(tasks.count() == 0){
        timer->stop();
        return;
    }
    ProgressBar->setValue(tasks.at(0)->progress);
    if(tasks.at(0)->progress == 100 || tasks.at(0)->status == false){
        if(tasks.at(0)->status == false)
            ProgressBar->setValue(0);
        delete tasks.at(0);
        tasks.removeAt(0);
    }
}

void MainWindow::slotEnter(){
    if(!status){ // 登录
        QString ip = ServerIPLineEdit->text();
        if(!ServerIP->setAddress(ip)){
            QMessageBox::information(this,"wrong","server ip error");
            return ;
        }
        if(UserNameLineEdit->text() == ""){
            QMessageBox::information(this,"wrong","user name error");
            return ;
        }
        UserName = UserNameLineEdit->text();

        tcpSocket = new QTcpSocket(this);
        connect(tcpSocket,SIGNAL(connected()),this,SLOT(slotConnected()));
        connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));
        connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(DataReceived()));

        /// 主动去连接：：中心服务器
        tcpSocket->connectToHost(*ServerIP,port);
        status = true;
    }
    else{ // 登出
        int length = 0;
        QString msg = UserName + ": leave chat room";
        FormatMsg(msg, OTHERMSG);
        if((length = tcpSocket->write(msg.toLatin1(),msg.length())) != msg.length()){
            return;
        }
        tcpSocket->disconnectFromHost();
        status = false;
        delete server;
    }
}

void MainWindow::slotDisconnected(){
    EnterBtn->setText("登陆");
    ShareBtn->setEnabled(false);
    FlushBtn->setEnabled(false);
    DisableBtn->setEnabled(false);
    SearchBtn->setEnabled(false);
    DownloadBtn->setEnabled(false);
    SignBtn->setEnabled(true);
}


void MainWindow::slotSignUp(){
    if(!status){
        QString ip = ServerIPLineEdit->text();
        if(!ServerIP->setAddress(ip)){
            QMessageBox::information(this,"wrong","server ip error");
            return ;
        }
        if(UserNameLineEdit->text() == ""){
            QMessageBox::information(this,"wrong","user name error");
            return ;
        }
        if(UserPasswdLineEdit->text() == ""){
            QMessageBox::information(this,"wrong","pass word error");
            return ;
        }
        UserName = UserNameLineEdit->text();
        QString passwd = UserPasswdLineEdit->text();

        tcpSocket = new QTcpSocket(this);
        connect(tcpSocket,SIGNAL(connected()),this,SLOT(SignConnected()));
        connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(SignDisconnected()));
        connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(SignDataReceived()));

        tcpSocket->connectToHost(*ServerIP,port);
        status = true;
    }
}


///连接中心服务器，连接后发送账号密码给服务器,并开启自身的服务端
void MainWindow::slotConnected(){
    EnterBtn->setText("注销");
    ShareBtn->setEnabled(true);
    FlushBtn->setEnabled(true);
    DisableBtn->setEnabled(true);
    SearchBtn->setEnabled(true);
    DownloadBtn->setEnabled(true);
    SignBtn->setEnabled(false);

    /// 此时，开启自身的“服务监听模式”：P2P文件服务器
    server = new class server(this, port + 1);

    int length = 0;
    QString passwd = UserPasswdLineEdit->text();
    QString msg = UserName + ";" + passwd + ";";
    FormatMsg(msg, NAMEMSG); //所有的消息 统一格式化

    /// 发送出去，即可
    if((length = tcpSocket->write(msg.toLatin1(),msg.length())) != msg.length()){
        return;
    }
}

void MainWindow::slotSendMeta(){
    QString filename = QFileDialog::getOpenFileName(this,"选择要共享的文件","/","files (*)");
    QFileInfo info(filename);
    meta mt;
    if(filename == ""){
        return;
    }

    mt.filename = info.fileName();
    mt.size = info.size();
    mt.ip = tcpSocket->localAddress().toString();
    mt.owner = UserName;
    mt.port = port;
    mt.filepath = info.filePath();
    QString msg = mt.toString();
    FormatMsg(msg, METAMSG);
    tcpSocket->write(msg.toLatin1(),msg.length());

}

void MainWindow::slotSearch(){
    QString filename = SearchLineEdit->text();
    if(filename == ""){
        return ;
    }

    QString msg = UserName + ";" + filename + ";";
    FormatMsg(msg, SEARCHMSG);
    tcpSocket->write(msg.toLatin1(),msg.length());
}

void MainWindow::slotFlushShare(){
    QString msg = UserName ;
    FormatMsg(msg, SHAREMSG);
    tcpSocket->write(msg.toLatin1(), msg.length());
}

void MainWindow::slotSelectDire(){
    QString dirname = QFileDialog::getExistingDirectory(this,"选择文件保存位置","/");
    DirLineEdit->setText(dirname);
}

void MainWindow::slotDownload(){
    QString dirname = DirLineEdit->text();
    if(dirname == ""){
        return;
    }
    int rows = ResultTableWidget->rowCount();
    bool flag = false;
    for(int i = 0; i < rows; ++i){
        if(ResultTableWidget->item(i,6)->checkState() == Qt::Checked
                && ResultTableWidget->item(i,4)->text() == "在线"){
            meta tm;
            tm.filename = ResultTableWidget->item(i,0)->text();
            tm.size = ResultTableWidget->item(i,1)->text().toLong();
            tm.filepath = ResultTableWidget->item(i,2)->text();
            tm.owner = ResultTableWidget->item(i,3)->text();
            tm.ip = ResultTableWidget->item(i,5)->text();
            tm.port = port;

            tasks.append(new Download(this, tm));
            tasks.last()->start_as_client(dirname); //开启客户端
            if(flag == false)
                timer->start(100);
            flag = true;
        }
    }
}

void MainWindow::slotDeleteShare(){
    QString msg = UserName + ";";
    int rows = ShareTableWidget->rowCount();
    int length = 0;

    for(int i = 0; i < rows; ++i){
        if(ShareTableWidget->item(i,5)->checkState() == Qt::Checked){
            msg = msg + ShareTableWidget->item(i,0)->text() + ";";
        }
    }
    FormatMsg(msg, DELETEMSG);
    if((length = tcpSocket->write(msg.toLatin1(),msg.length())) != msg.length()){
        return;
    }
}

void MainWindow::DataReceived(){
    int flag = 0;
    while (tcpSocket->bytesAvailable() > 8) {
        char buf[1024];
        tcpSocket->read(buf,8);

        QString head = buf;
        long msglen = 0;
        QString kind;
        if(head == "error|||"){
            return;
        }
        else{
            msglen = head.mid(2,6).toLong();
            kind = head.mid(0,2);
            tcpSocket->read(buf,msglen);
            buf[msglen] = 0;
        }

        QString msg = buf;
        if(kind == "RK"){ //resource messsage
            if(flag == 0){
                ShareTableWidget->clear();
                ShareTableWidget->setRowCount(0);
                QStringList tableHead;
                tableHead << "文件名" << "大小" << "文件路径" << "IP" << "端口" << "选择";
                ShareTableWidget->setHorizontalHeaderLabels(tableHead);
            }
            flag = 1;
            UpdateShareTableWidget(msg);


        }
        else if(kind == "SR"){ //search messsage
            if(flag == 0){
                ResultTableWidget->clear();
                ResultTableWidget->setRowCount(0);
                QStringList tableHead;
                tableHead.clear();
                tableHead << "文件名" << "大小" << "文件路径" << "拥有者" << "状态" << "IP" << "选择";
                ResultTableWidget->setHorizontalHeaderLabels(tableHead);
            }
            flag = 1;
            UpdateResultTableWidget(msg);
        }

        else if(kind == "OT"){ //normal message
            ContentListWidget->addItem(msg);
        }
    }
}

void MainWindow::UpdateShareTableWidget(QString msg){
    if(msg == ""){
        return;
    }

    meta tm(msg);
    int rows = ShareTableWidget->rowCount();

    ShareTableWidget->setRowCount(rows + 1);
    QTableWidgetItem *item = new QTableWidgetItem(tm.filename);
    item->setTextAlignment(Qt::AlignCenter);
    ShareTableWidget->setItem(rows, 0, item);
    item = new QTableWidgetItem(QString::number(tm.size));
    item->setTextAlignment(Qt::AlignCenter);
    ShareTableWidget->setItem(rows, 1, item);
    item = new QTableWidgetItem(tm.filepath);
    item->setTextAlignment(Qt::AlignCenter);
    ShareTableWidget->setItem(rows, 2, item);
    item = new QTableWidgetItem(tm.ip);
    item->setTextAlignment(Qt::AlignCenter);
    ShareTableWidget->setItem(rows, 3, item);
    item = new QTableWidgetItem(QString::number(tm.port));
    item->setTextAlignment(Qt::AlignCenter);
    ShareTableWidget->setItem(rows, 4, item);
    item = new QTableWidgetItem("check");
    item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    item->setCheckState(Qt::Unchecked);
    item->setTextAlignment(Qt::AlignCenter);
    ShareTableWidget->setItem(rows, 5, item);
}

//head is 8 bits
void MainWindow::FormatMsg(QString &msg, MsgKind kind){
    qint64 len = msg.length();
    QString s = QString::number(len);
    if(len > 100000){
        msg = "error|||";
        return;
    }
    for(int i = s.length(); i < 6; ++i){
        s = "0" + s;
    }
    switch (kind) {
    case NAMEMSG:
        msg = "NM" + s + msg;
        break;
    case METAMSG:
        msg = "MM" + s + msg;
        break;
    case DISCONNECT:
        msg = "DC" + s + msg;
        break;
    case SHAREMSG:
        msg = "SM" + s + msg;
        break;
    case DELETEMSG:
        msg = "DM" + s + msg;
        break;
    case SEARCHMSG:
        msg = "SR" + s + msg;
        break;
    case SIGNMSG:
        msg = "GM" + s + msg;
        break;
    case OTHERMSG:
        msg = "OT" + s + msg;
        break;
    }
}

void MainWindow::UpdateResultTableWidget(QString &msg){
    if(msg == ""){
        return;
    }

    meta tm(msg);
    int rows = ResultTableWidget->rowCount();

    ResultTableWidget->setRowCount(rows + 1);
    QTableWidgetItem *item = new QTableWidgetItem(tm.filename);
    item->setTextAlignment(Qt::AlignCenter);
    ResultTableWidget->setItem(rows, 0, item);
    item = new QTableWidgetItem(QString::number(tm.size));
    item->setTextAlignment(Qt::AlignCenter);
    ResultTableWidget->setItem(rows, 1, item);
    item = new QTableWidgetItem(tm.filepath);
    item->setTextAlignment(Qt::AlignCenter);
    ResultTableWidget->setItem(rows, 2, item);
    item = new QTableWidgetItem(tm.owner);
    item->setTextAlignment(Qt::AlignCenter);
    ResultTableWidget->setItem(rows, 3, item);
    if(tm.online == "on")
        item = new QTableWidgetItem("在线");
    else
        item = new QTableWidgetItem("离线");
    item->setTextAlignment(Qt::AlignCenter);
    ResultTableWidget->setItem(rows, 4, item);
    item = new QTableWidgetItem(tm.ip);
    item->setTextAlignment(Qt::AlignCenter);
    ResultTableWidget->setItem(rows, 5, item);
    item = new QTableWidgetItem("check");
    item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    item->setCheckState(Qt::Unchecked);
    item->setTextAlignment(Qt::AlignCenter);
    ResultTableWidget->setItem(rows, 6, item);

}


MainWindow::~MainWindow()
{
    delete PortLabel;
    delete PortLineEdit;
    delete EnterBtn;
    delete SignBtn;
    delete UserNameLabel;
    delete UserNameLineEdit;
    delete UserPasswdLabel;
    delete UserPasswdLineEdit;
    delete ServerIPLabel;
    delete ServerIPLineEdit;
    delete ContentListWidget;
    delete ShareTableWidget;
    delete DisableBtn;
    delete FlushBtn;
    delete ResultTableWidget;
    delete SearchLineEdit;
    delete SearchBtn;
    delete DownloadBtn;
    delete timer;
    delete ProgressBar;
    delete SelectDirBtn;
    delete DirLineEdit;
    delete layout1;
    delete layout2;
    delete layout3;
    delete mainLayout;
    delete myWidget;
    delete ServerIP;
}

