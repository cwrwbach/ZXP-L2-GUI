#pragma once

#include <QMainWindow>
#include <QUdpSocket>



class Udp : public QObject
{
Q_OBJECT

  //  void soxit();
   // void sendgram();
    

    QUdpSocket *socket = nullptr;

public:
  void soxit();
  void sendgram();
public slots:    
    void processPendingDatagrams();
};


