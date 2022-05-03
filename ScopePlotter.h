#pragma once
#include <QtGui>
#include <QFrame>

#define HORZ_DIVS_MAX 50 
#define MAX_SCREENSIZE 1920 //Max reolution of the monitor

class ScopePlotter : public QFrame
{
    Q_OBJECT

public:
    explicit ScopePlotter(QWidget *parent = 0);
    ~ScopePlotter();
  
    void draw_trace( qint32 *,int,int ); //draw new data onto screen plot
	void updateOverlay() { drawOverlay(); }

signals:
    void newFrequency(qint64 freq); // emitted when frequency has changed

public slots:
    void setPlotColor(const QColor color);
    void setCenterFreq(quint64 f);
    void setPeakDetection(bool enabled, float c);
    void setPeakHold(bool enabled);
 
protected: //re-implemented widget event handlers
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void wheelEvent( QWheelEvent * event );

private:
    enum eCapturetype {
        NONE,
        LEFT,
        CENTER,
        RIGHT,
        YAXIS,
        XAXIS
    };
    void drawOverlay();
    bool isPointCloseTo(int x, int xr, int delta){return ((x > (xr-delta) ) && ( x<(xr+delta)) );}
    int m_XAxisYCenter;
    int m_YAxisWidth;

    QPixmap m_FftPixmap;
    QPixmap m_OverlayPixmap;
    QPixmap m_WaterfallPixmap;
    QSize m_Size;
    QString m_HDivText[HORZ_DIVS_MAX];
    bool m_Running;
    bool m_DrawOverlay;
    int m_Percent2DScreen;
    int    plot_HorDivs;   /*!< Current number of horizontal divisions. Calculated from width. */
    int    plot_VerDivs;   /*!< Current number of vertical divisions. Calculated from height. */
    double m_MaxdB;
    double m_MindB;
    qint32 m_dBStepSize;
    qint32 m_FreqUnits;
    int m_FreqDigits;  /*!< Number of decimal digits in frequency strings. */
    int m_FontSize;  /*!< Font size in points. */
    int HdivDelta; /*!< Minimum distance in pixels between two horizontal grid lines (vertical division). */
    int m_VdivDelta; /*!< Minimum distance in pixels between two vertical grid lines (horizontal division). */

    QColor m_FftColor, m_FftCol0, m_FftCol1, m_PeakHoldColor;

    void draw();	//draw new fft data onto screen plot
    void setRunningState(bool running) { m_Running = running; }
    void setNewFftData(float *fftData, int size);
    void setNewFftData(float *fftData, float *wfData, int size);
    void setFreqUnits(qint32 unit) { m_FreqUnits = unit; }
    void setDemodCenterFreq(quint64 f) { m_DemodCenterFreq = f; }
    void setDemodRanges(int FLowCmin, int FLowCmax, int FHiCmin, int FHiCmax, bool symetric);

    void setSampleRate(float rate)
    {
        if (rate > 0.0)
        {
            g_SampleFreq = rate;
            drawOverlay();
        }
    }

    float getSampleRate(void)
    {
        return g_SampleFreq;
    }

    void setFftCenterFreq(qint64 f) {
        qint64 limit = ((qint64)g_SampleFreq + m_Span) / 2 - 1;
        m_FftCenter = qBound(-limit, f, limit);
    }

    int     getNearestPeak(QPoint pt);
//    void        makeFrequencyStrings();
    int         xFromFreq(qint64 freq);
    qint64      freqFromX(int x);

    void getScreenIntegerFFTData(qint32 plotHeight, qint32 plotWidth,
                                 float maxdB, float mindB,
                                 qint64 startFreq, qint64 stopFreq,
                                 float *inBuf, qint32 *outBuf,
                                 qint32 *maxbin, qint32 *minbin)const;
 
    bool        m_PeakHoldActive;
    bool        m_PeakHoldValid;
    float      *m_fftData;     /*! pointer to incoming FFT data */
    float      *m_wfData;
    int         g_fftDataSize;

    eCapturetype    m_CursorCaptured;
 
    qint64      m_FftCenter;        // Center freq in the -span ... +span range
    qint64      m_DemodCenterFreq;
    qint64      m_StartFreqAdj;
    qint64      m_FreqPerDiv;
    int         m_HorDivs;   /*!< number of horizontal divisions. Calculated from width. */
    int         m_VerDivs;   /*!< number of vertical divisions. Calculated from height. */
    float       m_PandMindB;
    float       m_PandMaxdB;
    float       m_WfMindB;
    float       m_WfMaxdB;
    qint64      m_Span;
    float       g_SampleFreq;    /*!< Sample rate. */
    QFont       m_Font;         /*!< Font used for plotter (system font) */
    int         m_HdivDelta; /*!< Minimum distance in pixels between two horizontal grid lines (vertical division). */
    float       m_PeakDetection;
};










