
#include "MainWindow.h"
#include "Udp.h"
#include <QDebug>
#include <unistd.h>


//https://doc.qt.io/qt-5/qudpsocket.html


bool stream_flag;
int fft_video_buf[1024];


void Udp::soxit()
{
socket = new QUdpSocket(this);
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
    printf(" size: %d \n",size);

         QByteArray datagram;
         datagram.resize(socket->pendingDatagramSize());
         socket->readDatagram(datagram.data(),datagram.size(),&sender,&port);
        printf(" << Messgae\n" );//Message From :: " << sender.toString();

if(size == 1040)
{ printf(" $ \n");
for(int i=0; i<1024;i++)
            fft_video_buf[i] = (int) datagram[i]; //-80 + (i/32); //(int) in_pak_buf[i];

stream_flag = true;
}


       // printf("Port From :: \n "); //<< port;
       // printf("Message :: \n"); //" //<< datagram;
    }
}








//dp::~Udp()
//{
//}





/*
#include "uchat.h"
#include "ui_uchat.h"


uchat::uchat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::uchat)
{
    ui->setupUi(this);
    clientSocket=new QUdpSocket(this);
    clientSocketc=new QUdpSocket(this);
    clientSocketc->bind(QHostAddress::LocalHost, 7000);
    connect(clientSocketc,SIGNAL(readyRead()),this,SLOT(readPendingDatagrams()));
}

uchat::~uchat()
{
    delete ui;
}

void uchat::on_sendButton_clicked()
{
    QString word=ui->lineEdit->text();
    ui->textBrowser->append(word);
    QByteArray buffer;
    buffer.resize(clientSocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    buffer=word.toUtf8();
    clientSocket->writeDatagram(buffer.data(), QHostAddress::LocalHost, 8001 );
}
void uchat::readPendingDatagrams()
{
    while (clientSocketc->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(clientSocketc->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        clientSocketc->readDatagram(buffer.data(), buffer.size(),&sender, &senderPort);
        ui->textBrowser->append(buffer.data());

    }
}
*/





