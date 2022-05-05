#include <string>
#include <vector>
#include <stdio.h>
#include <QtGui>
#include <QtWidgets/QAction>
#include <math.h>
#include <complex.h>
#include "MainWindow.h"
#include <unistd.h>
#include "ui_QtBase-001.h"
#include "Rxr.h"
#include <QUdpSocket>

#define VERSION "QT5 Scope"
#define FFT_POINTS 1024
#define RX_BUF_SIZE 1024

extern bool stream_flag;
extern int fft_video_buf[];
int status[256]; //FIXME
Rxr radio_rx;

//---

MainWindow::MainWindow(const QString cfgfile, QWidget *parent) :
    QMainWindow(parent),    ui(new Ui::MainWindow)
{
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

connect(ui->src16p1, SIGNAL(clicked()), this, SLOT(set_src16p1()));
connect(ui->src16p2, SIGNAL(clicked()), this, SLOT(set_src16p2()));
connect(ui->src14p1, SIGNAL(clicked()), this, SLOT(set_src14p1()));
connect(ui->src14p2, SIGNAL(clicked()), this, SLOT(set_src14p2()));



//connect(ui->ip0, SIGNAL(clicked()), this, SLOT(set_ip0()));
//connect(ui->ip1, SIGNAL(clicked()), this, SLOT(set_ip1()));
//connect(ui->ip2, SIGNAL(clicked()), this, SLOT(set_ip2()));
//connect(ui->ip3, SIGNAL(clicked()), this, SLOT(set_ip3()));
//connect(ui->ip4, SIGNAL(clicked()), this, SLOT(set_ip4()));
//connect(ui->ip5, SIGNAL(clicked()), this, SLOT(set_ip5()));

connect(ui->rf_gain, SIGNAL(valueChanged(int)), this, SLOT(set_rfg(int)));
connect(ui->af_gain, SIGNAL(valueChanged(int)), this, SLOT(set_afg(int)));

//connect(ui->mir_gr, SIGNAL(valueChanged(int)), this, SLOT(set_mir_gr(int)));

//connect(ui->mir_dab_notch, SIGNAL(stateChanged(int)), this, SLOT(set_mir_dab_n(int)));
//connect(ui->mir_bc_notch, SIGNAL(stateChanged(int)), this, SLOT(set_mir_bc_n(int)));
//connect(ui->mir_lna, SIGNAL(stateChanged(int)), this, SLOT(set_mir_lna(int)));

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
radio_rx.setup_socket();
usleep(200000);
}	

void MainWindow::setNewFrequency(qint64 newfreq)
{
int send_cf = newfreq;	
ui->freqCtrl->setFrequency(newfreq);
ui->alpha_plotter->setCenterFreq(send_cf);
radio_rx.update_radio_cf(send_cf);
}

void MainWindow::set_rfg(int gain)
{
radio_rx.update_radio_rfg(gain);
}

void MainWindow::set_afg(int gain)
{
radio_rx.update_radio_afg(gain);
}


void MainWindow::set_sr0(){ radio_rx.update_radio_sr(0);}
void MainWindow::set_sr1(){ radio_rx.update_radio_sr(1);}
void MainWindow::set_sr2(){ radio_rx.update_radio_sr(2);}
void MainWindow::set_sr3(){ radio_rx.update_radio_sr(3);}
void MainWindow::set_sr4(){ radio_rx.update_radio_sr(4);}
void MainWindow::set_sr5(){ radio_rx.update_radio_sr(5);}

void MainWindow::set_ar0(){ radio_rx.update_radio_ar(0);}
void MainWindow::set_ar1(){ radio_rx.update_radio_ar(1);}
void MainWindow::set_ar2(){ radio_rx.update_radio_ar(2);}
void MainWindow::set_ar3(){ radio_rx.update_radio_ar(3);}

void MainWindow::set_dsb(){ radio_rx.update_radio_demod(1);}
void MainWindow::set_usb(){ radio_rx.update_radio_demod(2);}
void MainWindow::set_lsb(){ radio_rx.update_radio_demod(3);}

void MainWindow::set_src16p1()
{
radio_rx.update_radio_chan(1); 
}

void MainWindow::set_src16p2()
{
radio_rx.update_radio_chan(2); 
}

void MainWindow::set_src14p1()
{
radio_rx.update_radio_chan(1); 
}

void MainWindow::set_src14p2()
{
radio_rx.update_radio_chan(2);
}

//---

void MainWindow::show_enable() // displays stream data
{
//char str_1[20];
//char str_2[20];
//char str_3[20];
//char str_4[20];
//char str_5[20];
//char str_6[20];

//printf(" Show_enable %d \n",bug2++);

if(stream_flag ==true)
    {
    stream_flag=false;
/*
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
*/
    ui->alpha_plotter->draw_trace(fft_video_buf,0,1024); //(left,Num points)
 //   printf(" After drawtrace %d\n",bug3++);
    }
}


