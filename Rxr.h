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
#define MIR_DECIM_CONTROL 11
#define MIR_BW 20
#define MIR_IF 21
#define MIR_AGC_ENABLE 30
#define MIR_AGC_SETPOINT 31
//#define MIR_AGC_LNASTATE 32
#define MIR_GR_MODE 40
#define MIR_SETGR 41
#define MIR_GR_MIN 42
#define MIR_GR_THRESH 43
#define MIR_RSP_GR 44
#define MIR_LNA_STATE 45
#define MIR_RSP_MINGR 46
#define MIR_GRALT_IDX 47
#define MIR_GRALT_LNASTATE 48
#define MIR_IF_GR 49
#define MIR_BC_NOTCH 50
#define MIR_DAB_NOTCH 51
#define MIR_DC_MODE 52
#define MIR_DC_TIME 53
#define MIR_DC_ENBL 54
#define MIR_IQ_ENBL 55
#define MIR_PPM_SET 60

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
    void update_radio_cf(int);
    void update_radio_sr(int);
    void update_radio_ar(int);
    void update_radio_rfg(int);
    void update_radio_afg(int);
    void update_mir_gr(int);
    void update_mir_dab_notch(int);
    void update_mir_bc_notch(int);
    void update_mir_lna(int);

public slots:    
    void processPendingDatagrams();
    void start_server_stream();
    
};


