#include "MainWindow.h"
#include "Rxr.h"
#include <QDebug>
#include <unistd.h>
#include <math.h>
#include <alsa/asoundlib.h>

//#define SERV_ADDR "192.168.2.2"
//#define SERV_ADDR "192.168.2.222"
#define SERV_ADDR "192.168.2.242"
//#define SERV_ADDR "192.168.2.101"
//#define SERV_ADDR "45.66.38.105"
#define AUDIO_RATE 8000
//#define AUDIO_RATE 11400 //11960 //7812  set to silly low rate for debugging

bool stream_flag;
int fft_video_buf[1024];
QVector<quint32> buffer(256);

char serv_addr[32];
int debug_fft;
//---

void Rxr::setup_socket()
{
socket = new QUdpSocket(this);

strcpy(serv_addr,SERV_ADDR);

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
socket->writeDatagram(buffer.data(), QHostAddress(serv_addr), 11361 );
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
        //debug
        //printf(" Debug FFT: %d \n",debug_fft++);
        for(int i=0; i<1024;i++)
            fft_video_buf[i] = (int) datagram[i];
            stream_flag = true;
            }

    if(size == 1042) //G711
        {
        //debug
        //printf("snd_pcm_avail %d \n",snd_pcm_avail (audio_device));
        int snd_err = snd_pcm_writei(audio_device, datagram, 1024);

        if(snd_err < 0 )
            {      
            //Debug
            printf(" S err %d \n",snd_err);
            snd_pcm_recover(audio_device, snd_err, 1); //catch underruns (silent flag set, or not)
            usleep(1000);
            }
        }
    }   
}

void Rxr::setup_sound()
{
int err;
int audio_sr = AUDIO_RATE;

printf(" Setup sound devicce\n");

strcpy(alsa_device,"default");


err = snd_pcm_open(&audio_device, alsa_device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK); //0
printf("err %d %d\n",err,__LINE__);
err = snd_pcm_set_params(audio_device,SND_PCM_FORMAT_A_LAW, SND_PCM_ACCESS_RW_INTERLEAVED,1,audio_sr,1,400000); //latency in
printf("err %d %d\n",err,__LINE__);
}

void Rxr::update_radio_cf(int cf )
{
//QVector<quint32> buffer(256);
float ppm_factor, freq;

ppm_factor = 0.0;
freq = cf;
freq = (int)floor(freq*(1.0 + ppm_factor *1.0e-6) + 0.5);
buffer[FREQ]=(int) freq;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
}	

void Rxr::update_radio_demod(int val)
{
printf("Demod %d\n",val);
buffer[DMOD]= val;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
};


void Rxr::start_server_stream(){};
void Rxr::update_radio_sr(int){};
void Rxr::update_radio_ar(int){};


void Rxr::update_radio_rfg(int val)
{
buffer[RFG]= val;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
printf("radio rfg: %d\n",val);
}


void Rxr::update_radio_afg(int val)
{
buffer[AFG]= val;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
printf("radio afg: %d\n",val);
};

void Rxr::update_mir_gr(int val)
{
buffer[MIR_RSP_GR]= val;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
printf("gr: %d\n",val);
}



void Rxr::update_mir_dab_notch(int val)
{
buffer[MIR_DAB_NOTCH ]= val;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
printf("dab %d\n",val);
}


void Rxr::update_mir_bc_notch(int val)
{
buffer[MIR_BC_NOTCH ]= val;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
printf("bc %d\n",val);
}

void Rxr::update_mir_lna(int val)
{
buffer[MIR_LNA_STATE]= val;
socket->writeDatagram((char*)buffer.data(),buffer.size()*sizeof(int),QHostAddress(serv_addr),11361);
printf("lna %d\n",val);
}



