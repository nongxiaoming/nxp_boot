#include "mainwindow.h"
#include "ui_mainwindow.h"


#define MAX_DATA_SIZE 4096

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->PageInit ();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::PageInit ()
{
    this->step=0;
    this->finsh_size=0;
    this->HexEdit=new QHexEdit(ui->groupBox_2);
    this->HexEdit->setReadOnly(true);
    ui->gridLayout_2->addWidget (this->HexEdit);
    this->setFixedSize( this->width (),this->height ());
    this->TcpSocket = new QTcpSocket(this);
    QObject::connect(this->TcpSocket,SIGNAL(readyRead()),this,SLOT(data_Receive()));
}
void MainWindow::data_Receive ()
{
    QByteArray datagram;//用于存放接收的数据报
    datagram= this->TcpSocket->readAll();
}
void MainWindow::on_UpdateBtn_clicked()
{
 this->step=0;
 this->finsh_size=0;

 this->TcpSocket->connectToHost (*IPAddr,hostPort);

}
void MainWindow::send_data()
{
  QByteArray data;
  quint16 pack_size=0;

    switch(this->step)
    {
      case 0:
        pack_size=7;
        data.append((quint8)SOH);
        data.append((char)0x00);
        data.append((quint8)0xff);
        data.append((quint8)((total_size>>24)&&0xff));
         data.append((quint8)((total_size>>16)&&0xff));
          data.append((quint8)((total_size>>8)&&0xff));
           data.append((quint8)((total_size)&&0xff));
            this->TcpSocket->write(data,pack_size);
        break;
      case 1:
        pack_size=MAX_DATA_SIZE+3;
        data.append((quint8)STX);
        data.append(pack_num);
        data.append((quint8)(~pack_num));
        if(this->total_size-this->finsh_size<MAX_DATA_SIZE)
        {
         QByteArray fill_data;
         fill_data.resize(MAX_DATA_SIZE-(this->total_size-this->finsh_size));
         fill_data.fill(0x00);
         data.append(&m_buf[this->finsh_size],this->total_size-this->finsh_size);
         data.append(fill_data);
        }else
        {
            data.append(&m_buf[this->finsh_size],MAX_DATA_SIZE);
        }
        this->TcpSocket->write(data,pack_size);
        break;
    case 2:
        break;
     default:
        this->step=0;
        break;
    }
}
void MainWindow::on_LoadBtn_clicked()
{

    QFileDialog *fileDialog = new QFileDialog(ui->centralWidget);//创建一个QFileDialog对象，构造函数中的参数可以有所添加。
       fileDialog->setWindowTitle(tr("打开固件"));//设置文件保存对话框的标题
       fileDialog->setAcceptMode(QFileDialog::AcceptOpen);//设置文件对话框为保存模式
       fileDialog->setFileMode(QFileDialog::AnyFile);//设置文件对话框弹出的时候显示任何文件，不论是文件夹还是文件
       fileDialog->setViewMode(QFileDialog::Detail);//文件以详细的形式显示，显示文件名，大小，创建日期等信息；

       //还有另一种形式QFileDialog::List，这个只是把文件的文件名以列表的形式显示出来
       fileDialog->setGeometry(10, 30, 300, 200); //设置文件对话框的显示位置
       fileDialog->setDirectory(".");//设置文件对话框打开时初始打开的位置
       fileDialog->setNameFilter("固件文件(*.bin)");//设置文件类型过滤器
       if (fileDialog->exec() == QDialog::Accepted)
       {
           //注意使用的是QFileDialog::Accepted或者QDialog::Accepted,不是QFileDialog::Accept
           QString path = fileDialog->selectedFiles()[0];
           QFileInfo fileinfo(path);
           QFile file(path);
           file.open(QIODevice::ReadOnly);
           this->total_size = file.size();
           QDataStream in(&file);
           m_buf = new char[this->total_size];
           in.readRawData(m_buf, this->total_size);
           file.close();
           QByteArray FWdat = QByteArray(m_buf, this->total_size);

           this->HexEdit->setData (FWdat);
           ui->fwNamelabel->setText (tr("固件名称:%1").arg (fileinfo.fileName ()));
          ui->fwSizelabel->setText (tr("固件大小:%1字节").arg (QString::number (this->total_size)));
           ui->fwDate_label->setText (tr("日期:%1").arg (fileinfo.created ().toString ("yyyy-M-d H:m")));

           this->update_log_output ("读入固件信息成功,请点击\"升级\"进行升级...");

       }
}
void MainWindow::update_log_output(QString info)
{
    QString time= QDateTime::currentDateTime ().toString ("yyyy-MM-dd hh:mm:ss");
    ui->textEdit->append (tr("[%1] %2").arg (time).arg (info));
}
