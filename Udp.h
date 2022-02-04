#pragma once

#include <QMainWindow>
#include <QUdpSocket>
#include <alsa/asoundlib.h>


class Udp : public QObject
{
Q_OBJECT

    QUdpSocket *socket = nullptr;
    snd_pcm_t *audio_device;
    void setup_sound();
    char alsa_device[64];

public:
    void soxit();
    void sendgram();
public slots:    
    void processPendingDatagrams();

};


