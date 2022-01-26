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
//    void draw_trace( double * );	//call to draw new data onto screen plot

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

public slots:
    void setPlotColor(const QColor color);


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

    bool isPointCloseTo(int x, int xr, int delta){return ((x > (xr-delta) ) && ( x<(xr+delta)) );}
	qint32 m_fftbuf[MAX_SCREENSIZE];
    qint32 dai_showbuf[MAX_SCREENSIZE];
	int m_XAxisYCenter;
    int m_YAxisWidth;

    QPixmap m_2DPixmap;
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

//};

//888888888888888888888888

//#ifndef PLOTTER_H
//#define PLOTTER_H
/*
#include <QtGui>
#include <QFont>
#include <QFrame>
#include <QImage>
#include <vector>
#include <QMap>

#define HORZ_DIVS_MAX 12    //50
#define VERT_DIVS_MIN 5
#define MAX_SCREENSIZE 16384

#define PEAK_CLICK_MAX_H_DISTANCE 10 //Maximum horizontal distance of clicked point from peak
#define PEAK_CLICK_MAX_V_DISTANCE 20 //Maximum vertical distance of clicked point from peak
#define PEAK_H_TOLERANCE 2


class CPlotter : public QFrame
{
    Q_OBJECT

public:
    explicit CPlotter(QWidget *parent = 0);
    ~CPlotter();
    ~CPlotter();
*/

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    //void SetSdrInterface(CSdrInterface* ptr){m_pSdrInterface = ptr;}
    void draw();		//call to draw new fft data onto screen plot
    void setRunningState(bool running) { m_Running = running; }
    void setClickResolution(int clickres) { m_ClickResolution = clickres; }
    void setFilterClickResolution(int clickres) { m_FilterClickResolution = clickres; }
    void setFilterBoxEnabled(bool enabled) { m_FilterBoxEnabled = enabled; }
    void setCenterLineEnabled(bool enabled) { m_CenterLineEnabled = enabled; }
    void setTooltipsEnabled(bool enabled) { m_TooltipsEnabled = enabled; }
    void setBookmarksEnabled(bool enabled) { m_BookmarksEnabled = enabled; }

    void setNewFftData(float *fftData, int size);
    void setNewFftData(float *fftData, float *wfData, int size);

    void setCenterFreq(quint64 f);
    void setFreqUnits(qint32 unit) { m_FreqUnits = unit; }

    void setDemodCenterFreq(quint64 f) { m_DemodCenterFreq = f; }

    /*! \brief Move the filter to freq_hz from center. */
    void setFilterOffset(qint64 freq_hz)
    {
        m_DemodCenterFreq = m_CenterFreq + freq_hz;
        drawOverlay();
    }
    qint64 getFilterOffset(void)
    {
        return m_DemodCenterFreq - m_CenterFreq;
    }

    int getFilterBw()
    {
        return m_DemodHiCutFreq - m_DemodLowCutFreq;
    }

    void setHiLowCutFrequencies(int LowCut, int HiCut)
    {
        m_DemodLowCutFreq = LowCut;
        m_DemodHiCutFreq = HiCut;
        drawOverlay();
    }

    void getHiLowCutFrequencies(int *LowCut, int *HiCut)
    {
        *LowCut = m_DemodLowCutFreq;
        *HiCut = m_DemodHiCutFreq;
    }

    void setDemodRanges(int FLowCmin, int FLowCmax, int FHiCmin, int FHiCmax, bool symetric);

    /* Shown bandwidth around SetCenterFreq() */
    void setSpanFreq(quint32 s)
    {
        if (s > 0 && s < INT_MAX) {
            m_Span = (qint32)s;
            setFftCenterFreq(m_FftCenter);
        }
        drawOverlay();
    }

//    void setHdivDelta(int delta) { m_HdivDelta = delta; }
//    void setVdivDelta(int delta) { m_VdivDelta = delta; }

//    void setFreqDigits(int digits) { m_FreqDigits = digits>=0 ? digits : 0; }

    /* Determines full bandwidth. */
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

signals:
 //   void newCenterFreq(qint64 f);
 //   void newDemodFreq(qint64 freq, qint64 delta); /* delta is the offset from the center */
 //   void newLowCutFreq(int f);
  //  void newHighCutFreq(int f);
   // void newFilterFreq(int low, int high);  /* substitute for NewLow / NewHigh */
    void pandapterRangeChanged(float min, float max);
    void newZoomLevel(float level);
    void newSize();

public slots:
    // zoom functions
    void resetHorizontalZoom(void);
    void moveToCenterFreq(void);
    void moveToDemodFreq(void);
    void zoomOnXAxis(float level);

    // other FFT slots
    void setFftPlotColor(const QColor color);
    void setFftFill(bool enabled);
    void setPeakHold(bool enabled);
    void setFftRange(float min, float max);
   // void setWfColormap(const QString &cmap);
    void setPandapterRange(float min, float max);
    void setWaterfallRange(float min, float max);
    void setPeakDetection(bool enabled, float c);
   // void updateOverlay();

    void setPercent2DScreen(int percent)
    {
        m_Percent2DScreen = percent;
        m_Size = QSize(0,0);
        resizeEvent(NULL);
    }

protected:
    //re-implemented widget event handlers
   // void paintEvent(QPaintEvent *event);
   // void resizeEvent(QResizeEvent* event);
   // void mouseMoveEvent(QMouseEvent * event);
  //  void mousePressEvent(QMouseEvent * event);
   // void mouseReleaseEvent(QMouseEvent * event);
   // void wheelEvent( QWheelEvent * event );

private:
   // enum eCapturetype {
    //    NOCAP,
    //    LEFT,
    //    CENTER,
    //    RIGHT,
    //    YAXIS,
    //    XAXIS,
    //    BOOKMARK
    //};

   // void        drawOverlay();
    void        makeFrequencyStrs();
    int         xFromFreq(qint64 freq);
    qint64      freqFromX(int x);
    void        zoomStepX(float factor, int x);
    qint64      roundFreq(qint64 freq, int resolution);
    quint64     msecFromY(int y);
    void        clampDemodParameters();
 //   bool        isPointCloseTo(int x, int xr, int delta)
 //   {
 //       return ((x > (xr - delta)) && (x < (xr + delta)));
 //   }
    void getScreenIntegerFFTData(qint32 plotHeight, qint32 plotWidth,
                                 float maxdB, float mindB,
                                 qint64 startFreq, qint64 stopFreq,
                                 float *inBuf, qint32 *outBuf,
                                 qint32 *maxbin, qint32 *minbin);
    void calcDivSize (qint64 low, qint64 high, int divswanted, qint64 &adjlow, qint64 &step, int& divs);

    bool        m_PeakHoldActive;
    bool        m_PeakHoldValid;
 //   qint32      m_fftbuf[MAX_SCREENSIZE];
    quint8      m_wfbuf[MAX_SCREENSIZE]; // used for accumulating waterfall data at high time spans
    qint32      m_fftPeakHoldBuf[MAX_SCREENSIZE];
    float      *m_fftData;     /*! pointer to incoming FFT data */
    float      *m_wfData;
    int         m_fftDataSize;

   // int         m_XAxisYCenter;
   // int         m_YAxisWidth;

    eCapturetype    m_CursorCaptured;
   // QPixmap     m_2DPixmap;
  //  QPixmap     m_OverlayPixmap;
    //QPixmap     m_WaterfallPixmap;
  //  QColor      m_ColorTbl[256];
   // QSize       m_Size;
  //  QString     m_Str;
  //  QString     m_HDivText[HORZ_DIVS_MAX+1];
  //  bool        m_Running;
  //  bool        m_DrawOverlay;
    qint64      m_CenterFreq;       // The HW frequency
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
  //  int         m_Percent2DScreen;

  //  int         m_FLowCmin;
  //  int         m_FLowCmax;
  //  int         m_FHiCmin;
  //  int         m_FHiCmax;
  //  bool        m_symetric;

    int         m_HorDivs;   /*!< Current number of horizontal divisions. Calculated from width. */
    int         m_VerDivs;   /*!< Current number of vertical divisions. Calculated from height. */

    float       m_PandMindB;
    float       m_PandMaxdB;
    float       m_WfMindB;
    float       m_WfMaxdB;

    qint64      m_Span;
    float       m_SampleFreq;    /*!< Sample rate. */
  //  qint32      m_FreqUnits;
    int         m_ClickResolution;
    int         m_FilterClickResolution;

    int         m_Xzero;
  //  int         m_Yzero;  /*!< Used to measure mouse drag direction. */
  //  int         m_FreqDigits;  /*!< Number of decimal digits in frequency strings. */

    QFont       m_Font;         /*!< Font used for plotter (system font) */
    int         m_HdivDelta; /*!< Minimum distance in pixels between two horizontal grid lines (vertical division). */
//    int         m_VdivDelta; /*!< Minimum distance in pixels between two vertical grid lines (horizontal division). */

    quint32     m_LastSampleRate;

 //   QColor      m_FftColor, m_FftFillCol, m_PeakHoldColor;
 
    QColor      m_FftFillCol;
 
    bool        m_FftFill;

    float       m_PeakDetection;
    QMap<int,int>   m_Peaks;

    QList< QPair<QRect, qint64> >     m_BookmarkTags;

    // Waterfall averaging
    quint64     tlast_wf_ms;        // last time waterfall has been updated
    quint64     msec_per_wfline;    // milliseconds between waterfall updates
    quint64     wf_span;            // waterfall span in milliseconds (0 = auto)
    int         fft_rate;           // expected FFT rate (needed when WF span is auto)
};



// contributed by Chris Kuethe @ckuethe
// source https://ai.googleblog.com/2019/08/turbo-improved-rainbow-colormap-for.html
/*
unsigned int turbo[256][3] = {
    {48,18,59}, {50,21,67},    {51,24,74},
    {52,27,81},   {53,30,88},   {54,33,95},    {55,36,102},   {56,39,109},
    {57,42,115},  {58,45,121},  {59,47,128},   {60,50,134},   {61,53,139},
    {62,56,145},  {63,59,151},  {63,62,156},   {64,64,162},   {65,67,167},
    {65,70,172},  {66,73,177},  {66,75,181},   {67,78,186},   {68,81,191},
    {68,84,195},  {68,86,199},  {69,89,203},   {69,92,207},   {69,94,211},
    {70,97,214},  {70,100,218}, {70,102,221},  {70,105,224},  {70,107,227},
    {71,110,230}, {71,113,233}, {71,115,235},  {71,118,238},  {71,120,240},
    {71,123,242}, {70,125,244}, {70,128,246},  {70,130,248},  {70,133,250},
    {70,135,251}, {69,138,252}, {69,140,253},  {68,143,254},  {67,145,254},
    {66,148,255}, {65,150,255}, {64,153,255},  {62,155,254},  {61,158,254},
    {59,160,253}, {58,163,252}, {56,165,251},  {55,168,250},  {53,171,248},
    {51,173,247}, {49,175,245}, {47,178,244},  {46,180,242},  {44,183,240},
    {42,185,238}, {40,188,235}, {39,190,233},  {37,192,231},  {35,195,228},
    {34,197,226}, {32,199,223}, {31,201,221},  {30,203,218},  {28,205,216},
    {27,208,213}, {26,210,210}, {26,212,208},  {25,213,205},  {24,215,202},
    {24,217,200}, {24,219,197}, {24,221,194},  {24,222,192},  {24,224,189},
    {25,226,187}, {25,227,185}, {26,228,182},  {28,230,180},  {29,231,178},
    {31,233,175}, {32,234,172}, {34,235,170},  {37,236,167},  {39,238,164},
    {42,239,161}, {44,240,158}, {47,241,155},  {50,242,152},  {53,243,148},
    {56,244,145}, {60,245,142}, {63,246,138},  {67,247,135},  {70,248,132},
    {74,248,128}, {78,249,125}, {82,250,122},  {85,250,118},  {89,251,115},
    {93,252,111}, {97,252,108}, {101,253,105}, {105,253,102}, {109,254,98},
    {113,254,95}, {117,254,92}, {121,254,89},  {125,255,86},  {128,255,83},
    {132,255,81}, {136,255,78}, {139,255,75},  {143,255,73},  {146,255,71},
    {150,254,68}, {153,254,66}, {156,254,64},  {159,253,63},  {161,253,61},
    {164,252,60}, {167,252,58}, {169,251,57},  {172,251,56},  {175,250,55},
    {177,249,54}, {180,248,54}, {183,247,53},  {185,246,53},  {188,245,52},
    {190,244,52}, {193,243,52}, {195,241,52},  {198,240,52},  {200,239,52},
    {203,237,52}, {205,236,52}, {208,234,52},  {210,233,53},  {212,231,53},
    {215,229,53}, {217,228,54}, {219,226,54},  {221,224,55},  {223,223,55},
    {225,221,55}, {227,219,56}, {229,217,56},  {231,215,57},  {233,213,57},
    {235,211,57}, {236,209,58}, {238,207,58},  {239,205,58},  {241,203,58},
    {242,201,58}, {244,199,58}, {245,197,58},  {246,195,58},  {247,193,58},
    {248,190,57}, {249,188,57}, {250,186,57},  {251,184,56},  {251,182,55},
    {252,179,54}, {252,177,54}, {253,174,53},  {253,172,52},  {254,169,51},
    {254,167,50}, {254,164,49}, {254,161,48},  {254,158,47},  {254,155,45},
    {254,153,44}, {254,150,43}, {254,147,42},  {254,144,41},  {253,141,39},
    {253,138,38}, {252,135,37}, {252,132,35},  {251,129,34},  {251,126,33},
    {250,123,31}, {249,120,30}, {249,117,29},  {248,114,28},  {247,111,26},
    {246,108,25}, {245,105,24}, {244,102,23},  {243,99,21},   {242,96,20},
    {241,93,19},  {240,91,18},  {239,88,17},   {237,85,16},   {236,83,15},
    {235,80,14},  {234,78,13},  {232,75,12},   {231,73,12},   {229,71,11},
    {228,69,10},  {226,67,10},  {225,65,9},    {223,63,8},    {221,61,8},
    {220,59,7},   {218,57,7},   {216,55,6},    {214,53,6},    {212,51,5},
    {210,49,5},   {208,47,5},   {206,45,4},    {204,43,4},    {202,42,4},
    {200,40,3},   {197,38,3},   {195,37,3},    {193,35,2},    {190,33,2},
    {188,32,2},   {185,30,2},   {183,29,2},    {180,27,1},    {178,26,1},
    {175,24,1},   {172,23,1},   {169,22,1},    {167,20,1},    {164,19,1},
    {161,18,1},   {158,16,1},   {155,15,1},    {152,14,1},    {149,13,1},
    {146,11,1},   {142,10,1},   {139,9,2},     {136,8,2},     {133,7,2},
    {129,6,2},    {126,5,2},    {122,4,3}
};
*/

/*
// contributed by @devnulling
unsigned char plasma[256][3] = {
    {12, 7, 134},   {16, 7, 135},   {19, 6, 137},   {21, 6, 138},   {24, 6, 139},
    {27, 6, 140},   {29, 6, 141},   {31, 5, 142},   {33, 5, 143},   {35, 5, 144},
    {37, 5, 145},   {39, 5, 146},   {41, 5, 147},   {43, 5, 148},   {45, 4, 148},
    {47, 4, 149},   {49, 4, 150},   {51, 4, 151},   {52, 4, 152},   {54, 4, 152},
    {56, 4, 153},   {58, 4, 154},   {59, 3, 154},   {61, 3, 155},   {63, 3, 156},
    {64, 3, 156},   {66, 3, 157},   {68, 3, 158},   {69, 3, 158},   {71, 2, 159},
    {73, 2, 159},   {74, 2, 160},   {76, 2, 161},   {78, 2, 161},   {79, 2, 162},
    {81, 1, 162},   {82, 1, 163},   {84, 1, 163},   {86, 1, 163},   {87, 1, 164},
    {89, 1, 164},   {90, 0, 165},   {92, 0, 165},   {94, 0, 165},   {95, 0, 166},
    {97, 0, 166},   {98, 0, 166},   {100, 0, 167},  {101, 0, 167},  {103, 0, 167},
    {104, 0, 167},  {106, 0, 167},  {108, 0, 168},  {109, 0, 168},  {111, 0, 168},
    {112, 0, 168},  {114, 0, 168},  {115, 0, 168},  {117, 0, 168},  {118, 1, 168},
    {120, 1, 168},  {121, 1, 168},  {123, 2, 168},  {124, 2, 167},  {126, 3, 167},
    {127, 3, 167},  {129, 4, 167},  {130, 4, 167},  {132, 5, 166},  {133, 6, 166},
    {134, 7, 166},  {136, 7, 165},  {137, 8, 165},  {139, 9, 164},  {140, 10, 164},
    {142, 12, 164}, {143, 13, 163}, {144, 14, 163}, {146, 15, 162}, {147, 16, 161},
    {149, 17, 161}, {150, 18, 160}, {151, 19, 160}, {153, 20, 159}, {154, 21, 158},
    {155, 23, 158}, {157, 24, 157}, {158, 25, 156}, {159, 26, 155}, {160, 27, 155},
    {162, 28, 154}, {163, 29, 153}, {164, 30, 152}, {165, 31, 151}, {167, 33, 151},
    {168, 34, 150}, {169, 35, 149}, {170, 36, 148}, {172, 37, 147}, {173, 38, 146},
    {174, 39, 145}, {175, 40, 144}, {176, 42, 143}, {177, 43, 143}, {178, 44, 142},
    {180, 45, 141}, {181, 46, 140}, {182, 47, 139}, {183, 48, 138}, {184, 50, 137},
    {185, 51, 136}, {186, 52, 135}, {187, 53, 134}, {188, 54, 133}, {189, 55, 132},
    {190, 56, 131}, {191, 57, 130}, {192, 59, 129}, {193, 60, 128}, {194, 61, 128},
    {195, 62, 127}, {196, 63, 126}, {197, 64, 125}, {198, 65, 124}, {199, 66, 123},
    {200, 68, 122}, {201, 69, 121}, {202, 70, 120}, {203, 71, 119}, {204, 72, 118},
    {205, 73, 117}, {206, 74, 117}, {207, 75, 116}, {208, 77, 115}, {209, 78, 114},
    {209, 79, 113}, {210, 80, 112}, {211, 81, 111}, {212, 82, 110}, {213, 83, 109},
    {214, 85, 109}, {215, 86, 108}, {215, 87, 107}, {216, 88, 106}, {217, 89, 105},
    {218, 90, 104}, {219, 91, 103}, {220, 93, 102}, {220, 94, 102}, {221, 95, 101},
    {222, 96, 100}, {223, 97, 99},  {223, 98, 98},  {224, 100, 97}, {225, 101, 96},
    {226, 102, 96}, {227, 103, 95}, {227, 104, 94}, {228, 106, 93}, {229, 107, 92},
    {229, 108, 91}, {230, 109, 90}, {231, 110, 90}, {232, 112, 89}, {232, 113, 88},
    {233, 114, 87}, {234, 115, 86}, {234, 116, 85}, {235, 118, 84}, {236, 119, 84},
    {236, 120, 83}, {237, 121, 82}, {237, 123, 81}, {238, 124, 80}, {239, 125, 79},
    {239, 126, 78}, {240, 128, 77}, {240, 129, 77}, {241, 130, 76}, {242, 132, 75},
    {242, 133, 74}, {243, 134, 73}, {243, 135, 72}, {244, 137, 71}, {244, 138, 71},
    {245, 139, 70}, {245, 141, 69}, {246, 142, 68}, {246, 143, 67}, {246, 145, 66},
    {247, 146, 65}, {247, 147, 65}, {248, 149, 64}, {248, 150, 63}, {248, 152, 62},
    {249, 153, 61}, {249, 154, 60}, {250, 156, 59}, {250, 157, 58}, {250, 159, 58},
    {250, 160, 57}, {251, 162, 56}, {251, 163, 55}, {251, 164, 54}, {252, 166, 53},
    {252, 167, 53}, {252, 169, 52}, {252, 170, 51}, {252, 172, 50}, {252, 173, 49},
    {253, 175, 49}, {253, 176, 48}, {253, 178, 47}, {253, 179, 46}, {253, 181, 45},
    {253, 182, 45}, {253, 184, 44}, {253, 185, 43}, {253, 187, 43}, {253, 188, 42},
    {253, 190, 41}, {253, 192, 41}, {253, 193, 40}, {253, 195, 40}, {253, 196, 39},
    {253, 198, 38}, {252, 199, 38}, {252, 201, 38}, {252, 203, 37}, {252, 204, 37},
    {252, 206, 37}, {251, 208, 36}, {251, 209, 36}, {251, 211, 36}, {250, 213, 36},
    {250, 214, 36}, {250, 216, 36}, {249, 217, 36}, {249, 219, 36}, {248, 221, 36},
    {248, 223, 36}, {247, 224, 36}, {247, 226, 37}, {246, 228, 37}, {246, 229, 37},
    {245, 231, 38}, {245, 233, 38}, {244, 234, 38}, {243, 236, 38}, {243, 238, 38},
    {242, 240, 38}, {242, 241, 38}, {241, 243, 38}, {240, 245, 37}, {240, 246, 35},
    {239, 248, 33}
};



void ScopePlotter::setWfColormap(const QString &cmap)
{
    int i;

    if (cmap.compare("gqrx", Qt::CaseInsensitive) == 0)
    {
        for (i = 0; i < 256; i++)
        {
            // level 0: black background
            if (i < 20)
                m_ColorTbl[i].setRgb(0, 0, 0);
            // level 1: black -> blue
            else if ((i >= 20) && (i < 70))
                m_ColorTbl[i].setRgb(0, 0, 140*(i-20)/50);
            // level 2: blue -> light-blue / greenish
            else if ((i >= 70) && (i < 100))
                m_ColorTbl[i].setRgb(60*(i-70)/30, 125*(i-70)/30, 115*(i-70)/30 + 140);
            // level 3: light blue -> yellow
            else if ((i >= 100) && (i < 150))
                m_ColorTbl[i].setRgb(195*(i-100)/50 + 60, 130*(i-100)/50 + 125, 255-(255*(i-100)/50));
            // level 4: yellow -> red
            else if ((i >= 150) && (i < 250))
                m_ColorTbl[i].setRgb(255, 255-255*(i-150)/100, 0);
            // level 5: red -> white
            else if (i >= 250)
                m_ColorTbl[i].setRgb(255, 255*(i-250)/5, 255*(i-250)/5);
        }
    }
    else if (cmap.compare("turbo", Qt::CaseInsensitive) == 0)
    {
        for (i = 0; i < 256; i++)
            m_ColorTbl[i].setRgb(turbo[i][0], turbo[i][1], turbo[i][2]);
    }
    else if (cmap.compare("plasma",Qt::CaseInsensitive) == 0)
    {
        for (i = 0; i < 256; i++)
            m_ColorTbl[i].setRgb(plasma[i][0], plasma[i][1], plasma[i][2]);
    }
    else if (cmap.compare("whitehotcompressed",Qt::CaseInsensitive) == 0)
    {
        // contributed by @drmpeg @devnulling
        // for use with high quality spectrum paining
        // see https://gist.github.com/drmpeg/31a9a7dd6918856aeb60
        for (int i = 0; i < 256; i++)
        {
            if (i < 64)
            {
                m_ColorTbl[i].setRgb(i*4, i*4, i*4);
            }
            else
            {
                m_ColorTbl[i].setRgb(255, 255, 255);
            }
        }
    }
    else if (cmap.compare("whitehot",Qt::CaseInsensitive) == 0)
    {
        for (i = 0; i < 256; i++)
            m_ColorTbl[i].setRgb(i, i, i);
    }
    else if (cmap.compare("blackhot",Qt::CaseInsensitive) == 0)
    {
        for (i = 0; i < 256; i++)
            m_ColorTbl[i].setRgb(255-i, 255-i, 255-i);
    }
}

*/




#endif // PLOTTER_H



