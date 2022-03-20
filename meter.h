#pragma once

#include <QtGui>
#include <QFrame>
#include <QImage>

class CMeter : public QFrame
{
    Q_OBJECT

public:
    explicit CMeter(QWidget *parent = 0);
    explicit CMeter(float min_level = -100.0, float max_level = 10.0,
                    QWidget *parent = 0);
    ~CMeter();

 //   QSize minimumSizeHint() const;
//    QSize sizeHint() const;

    void setMin(float min_level);
    void setMax(float max_level);
    void setRange(float min_level, float max_level);

    void draw();
    void UpdateOverlay(){DrawOverlay();}

public slots:
    void setLevel(float dbfs);
    void setSqlLevel(float dbfs);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);

private:
    void DrawOverlay();

    QFont   m_Font;
    QPixmap m_FftPixmap;
    QPixmap m_OverlayPixmap;
    QSize   m_Size;
    QString m_Str;
    qreal   m_pixperdb;     // pixels / dB
    int     m_Siglevel;
    int     m_dBFS;
    qreal   m_Sql;
    qreal   m_SqlLevel;
};
