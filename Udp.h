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


void start_server_stream();
void update_pitaya_cf(int);
void update_pitaya_sr(int);
void update_pitaya_ar(int);
void update_pitaya_demod(int);
void update_pitaya_rfg(int);
void update_pitaya_afg(int);
void update_mir_gr(int);
void update_mir_dab_notch(int);
void update_mir_bc_notch(int);
void update_mir_lna(int);







};


