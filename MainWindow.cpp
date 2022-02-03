#include <string>
#include <vector>
#include <stdio.h>
#include <QtGui>
#include <QtWidgets/QAction>
#include <math.h>
#include <complex.h>
#include "MainWindow.h"
#include "MainWindow.h"
#include "ui_QtBase-001.h"
#include "Udp.h"
#include <QUdpSocket>

#define VERSION "QT5 Scope"
#define FFT_POINTS 1024
#define RX_BUF_SIZE 1024

extern "C" void start_server_stream();
extern "C" void update_pitaya_cf(int);
extern "C" void update_pitaya_sr(int);
extern "C" void update_pitaya_ar(int);
extern "C" void update_pitaya_demod(int);
extern "C" void update_pitaya_rfg(int);
extern "C" void update_pitaya_afg(int);
extern "C" void update_mir_gr(int);
extern "C" void update_mir_dab_notch(int);
extern "C" void update_mir_bc_notch(int);
extern "C" void update_mir_lna(int);


extern bool stream_flag;
extern int fft_video_buf[];
extern int status[];

//QUdpSocket *socket; // = nullptr;
//---

Udp fido;


MainWindow::MainWindow(const QString cfgfile, QWidget *parent) :
    QMainWindow(parent),    ui(new Ui::MainWindow)
{

printf("Satring %d \n",__LINE__);

fido.soxit();

printf("After %d \n",__LINE__);
//Udp->sendgram();

printf("After %d \n",__LINE__);

ui->setupUi(this);
setWindowTitle(QString(VERSION));

QObject::connect(ui->setupRsp, SIGNAL(clicked()), this, SLOT(hardware_setup()));
 
connect(ui->freqCtrl, SIGNAL(newFrequency(qint64)), this, SLOT(setNewFrequency(qint64)));
connect(ui->alpha_plotter, SIGNAL(newFrequency(qint64)), this, SLOT(setNewFrequency(qint64)));

connect(ui->dsb, SIGNAL(clicked()), this, SLOT(set_dsb()));
connect(ui->usb, SIGNAL(clicked()), this, SLOT(set_usb()));
connect(ui->lsb, SIGNAL(clicked()), this, SLOT(set_lsb()));

connect(ui->sr0, SIGNAL(clicked()), this, SLOT(set_sr0()));
connect(ui->sr1, SIGNAL(clicked()), this, SLOT(set_sr1()));
connect(ui->sr2, SIGNAL(clicked()), this, SLOT(set_sr2()));
connect(ui->sr3, SIGNAL(clicked()), this, SLOT(set_sr3()));
connect(ui->sr4, SIGNAL(clicked()), this, SLOT(set_sr4()));
connect(ui->sr5, SIGNAL(clicked()), this, SLOT(set_sr5()));

connect(ui->ar0, SIGNAL(clicked()), this, SLOT(set_ar0()));
connect(ui->ar1, SIGNAL(clicked()), this, SLOT(set_ar1()));
connect(ui->ar2, SIGNAL(clicked()), this, SLOT(set_ar2()));
connect(ui->ar3, SIGNAL(clicked()), this, SLOT(set_ar3()));

//connect(ui->ip0, SIGNAL(clicked()), this, SLOT(set_ip0()));
//connect(ui->ip1, SIGNAL(clicked()), this, SLOT(set_ip1()));
//connect(ui->ip2, SIGNAL(clicked()), this, SLOT(set_ip2()));
//connect(ui->ip3, SIGNAL(clicked()), this, SLOT(set_ip3()));
//connect(ui->ip4, SIGNAL(clicked()), this, SLOT(set_ip4()));
//connect(ui->ip5, SIGNAL(clicked()), this, SLOT(set_ip5()));

connect(ui->rf_gain, SIGNAL(valueChanged(int)), this, SLOT(set_rfg(int)));
connect(ui->af_gain, SIGNAL(valueChanged(int)), this, SLOT(set_afg(int)));

connect(ui->mir_gr, SIGNAL(valueChanged(int)), this, SLOT(set_mir_gr(int)));

connect(ui->mir_dab_notch, SIGNAL(stateChanged(int)), this, SLOT(set_mir_dab_n(int)));
connect(ui->mir_bc_notch, SIGNAL(stateChanged(int)), this, SLOT(set_mir_bc_n(int)));
connect(ui->mir_lna, SIGNAL(stateChanged(int)), this, SLOT(set_mir_lna(int)));

fft_timer = new QTimer(this);
connect(fft_timer, SIGNAL(timeout()), this, SLOT(show_enable()));
fft_timer->start(200); //milli secs

ui->Bar->setValue(128); // = 128;
}

//---

MainWindow::~MainWindow()
{
fft_timer->stop();
delete fft_timer;
delete ui;
}

//---

void MainWindow::hardware_setup()
{
printf("hardware setup started... \n");
start_server_stream();
}	

void MainWindow::setNewFrequency(qint64 newfreq)
{
int send_cf = newfreq;	


ui->freqCtrl->setFrequency(newfreq);

ui->alpha_plotter->setCenterFreq(send_cf);


printf(" Set New freq: %d \n ",send_cf);
update_pitaya_cf(send_cf);
}

void MainWindow::set_rfg(int gain)
{
update_pitaya_rfg(gain);
}

void MainWindow::set_afg(int gain)
{
update_pitaya_afg(gain);
}

void MainWindow::set_mir_gr(int gain)
{
update_mir_gr(gain);
}

void MainWindow::set_sr0(){ update_pitaya_sr(0);}
void MainWindow::set_sr1(){ update_pitaya_sr(1);}
void MainWindow::set_sr2(){ update_pitaya_sr(2);}
void MainWindow::set_sr3(){ update_pitaya_sr(3);}
void MainWindow::set_sr4(){ update_pitaya_sr(4);}
void MainWindow::set_sr5(){ update_pitaya_sr(5);}

void MainWindow::set_ar0(){ update_pitaya_ar(0);}
void MainWindow::set_ar1(){ update_pitaya_ar(1);}
void MainWindow::set_ar2(){ update_pitaya_ar(2);}
void MainWindow::set_ar3(){ update_pitaya_ar(3);}

void MainWindow::set_dsb(){ update_pitaya_demod(1);}
void MainWindow::set_usb(){ update_pitaya_demod(2);}
void MainWindow::set_lsb(){ update_pitaya_demod(3);}


void MainWindow::set_mir_dab_n(int val){ update_mir_dab_notch(val);}
void MainWindow::set_mir_bc_n(int val){ update_mir_bc_notch(val);}
void MainWindow::set_mir_lna(int val){ update_mir_lna(val);}

//---


void MainWindow::MyUDP()
{
printf("After %d \n",__LINE__);
//socket = new QUdpSocket(this);
printf("After %d \n",__LINE__);  
  //We need to bind the UDP socket to an address and a port
 // socket->bind(QHostAddress("192.168.2.2"),11361);         //ex. Address localhost, port 1234
printf("After %d \n",__LINE__);  
 // connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
printf("After %d \n",__LINE__);
}

void MainWindow::readyRead()
{
QByteArray Buffer;
printf(" ****************** %d \n",__LINE__);  
//Buffer.resize(socket->pendingDatagramSize());
  
QHostAddress sender;
quint16 senderPort;
//socket->readDatagram(Buffer.data(),Buffer.size(),&sender,&senderPort);
printf(" Caught  a tropical fish \n");
}


//https://gist.github.com/lamprosg/4593723
/*
#include "myudp.h"
 
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  
  MyUDP server;
  MyUDP server;
  
  server.SayHello();
  
  return a.exec();
}

#include "myudp.h"
 
MyUDP::MyUDP(QObject *parent) : 
    QObject(parent)
{
  socket = new QUdpSocket(this);
  
  //We need to bind the UDP socket to an address and a port
  socket->bind(QHostAddress::LocalHost,1234);         //ex. Address localhost, port 1234
  
  connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
}


void MyUDP::SayHello()      //Just spit out some data
{
  QByteArray Data;
  Data.append("Hello from UDP land");
  
  socket->writeDatagram(Data,QHostAddress::LocalHost,1234);
  
  //If you want to broadcast something you send it to your broadcast address
  //ex. 192.2.1.255
}


void MyUDP::readyRead()     //Read something
{
  QByteArray Buffer;
  Buffer.resize(socket->pendingDatagramSize());
  
  QHostAddress sender;
  quint16 senderPort;
  socket->readDatagram(Buffer.data(),Buffer.size(),&sender,&senderPort);
  
  //The address will be sender.toString()
}


//Base class QObject

#include <QUdpSocket>
 
class MyUDP : public QObject
{
  Q_OBJECT
 
  public:
      explicit MyUDP(QObject *parent = 0);

      void SayHello();

  private:
      QUdpSocket *socket;
 
  signals:
 
  public slots:
      void readyRead();
 
};
*/


//---

void MainWindow::show_enable() // displays stream data
{
char str_1[20];
char str_2[20];
char str_3[20];
char str_4[20];
char str_5[20];
char str_6[20];

while(stream_flag == false)
	{
	QCoreApplication::processEvents();
	}
stream_flag=false;
sprintf(str_1,"Test 1: %d ",status[1]);
sprintf(str_2,"Test 2: %d ",status[2]);
sprintf(str_3,"Test 3: %d ",status[3]);
sprintf(str_4,"Test 4: %d ",status[4]);
sprintf(str_5,"Test 5: %d ",status[5]);
sprintf(str_6,"Test 5: %d ",status[6]);

ui->stat_1->setText(str_1);
ui->stat_2->setText(str_2);
ui->stat_3->setText(str_3);
ui->stat_4->setText(str_4);
ui->stat_5->setText(str_5);
ui->stat_6->setText(str_6);

ui->alpha_plotter->draw_trace(fft_video_buf,0,1024,-1500,-200); //(left,Num points,lower,upper)
}
