#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include "crc.h"


QSerialPort serial;             //串口对象
QList<QSerialPortInfo> portList;//串口列表
QLabel *lblStatus;             //状态栏状态标签
QLabel *lblRxByte;             //状态栏接收字节标签
QLabel *lblTxByte;             //状态栏发送字节标签

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //创建状态标签
    lblStatus = new QLabel();
    ui->statusbar->addWidget(lblStatus);
    lblStatus->setMinimumWidth(500);
    lblStatus->setMaximumWidth(500);
    lblStatus->setText(" 未打开串口");

    //创建接收和发送字节数标签
    lblRxByte = new QLabel();
    ui->statusbar->addWidget(lblRxByte);
    lblRxByte->setMinimumWidth(100);
    lblRxByte->setText("Rx:");
    lblTxByte = new QLabel();
    ui->statusbar->addWidget(lblTxByte);
    lblTxByte->setMinimumWidth(100);
    lblTxByte->setText("Tx:");

    //设置默认选项
    ui->comBaudRate->setCurrentIndex(3);
    ui->comDataBits->setCurrentIndex(3);
    ui->stop->setChecked(true);

    //更新串口列表
    UpdatePortList();

    //定义串口更新定时器
    timerUpdatePort = new QTimer(this);
    timerUpdatePort->start(1000);

    //连接信号槽
    connect(&serial, &QSerialPort::readyRead, this, &MainWindow::Read_Data);
    connect(timerUpdatePort, SIGNAL(timeout()), this, SLOT(on_timeout_UpdatePort()));
}

//窗体析构
MainWindow::~MainWindow()
{
    serial.close();
    delete timerUpdatePort;
    delete ui;
}

//更新串口列表
void MainWindow::UpdatePortList()
{
    QList<QSerialPortInfo> newList = QSerialPortInfo::availablePorts();
    if(portList.size() != newList.size())
    {
        portList = newList;
        ui->cmbSerialPort->clear();
        foreach (const QSerialPortInfo &info, portList)
        {
            ui->cmbSerialPort->addItem(info.portName() + " " + info.description());
        }
    }
}

//串口更新定时器触发
void MainWindow::on_timeout_UpdatePort()
{
    UpdatePortList();
}

//打开串口
void MainWindow::on_start_triggered(bool checked)
{
    ui->start->setChecked(true);
    ui->pause->setChecked(false);
    ui->stop->setChecked(false);

    if(portList.isEmpty())
    {
        ui->start->setChecked(false);
        ui->pause->setChecked(false);
        ui->stop->setChecked(true);
        QMessageBox::warning(this, "错误", "没有相应的串口！", QMessageBox::Ok);
        qDebug("\033[31m"); //qDebug设置成红色
        qDebug()  << "Error:没有可用串口";
        qDebug("\033[30m"); //qDebug设置成黑色
        return;
    }

    if(checked)
    {
        qDebug() << "start:" << ui->cmbSerialPort->currentText();
        //设置串口名
        serial.setPortName(portList[ui->cmbSerialPort->currentIndex()].portName());
        //设置波特率
        serial.setBaudRate(ui->comBaudRate->currentText().toInt());
        //设置数据位数
        serial.setDataBits((QSerialPort::DataBits)ui->comDataBits->currentText().toInt());
        //设置奇偶校验
        switch (ui->comParity->currentIndex())
        {
            case 0: serial.setParity(QSerialPort::NoParity);   break;
            case 1: serial.setParity(QSerialPort::EvenParity); break;
            case 2: serial.setParity(QSerialPort::OddParity);  break;
            case 3: serial.setParity(QSerialPort::SpaceParity);break;
            case 4: serial.setParity(QSerialPort::MarkParity); break;
        }
        //设置停止位
        switch (ui->comStopBits->currentIndex())
        {
            case 0: serial.setStopBits(QSerialPort::OneStop);       break;
            case 1: serial.setStopBits(QSerialPort::OneAndHalfStop);break;
            case 2: serial.setStopBits(QSerialPort::TwoStop);       break;
        }
        //设置流控制
        switch (ui->comFlowControl->currentIndex())
        {
            case 0: serial.setFlowControl(QSerialPort::NoFlowControl);  break;
            case 1: serial.setFlowControl(QSerialPort::HardwareControl);break;
            case 2: serial.setFlowControl(QSerialPort::SoftwareControl);break;
        }
        //打开串口
        if(serial.open(QIODevice::ReadWrite))
        {
            QString text = " 串口打开：" + ui->cmbSerialPort->currentText();
            lblStatus->setText(text);
        }
        else
        {
            QMessageBox::warning(this, "错误", "没有相应的串口！", QMessageBox::Ok);
            qDebug("\033[31m"); //qDebug设置成红色
            qDebug()  << "Error:" << serial.error();//打印错误码
            qDebug("\033[30m"); //qDebug设置成黑色
            serial.clearError();
            ui->start->setChecked(false);
            ui->pause->setChecked(false);
            ui->stop->setChecked(true);
        }
    }

}

//暂停串口
void MainWindow::on_pause_triggered(bool checked)
{
    ui->start->setChecked(false);
    ui->pause->setChecked(true);
    ui->stop->setChecked(false);
    if(checked)
    {
        qDebug() << "pause";
    }
}

//停止串口
void MainWindow::on_stop_triggered(bool checked)
{
    ui->start->setChecked(false);
    ui->pause->setChecked(false);
    ui->stop->setChecked(true);
    if(checked)
    {
        qDebug() << "stop";
        serial.close();
        lblStatus->setText(tr("未打开串口"));
    }
}

//发送一条信息
void MainWindow::on_btnSend_clicked()
{
    QByteArray sData;
    //获取输入窗口sendData的数据
    QString Data = ui->textEditTx->toPlainText();
    if(ui->radioTxAscii->isChecked()) sData = Data.toUtf8();   //按Ascii发送
    else sData = QByteArray::fromHex (Data.toLatin1().data()); //按Hex发送
    // 写入发送缓存区
    serial.write(sData);
}


//读取接收到的数据
void MainWindow::Read_Data()
{
    if(ui->pause->isChecked()) return;  //暂停时不处理接收
    QByteArray buf;
    buf = serial.readAll();
    if(!buf.isEmpty())
    {
        QString str = QString::fromLocal8Bit(buf);  //支持中文

        if(ui->radioRxHex->isChecked())
            str = buf.toHex(' ').toUpper(); //转16进制显示，带空格大写

        if(ui->cbxWordWrap->isChecked())//换行显示
        {
            ui->textShowRx->append(str);
        }
        else                            //不换行显示
        {
            QTextCursor tc = ui->textShowRx->textCursor();
            tc.movePosition(QTextCursor::End);
            tc.insertText(str + " ");
        }
    }
    buf.clear();
}
