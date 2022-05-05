#pragma once

#include <QMainWindow>
#include <QUdpSocket>
#include <alsa/asoundlib.h>

#define FREQ 4
#define SRATE 5
#define ARATE 6
#define RFG 7
#define AFG 8
#define DMOD 9
#define SSEL 10
#define CHAN_SEL 11


class Rxr : public QObject
{
    Q_OBJECT
    QUdpSocket *socket = nullptr;
    snd_pcm_t *audio_device;
    void setup_sound();
    char alsa_device[64];

public:
    void setup_socket();
    void sendgram();
    void update_radio_demod(int);
    void update_radio_chan(int);
    void update_radio_cf(int);
    void update_radio_sr(int);
    void update_radio_ar(int);
    void update_radio_rfg(int);
    void update_radio_afg(int);

public slots:    
    void processPendingDatagrams();
    void start_server_stream();
    
};


