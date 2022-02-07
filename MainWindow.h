#pragma once

#include <QColor>
#include <QMainWindow>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QUdpSocket>

#define MAX_FFT_SIZE 512

namespace Ui {
    class MainWindow;  /*! The main window UI */
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
     
    explicit MainWindow(const QString cfgfile, QWidget *parent = 0);
    ~MainWindow();

bool configOk; /*!< Main app uses this flag to know whether we should abort or continue. */

signals:
    void configChanged(QSettings *settings); /*!< New configuration has been loaded. */
	
public slots:

public:

	
//private:
    Ui::MainWindow *ui;
private:
    QPointer<QSettings> m_settings;  /*!< Application wide settings. */
    QString             m_cfg_dir;   /*!< Default config dir, e.g. XDG_CONFIG_HOME. */
    QTimer   *fft_timer;
//    

private slots:

void setNewFrequency(qint64 );
void hardware_setup();
void show_enable();

void set_sr0();
void set_sr1();
void set_sr2();
void set_sr3();
void set_sr4();
void set_sr5();

void set_ar0();
void set_ar1();
void set_ar2();
void set_ar3();

void set_dsb();
void set_usb();
void set_lsb();
void set_rfg(int);
void set_afg(int);

void set_mir_gr(int);
void set_mir_dab_n(int);
void set_mir_bc_n(int);
void set_mir_lna(int);

};

