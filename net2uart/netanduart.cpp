#include "netanduart.h"
#include <QWidget>
#include <QRgb>
NetAndUart::NetAndUart(QWidget *parent) :
    QWidget(parent)
{
    //ui->setupUi(this);
    serial = NULL;
    readAll = true;
    timerData = new QTimer();

    init_ui();
    init_connect();

}

NetAndUart::~NetAndUart()
{
    if (serial) {
        serial->close();
        delete serial;
    }

    if(socket.isOpen()) {
        socket.close();
    }

    delete mainLayout;
}

void NetAndUart::init_ui()
{
    port_index = new QComboBox;
    availaComPort();
    uartIsOpen = false;

    port_rate_index = new QComboBox;
    //port_rate_index->addItem("波特率选择");
    port_rate_index->addItem(tr("9600"));
    port_rate_index->addItem(tr("19200"));
    port_rate_index->addItem(tr("115200"));

    open_button = new QPushButton("打开串口");
    timeoutLabel = new QLabel(tr("串口数据报间隔时间（ms）"));
    timeEdit = new QLineEdit;
    timeEdit->setText(tr("1000"));
    timeEdit->setValidator(new QIntValidator(1, 5000, this));
    uart_print = new QTextBrowser;


    //网络相关的
    hostLabel = new QLabel(tr("Server ip:"));
    portLabel = new QLabel(tr("Server port:"));
    hostLineEdit = new QLineEdit;
    hostLineEdit->setText("10.2.0.71");
    portLineEdit = new QLineEdit;
    portLineEdit->setText("8787");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    groupLabel = new QLabel(tr("group:"));
    groupEdit =  new QLineEdit;
    groupEdit->setText("1");
    groupEdit->setValidator(new QIntValidator(1, 255, this));

    getFortuneButton = new QPushButton(tr("开始连接"));
    getFortuneButton->setDefault(true);
    //getFortuneButton->setEnabled(false);

    net_print = new QTextBrowser;
/*
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
*/
    mainLayout = new QGridLayout;

    mainLayout->addWidget(port_index,      0, 0, 1, 1);
    mainLayout->addWidget(port_rate_index, 0, 1, 1, 1);
    mainLayout->addWidget(open_button,     0, 2, 1, 1);
    mainLayout->addWidget(timeoutLabel,    0, 3, 1, 1);
    mainLayout->addWidget(timeEdit,        0, 4, 1, 1);
    mainLayout->addWidget(uart_print,      1, 0, 3, 4);

    mainLayout->addWidget(hostLabel,       5, 0, 1, 1);
    mainLayout->addWidget(hostLineEdit,    5, 1, 1, 1);
    mainLayout->addWidget(portLabel,       5, 2, 1, 1);
    mainLayout->addWidget(portLineEdit,    5, 3, 1, 1);
    mainLayout->addWidget(getFortuneButton,5, 4, 1, 1);
    mainLayout->addWidget(groupLabel,      6, 0, 1, 1);
    mainLayout->addWidget(groupEdit,       6, 1, 1, 1);
    mainLayout->addWidget(net_print,       7, 0, 3, 4);

    this->setLayout(mainLayout);

}

void NetAndUart::init_connect()
{
    //打开串口，根据参数，初始化串口
    connect(open_button, SIGNAL(clicked()), this, SLOT(open_button_clicked()));
    connect(timerData, SIGNAL(timeout()), this, SLOT(timeFunction()));
    //网络相关接口
    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(getFortuneButton, SIGNAL(clicked()), this, SLOT(requestNewFortune()));

}

//打开串口
void NetAndUart::open_button_clicked()
{
    QString portName;
    int btl;

    if (!uartIsOpen) {
        btl = port_rate_index->currentText().toInt();
        portName = port_index->currentText();

        if(serial){
            serial->close();
            delete serial;
            serial = NULL;
        }

        serial  = new QSerialPort(portName);

        serial->setBaudRate(btl);  //波特率
        serial->setDataBits(QSerialPort::Data8); //数据位
        serial->setParity(QSerialPort::NoParity);    //无奇偶校验
        serial->setStopBits(QSerialPort::OneStop);   //停止位1
        serial->setFlowControl(QSerialPort::NoFlowControl);  //无控制

        if (!serial->open(QIODevice::ReadWrite)) {
            uartPrint((QString)("open failed"));
            return;
        }
        connect(serial, SIGNAL(readyRead()), this, SLOT(begin_to_recvie()));
        uartIsOpen = true;
        open_button->setText("关闭串口");
    } else {
        uartIsOpen = false;
        open_button->setText("打开串口");
        if(serial){
            serial->close();
            delete serial;
            serial = NULL;
        }
    }
}

void NetAndUart::availaComPort()
{
    QList<QSerialPortInfo>  infos = QSerialPortInfo::availablePorts();
    if(infos.isEmpty())
    {
        port_index->addItem("没有可用串口!");
        return;
    }

    foreach (QSerialPortInfo info, infos) {
        port_index->addItem(info.portName());
    }
}

void NetAndUart::uartPrint(QString data, QColor c)
{
    uart_print->setTextColor(c);
    uart_print->setFontPointSize(12);
    uart_print->append(data);
}

void NetAndUart::begin_to_recvie()
{
    int time = 1000;
    if (timerData->isActive()) {
        timerData->stop();
    }
    if (!timeEdit->text().isEmpty()) {
        time = timeEdit->text().toInt();
    }

    timerData->start(time);

    if (readAll) {
        uartData = serial->readAll();
        readAll = false;
    } else {
        uartData.append(serial->readAll());
    }
     qDebug("read uart data");
    return;
}

void NetAndUart::timeFunction()
{
    QString buf = "recive:";
    QString buf2 = "hex:";
    if (!readAll) {
        buf += QString(uartData);
        sendDataNet(uartData);
        uartPrint(buf, QColor(255,0,0));
        buf2 += uartData.toHex();
        uartPrint(buf2, QColor(255,0,0));
    }
    readAll = true;
}

void NetAndUart::uart_send(unsigned char *data, int len)
{
    int sendLen = 0;
    QString buf = "send:",tmp;

    if (!serial) {
        uartPrint((QString)"串口没有打开", QColor(255,0,0));
        return;
    }

    for (int i = 0; i < len; i++) {
        tmp.sprintf("%02x ", data[i]);
        buf.append(tmp);
    }

    uartPrint(buf, QColor(0,0,255));

    sendLen = serial->write((char *)data, len);
    serial->flush();
    if (sendLen != len) {
        uartPrint((QString)"串口发送数据错误", QColor(255,0,0));
    }
}

/*
 *网络相关接口
 *
 *
 */

void NetAndUart::netPrint(QString data, QColor c)
{
    net_print->setTextColor(c);
    net_print->setFontPointSize(12);
    net_print->append(data);
}

void NetAndUart::requestNewFortune()
{
    const int Timeout = 5 * 1000;
    QString serverName = hostLineEdit->text();
    quint16 serverPort = portLineEdit->text().toInt();

    if (0 == getFortuneButton->text().compare(tr("开始连接"))) {
        if (groupEdit->text().isEmpty()) {
            uartPrint((QString)("group is null!"));
            return;
        }

        socket.connectToHost(serverName, serverPort);
        if (!socket.waitForConnected(Timeout)) {
            emit error(socket.error(), socket.errorString());
            return;
        }

        connect(&socket, SIGNAL(readyRead()), this, SLOT(reciveDataNet()));
        connect(&socket, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
        sendDataNet(groupEdit->text().toLatin1());
        getFortuneButton->setText(tr("断开连接"));
    } else {
        if(socket.isOpen()) {
            socket.close();
            netPrint("关闭网络端口");
        }
        getFortuneButton->setText(tr("开始连接"));
    }
}

void NetAndUart::enableGetFortuneButton()
{
    bool enable(!hostLineEdit->text().isEmpty() && !portLineEdit->text().isEmpty());
    getFortuneButton->setEnabled(enable);
    getFortuneButton->setText("开始连接");
}

void NetAndUart::displayError(int socketError, const QString &message)
{
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Blocking Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Blocking Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Blocking Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(message));
    }

    getFortuneButton->setEnabled(true);
}

void NetAndUart::sendDataNet(QByteArray message)
{
    QByteArray buf = "send:";
    QByteArray buf2 = "hex:";

    if (socket.isOpen()) {
        socket.write(message);
    }

    buf += message;
    netPrint(buf, QColor(0,0,255));
    buf2 += message.toHex();
    netPrint(buf2, QColor(0,0,255));
}

void NetAndUart::reciveDataNet()
{
    QString currentFortune, buf = "recive:";
    QByteArray buf2 = "hex:";

    if (socket.bytesAvailable() < (int)sizeof(quint16)) {
        return;
    }

    currentFortune.append(socket.readAll());


    buf += currentFortune;
    netPrint(buf, QColor(0,100,255));
    buf2 += currentFortune.toLocal8Bit().toHex();
    netPrint(buf2, QColor(0,100,255));
}

void NetAndUart::disconnectedSlot()
{
    socket.close();
    getFortuneButton->setEnabled(true);
    getFortuneButton->setText("打开链接");
}
