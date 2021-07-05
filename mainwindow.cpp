#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("串口调试助手");

    QLabel *statusLabel;
    statusLabel = new QLabel;
    statusLabel->setMinimumSize(150, 20); // 设置标签最小大小
    //statusLabel->setFrameShape(QFrame::WinPanel); // 设置标签形状
    statusLabel->setFrameShadow(QFrame::Sunken); // 设置标签阴影
    ui->statusbar->addWidget(statusLabel);
    statusLabel->setText(tr("串口调试助手准备就绪！"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

