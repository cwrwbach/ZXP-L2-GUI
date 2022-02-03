#pragma once

#include <QMainWindow>
#include <QUdpSocket>



class Udp : public QObject
{
Q_OBJECT

public:
    
private slots:
    void soxit();
    void processPendingDatagrams();
private:
    QUdpSocket *socket = nullptr;
};

