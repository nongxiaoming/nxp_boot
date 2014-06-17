#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include "qhexedit.h"

namespace Ui {
class MainWindow;
}
enum Status_code {
    NONE = 0x00,
    SOH  = 0x01,
    STX  = 0x02,
    EOT  = 0x04,
    ACK  = 0x06,
    NAK  = 0x15,
    CAN  = 0x18,
    CODE_C    = 0x43,
};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
   void on_UpdateBtn_clicked();
   void on_LoadBtn_clicked();
   void data_Receive ();
private:
    Ui::MainWindow *ui;
    QHexEdit *HexEdit;
    QTcpSocket *TcpSocket;
    QHostAddress *IPAddr;
    quint16 hostPort;
    quint8 pack_num;
    char* m_buf;
    int finsh_size;
    int total_size;
    int step;
    void PageInit();
    void send_data();
    void update_log_output(QString info);
};

#endif // MAINWINDOW_H
