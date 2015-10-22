#ifndef NETANDUART_H
#define NETANDUART_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <QByteArray>
#include <QTimer>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QGridLayout>
#include <QWidget>
#include <QStackedLayout>
#include <QFormLayout>
#include <QDebug>
#include <QByteArray>
#include <QtWidgets>
#include <QtNetwork>


class NetAndUart : public QWidget
{
    Q_OBJECT

public:
    explicit NetAndUart(QWidget *parent = 0);
    ~NetAndUart();
    QGridLayout *mainLayout;
    QComboBox *port_index; //串口索引
    QComboBox *port_rate_index; //串口波特率索引
    QPushButton *open_button; //打开串口按键
    QLabel *timeoutLabel;
    QLineEdit *timeEdit;
    QTextBrowser *uart_print; //串口数据显示区域
    bool uartIsOpen;
    QSerialPort *serial;
    void uart_send(unsigned char *data, int len);
public:
    //net work
    QLabel *hostLabel;
    QLabel *portLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QLabel *groupLabel;
    QLineEdit *groupEdit;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
    QTextBrowser *net_print; //串口数据显示区域
    QTcpSocket socket;

    QString currentFortune;

public:
    void sendDataNet(QByteArray message);
    void netPrint(QString data, QColor c = QColor(128, 136, 100));

private:
    QTimer *timerData;
    QByteArray uartData;
    bool readAll;

private:
    void init_ui();
    void init_connect();
    void availaComPort();
    void uartPrint(QString data, QColor c = QColor(128, 136, 100));

public slots:
    void open_button_clicked();
    void begin_to_recvie();
    void timeFunction();

//网络部分
signals:
    void error(int socketError, const QString &message);
private slots:
    void requestNewFortune();
    void displayError(int socketError, const QString &message);
    void enableGetFortuneButton();
    void reciveDataNet();
    void disconnectedSlot();
};

#endif // NETANDUART_H
