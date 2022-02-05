#include "MainWindow.h"
#include "Udp.h"
#include <QDebug>
#include <unistd.h>
#include <math.h>
#include <alsa/asoundlib.h>

#define FREQ 4
#define SRATE 5
#define ARATE 6
#define RFG 7
#define AFG 8
#define DMOD 9
#define SSEL 10

bool stream_flag;
int fft_video_buf[1024];

//---

void Rxr::setup_socket()
{
socket = new QUdpSocket(this);
setup_sound();
usleep(100000);//FIXME - probably not needed

bool result =  socket->bind(QHostAddress::AnyIPv4, 11361);
qDebug() << result;

if(result)
    printf(" PASS \n");
else
    printf(" FAIL \n");

usleep(100000); //FIXME - probably not needed
connect(socket, &QUdpSocket::readyRead,
            this, &Rxr::processPendingDatagrams);
sendgram();
usleep(100000);//FIXME - probably not needed
}


void Rxr::sendgram()
{
QString word="Sign on message";
QByteArray buffer;
buffer.resize(socket->pendingDatagramSize());
//QHostAddress sender;
//quint16 senderPort;
buffer=word.toUtf8();
socket->writeDatagram(buffer.data(), QHostAddress::LocalHost, 11361 );
}


void Rxr::processPendingDatagrams()
 {
int size;
QByteArray datagram;   
QHostAddress sender;
u_int16_t port;

while (socket->hasPendingDatagrams())
    {
    size = socket->pendingDatagramSize(); 
    datagram.resize(socket->pendingDatagramSize());
    socket->readDatagram(datagram.data(),datagram.size(),&sender,&port);
      
    if(size == 1040) //FFT
        { 
        for(int i=0; i<1024;i++)
            fft_video_buf[i] = (int) datagram[i];

        stream_flag = true;
            }

    if(size == 1042) //G711
        {
        int snd_err = snd_pcm_writei(audio_device, datagram, 1024);
        if(snd_err < 0 )
            {      
            snd_pcm_recover(audio_device, snd_err, 1); //catch underruns (silent flag set, or not)
            usleep(1000);
            }
        }
    }   
}

void Rxr::setup_sound()
{
int err;
int audio_sr = 8000;

printf(" Setup sound devicce\n");

strcpy(alsa_device,"default");


err = snd_pcm_open(&audio_device, alsa_device, SND_PCM_STREAM_PLAYBACK, 0);
printf("err %d %d\n",err,__LINE__);
err = snd_pcm_set_params(audio_device,SND_PCM_FORMAT_A_LAW, SND_PCM_ACCESS_RW_INTERLEAVED,1,audio_sr,1,400000); //latency in
printf("err %d %d\n",err,__LINE__);
}

void Rxr::update_radio_cf(int cf )
{
QVector<quint32> buffer(256);
float ppm_factor, freq;
int new_data;

ppm_factor = 0.0;
freq = cf;
freq = (int)floor(freq*(1.0 + ppm_factor *1.0e-6) + 0.5);
buffer[FREQ]=(int) freq;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress::LocalHost,11361);
}	


void Rxr::start_server_stream(){};
void Rxr::update_radio_sr(int){};
void Rxr::update_radio_ar(int){};
void Rxr::update_radio_demod(int){};
void Rxr::update_radio_rfg(int){};
void Rxr::update_radio_afg(int){};
void Rxr::update_mir_gr(int){};
void Rxr::update_mir_dab_notch(int fred){printf("fred %d\n",fred);};
void Rxr::update_mir_bc_notch(int){};
void Rxr::update_mir_lna(int){};
