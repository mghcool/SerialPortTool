#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include "crccheck.h"


QSerialPort serial;             //串口对象
QList<QSerialPortInfo> portList;//串口列表
QLabel *lblStatus;             //状态栏状态标签
QLabel *lblRxByte;             //状态栏接收字节标签
QLabel *lblTxByte;             //状态栏发送字节标签
CrcCheck crcObj;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //创建状态标签
    lblStatus = new QLabel();
    ui->statusbar->addWidget(lblStatus);
    lblStatus->setMinimumWidth(500);
    lblStatus->setMaximumWidth(500);
    lblStatus->setStyleSheet("color:red;");
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

    //添加crc选项
    for(int i = 0; i < crcObj.modelListSize; i++)
    {
        ui->cmbCRCType->addItem(crcObj.modelList[i].name);
    }

    //更新串口列表
    UpdatePortList();

    //定义串口更新定时器
    timerUpdatePort = new QTimer(this);
    timerUpdatePort->start(1000);

    //连接信号槽
    connect(&serial, SIGNAL(readyRead()), this, SLOT(slot_PortReceive()));
    connect(timerUpdatePort, SIGNAL(timeout()), this, SLOT(slot_UpdatePort()));
}

//窗体析构
MainWindow::~MainWindow()
{
    serial.close();
    timerUpdatePort->stop();
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
void MainWindow::slot_UpdatePort()
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
        return;
    }

    if(checked)
    {
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
            QString text = " 串口打开："
                    + ui->cmbSerialPort->currentText()
                    + " "
                    + ui->comBaudRate->currentText();
            lblStatus->setStyleSheet("color:green;");
            lblStatus->setText(text);
        }
        else
        {
            switch (serial.error()) {
            case QSerialPort::PermissionError:
                QMessageBox::warning(this, "错误", "串口被占用！", QMessageBox::Ok);
                break;
            case QSerialPort::OpenError:
                QMessageBox::warning(this, "错误", "无法打开串口", QMessageBox::Ok);
                break;
            default:
                QMessageBox::warning(this, "错误", "打开串口错误！", QMessageBox::Ok);
            }
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
        serial.close();
        lblStatus->setStyleSheet("color:red;");
        lblStatus->setText(tr(" 未打开串口"));
    }
}

//发送一条信息
void MainWindow::on_btnSend_clicked()
{
    //如果串口没有打开，那就打开串口
    if(!ui->start->isChecked())
    {
        on_start_triggered(true);
        return;
    }
    //获取输入窗口sendData的数据
    QString inputText = ui->textEditTx->toPlainText();
    //转换数据
    QByteArray sendData;
    if(ui->radioTxAscii->isChecked()) sendData = inputText.toUtf8();   //按Ascii发送
    else sendData = QByteArray::fromHex (inputText.toLatin1().data()); //按Hex发送
    //CRC校验
    if(ui->cbxCRC->isChecked())
    {
        quint32 crcVal = crcObj.computeCrcVal(sendData, ui->cmbCRCType->currentIndex());
        if(ui->cbxCRCExchange->isChecked())
        {
            sendData.append(crcVal & 0x00FF);
            sendData.append(crcVal >> 8);
        }
        else
        {
            sendData.append(crcVal >> 8);
            sendData.append(crcVal & 0x00FF);
        }
    }
    // 写入发送缓存区
    serial.write(sendData);
    //添加到历史区
    if(ui->cmbSendHistory->itemText(0) != inputText)
    {
        ui->cmbSendHistory->insertItem(0, inputText);
        ui->cmbSendHistory->setCurrentIndex(0);
    }
}


//读取接收到的数据
void MainWindow::slot_PortReceive()
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

//清理接收区
void MainWindow::on_clean_triggered()
{
    ui->textShowRx->clear();
}
