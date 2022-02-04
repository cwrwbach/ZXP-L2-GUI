
#include "MainWindow.h"
#include "Udp.h"
#include <QDebug>
#include <unistd.h>

#include <alsa/asoundlib.h>

//https://doc.qt.io/qt-5/qudpsocket.html


bool stream_flag;
int fft_video_buf[1024];


void Udp::soxit()
{
socket = new QUdpSocket(this);


setup_sound();
sleep(1);



bool result =  socket->bind(QHostAddress::AnyIPv4, 11361);
qDebug() << result;

if(result)
    {
    printf(" PASS \n");;
        }
else
    {
    printf(" FAIL \n");
    }

printf(" *************  BIDD, it wurked *********** \n)");


usleep(100000);

connect(socket, &QUdpSocket::readyRead,
            this, &Udp::processPendingDatagrams);
sendgram();
usleep(100000);
}


void Udp::sendgram()
{
QString word="freddyhamster";
//ui->textBrowser->append(word);

QByteArray buffer;
buffer.resize(socket->pendingDatagramSize());
QHostAddress sender;
quint16 senderPort;
buffer=word.toUtf8();
socket->writeDatagram(buffer.data(), QHostAddress::LocalHost, 11361 );
}


void Udp::processPendingDatagrams()
 {
int size;
   
QHostAddress sender;
u_int16_t port;

while (socket->hasPendingDatagrams())
    {

    size = socket->pendingDatagramSize(); 
   // printf(" size: %d \n",size);

         QByteArray datagram;
         datagram.resize(socket->pendingDatagramSize());
         socket->readDatagram(datagram.data(),datagram.size(),&sender,&port);
       // printf(" << Messgae\n" );//Message From :: " << sender.toString();

    if(size == 1040)
        { //printf(" $ \n");
        for(int i=0; i<1024;i++)
            fft_video_buf[i] = (int) datagram[i]; //-80 + (i/32); //(int) in_pak_buf[i];

        stream_flag = true;
            }

    if(size == 1042) //1042
        {

        int snd_err = snd_pcm_writei(audio_device, datagram, 1024);
        printf("err %d %d\n",snd_err,__LINE__);
        }

    }   //
}


void Udp::setup_sound()
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
