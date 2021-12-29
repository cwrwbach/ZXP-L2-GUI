
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColor>
#include <QMainWindow>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>


//#include "mirsdrapi-rsp.h"

#define MAX_FFT_SIZE 512



namespace Ui {
    class MainWindow;  /*! The main window UI */
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString cfgfile, bool edit_conf, QWidget *parent = 0);

    ~MainWindow();





    bool configOk; /*!< Main app uses this flag to know whether we should abort or continue. */

signals:
    void configChanged(QSettings *settings); /*!< New configuration has been loaded. */
	
	void mySignal(int myParameter); 

public slots:
 //   void setNewFrequency(qint64 rx_freq);

	
private:
    Ui::MainWindow *ui;

    QPointer<QSettings> m_settings;  /*!< Application wide settings. */
    QString             m_cfg_dir;   /*!< Default config dir, e.g. XDG_CONFIG_HOME. */
  
    QTimer   *fft_timer;


private slots:


void set_cf();
void set_pitaya_cf();
void setNewFrequency(qint64 );
void hardware_setup();
void show_enable();

/*
void set_sr2M();
void set_sr4M();
void set_sr5M();
void set_sr6M();
void set_sr8M();
void set_sr10M();


void set_decim0();
void set_decim2();
void set_decim4();
void set_decim8();
void set_decim16();
void set_decim32();
*/


};


#endif // MAINWINDOW_H
