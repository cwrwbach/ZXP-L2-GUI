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

#define VERSION "QT5 Scope"
#define RX_BUF_SIZE 1000000
#define FFT_POINTS 1024

#define DEFAULT_SAMPLE_RATE		2.0
#define ONE_MEG 1048576
#define PI 3.141592654

extern "C" void start_server_stream();
extern "C" void update_pitaya_cf(int);
extern "C" void update_pitaya_sr(int);
extern "C" void update_pitaya_decim(int);
extern bool stream_flag;
bool down_convert_flag;

extern int resample_factor;

extern float xfer_buf_p[];
extern float xfer_buf_q[];

extern char fft_video_buf[];
extern int status[];

bool gain_flag = 0;
double centerFreqVal;
double sampleRateVal = DEFAULT_SAMPLE_RATE;
int decimVal;
int srVal;


ulong rxbuf_x = 0;
ulong chunk_x = 0;

int devModel = 1;

int display_buf[RX_BUF_SIZE]; 
int gri_val;

int atten_val;
int ifBandWidth = 1536;
int ifType = 0;



int shift = 30000;
double freq = 0.0;
int show_count = 0;

int samplesPerPacket;
int packs_rxd;
float fft_window[FFT_POINTS];
//---

MainWindow::MainWindow(const QString cfgfile, bool edit_conf, QWidget *parent) :
    QMainWindow(parent),    ui(new Ui::MainWindow)
{
ui->setupUi(this);
setWindowTitle(QString(VERSION));

QObject::connect(ui->setupRsp, SIGNAL(clicked()), this, SLOT(hardware_setup()));
 
connect(ui->freqCtrl, SIGNAL(newFrequency(qint64)), this, SLOT(setNewFrequency(qint64)));

fft_timer = new QTimer(this);
connect(fft_timer, SIGNAL(timeout()), this, SLOT(show_enable()));
fft_timer->start(200); //milli secs

ui->freqCtrl->setFrequency(5500000);

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
//set_sr();
set_cf();

}	

//---

void MainWindow::set_cf()
{

}

void MainWindow::setNewFrequency(qint64 )
{
int send_cf;	

send_cf = ui->freqCtrl->getFrequency();

printf(" cf: %d \n ",send_cf);

update_pitaya_cf(send_cf);

}


void MainWindow::set_pitaya_cf()
{
int send_cf;	

send_cf = ui->freqCtrl->getFrequency();

printf(" cf: %d \n ",send_cf);

update_pitaya_cf(send_cf);

}
/*
void MainWindow::set_sr2M(){ update_pitaya_sr(0);resample_factor = 2;}
void MainWindow::set_sr4M(){ update_pitaya_sr(1);resample_factor = 5;}
void MainWindow::set_sr5M(){ update_pitaya_sr(2);resample_factor = 10;}
void MainWindow::set_sr6M(){ update_pitaya_sr(3);resample_factor = 25;}
void MainWindow::set_sr8M(){ update_pitaya_sr(4);resample_factor = 50;}
void MainWindow::set_sr10M(){update_pitaya_sr(5);resample_factor = 125;}

void MainWindow::set_decim0(){ decimVal = 0 ;update_pitaya_decim(0);}
void MainWindow::set_decim2(){ decimVal = 2 ;update_pitaya_decim(1);}
void MainWindow::set_decim4(){ decimVal = 4 ;update_pitaya_decim(2);}
void MainWindow::set_decim8(){ decimVal = 8 ;update_pitaya_decim(3);}
void MainWindow::set_decim16(){ decimVal = 16 ;update_pitaya_decim(4);}
void MainWindow::set_decim32(){ decimVal = 32 ;update_pitaya_decim(5);}
*/
//---

void MainWindow::show_enable() // displays stream data
{
uint i,j;
double see_p, see_q,see, normalise, log_see[1024];
int N;
static int count;

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


for(i=0;i<FFT_POINTS;i++)
    display_buf[i] = (int) fft_video_buf[i] ;
	ui->alpha_plotter->draw_trace(display_buf,0,1024,-1500,-200); //(left,Num points,lower,upper)
	ui->wf_plotter->draw_trace(display_buf,0,1024,-4500,-200);
	stream_flag=false;
}
