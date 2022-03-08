#ifndef PLOTTER_H
#define PLOTTER_H

#include <QtGui>
#include <QFrame>
#include <QImage>
#include <vector>
#include <QMap>

#define HORZ_DIVS_MAX 50 //12
#define MAX_SCREENSIZE 4096

class ScopePlotter : public QFrame
{
    Q_OBJECT

public:
    explicit ScopePlotter(QWidget *parent = 0);
    ~ScopePlotter();
  
    void draw_trace( qint32 *,int,int,int,int );	//call to draw new data onto screen plot
	void updateOverlay() { drawOverlay(); }
    void setFontSize(int points) { m_FontSize = points; }
    void setHdivDelta(int delta) { HdivDelta = delta; }
    void setVdivDelta(int delta) { m_VdivDelta = delta; }
    void setFreqDigits(int digits) { m_FreqDigits = digits>=0 ? digits : 0; }

signals:
    void newCenterFreq(qint64 f);
    void newDemodFreq(qint64 freq, qint64 delta); /* delta is the offset from the center */
    void newLowCutFreq(int f);
    void newHighCutFreq(int f);
    void newFilterFreq(int low, int high);  /* substute for NewLow / NewHigh */
    void newFrequency(qint64 freq); // emitted when frequency has changed

public slots:
    void setPlotColor(const QColor color);
    void setCenterFreq(quint64 f);
    void setPeakDetection(bool enabled, float c);
    void setPeakHold(bool enabled);
    void setFftRange(float min, float max);
    void setPandapterRange(float min, float max);
    void setWaterfallRange(float min, float max);
    void moveToCenterFreq(void);

protected:
    //re-implemented widget event handlers
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


/*
  void getScreenIntegerFFTData(qint32 plotHeight, qint32 plotWidth,
                                 float maxdB, float mindB,
                                 qint64 startFreq, qint64 stopFreq,
                                 float *inBuf, qint32 *outBuf,
                                 qint32 *maxbin, qint32 *minbin) const;
* */

//    static void calcDivSize (qint64 low, qint64 high, int divswanted, qint64 &adjlow, qint64 &step, int& divs);






    bool isPointCloseTo(int x, int xr, int delta){return ((x > (xr-delta) ) && ( x<(xr+delta)) );}
	qint32 m_fftbuf[MAX_SCREENSIZE];
    qint32 dai_showbuf[MAX_SCREENSIZE];
	int m_XAxisYCenter;
    int m_YAxisWidth;

    QPixmap m_FftPixmap;
    QPixmap m_OverlayPixmap;
    QPixmap m_WaterfallPixmap;
    QColor m_ColorTbl[256];
    QSize m_Size;
    QString m_Str;
    QString m_HDivText[HORZ_DIVS_MAX+1];
    bool m_Running;
    bool m_DrawOverlay;
 
    int m_Percent2DScreen;

    int m_FLowCmin;
    int m_FLowCmax;
    int m_FHiCmin;
    int m_FHiCmax;
    bool m_symetric;

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

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

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
            m_SampleFreq = rate;
            drawOverlay();
        }
    }

    float getSampleRate(void)
    {
        return m_SampleFreq;
    }

    void setFftCenterFreq(qint64 f) {
        qint64 limit = ((qint64)m_SampleFreq + m_Span) / 2 - 1;
        m_FftCenter = qBound(-limit, f, limit);
    }

    int     getNearestPeak(QPoint pt);
    void    setWaterfallSpan(quint64 span_ms);
    quint64 getWfTimeRes(void);
    void    setFftRate(int rate_hz);
    void    clearWaterfall(void);
    bool    saveWaterfall(const QString & filename) const;

    void        makeFrequencyStrs();
    int         xFromFreq(qint64 freq);
    qint64      freqFromX(int x);
    qint64      roundFreq(qint64 freq, int resolution);
    quint64     msecFromY(int y);
    void        clampDemodParameters();

    void getScreenIntegerFFTData(qint32 plotHeight, qint32 plotWidth,
                                 float maxdB, float mindB,
                                 qint64 startFreq, qint64 stopFreq,
                                 float *inBuf, qint32 *outBuf,
                                 qint32 *maxbin, qint32 *minbin)const;
    void calcDivSize (qint64 low, qint64 high, int divswanted, qint64 &adjlow, qint64 &step, int& divs);

    bool        m_PeakHoldActive;
    bool        m_PeakHoldValid;
    quint8      m_wfbuf[MAX_SCREENSIZE]; // used for accumulating waterfall data at high time spans
    qint32      m_fftPeakHoldBuf[MAX_SCREENSIZE];
    float      *m_fftData;     /*! pointer to incoming FFT data */
    float      *m_wfData;
    int         m_fftDataSize;

    eCapturetype    m_CursorCaptured;
 
    qint64      m_FftCenter;        // Center freq in the -span ... +span range
    qint64      m_DemodCenterFreq;
    qint64      m_StartFreqAdj;
    qint64      m_FreqPerDiv;
    bool        m_CenterLineEnabled;  /*!< Distinguish center line. */
    bool        m_FilterBoxEnabled;   /*!< Draw filter box. */
    bool        m_TooltipsEnabled;     /*!< Tooltips enabled */
    bool        m_BookmarksEnabled;   /*!< Show/hide bookmarks on spectrum */
    int         m_DemodHiCutFreq;
    int         m_DemodLowCutFreq;
    int         m_DemodFreqX;		//screen coordinate x position
    int         m_DemodHiCutFreqX;	//screen coordinate x position
    int         m_DemodLowCutFreqX;	//screen coordinate x position
    int         m_CursorCaptureDelta;
    int         m_GrabPosition;
 
    int         m_HorDivs;   /*!< number of horizontal divisions. Calculated from width. */
    int         m_VerDivs;   /*!< number of vertical divisions. Calculated from height. */

    float       m_PandMindB;
    float       m_PandMaxdB;
    float       m_WfMindB;
    float       m_WfMaxdB;

    qint64      m_Span;
    float       m_SampleFreq;    /*!< Sample rate. */

    int         m_ClickResolution;
    QFont       m_Font;         /*!< Font used for plotter (system font) */
    int         m_HdivDelta; /*!< Minimum distance in pixels between two horizontal grid lines (vertical division). */

    float       m_PeakDetection;

};






#endif // PLOTTER_H



