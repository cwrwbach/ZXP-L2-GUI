#include <stdlib.h>
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QtGlobal>
#include <QToolTip>
#include <cstdint>
#include <stdexcept>
#include "ScopePlotter.h"
#include "waterfall.h"



//#define NEWDRAW

#define PLOTTER_DEBUG


#define PLOT_WIDTH 1024
#define PLOT_HEIGHT 500
//#define PLOT_VER_DIVS 13
//#define PLOT_HOR_DIVS 20
#define CUR_CUT_DELTA 5		//cursor capture delta in pixels

#define FFT_MIN_DB     -160.f
#define FFT_MAX_DB      0.f

// Colors of type QRgb in 0xAARRGGBB format (unsigned int)
//#define PLOTTER_BGD_COLOR           0xFF1F1D1D
#define PLOTTER_BGD_COLOR           0xFF00002f

#define PLOTTER_GRID_COLOR          0xFF444242
#define PLOTTER_TEXT_COLOR          0xFFDADADA
#define PLOTTER_CENTER_LINE_COLOR   0xFF788296
#define PLOTTER_FILTER_LINE_COLOR   0xFFFF7171
#define PLOTTER_FILTER_BOX_COLOR    0xFFA0A0A4
#define VERT_DIVS_MIN 5


extern int g_audio_sample_rate;
extern int g_sample_rate;
extern int g_fft_size;
extern int g_center_frequency;




ScopePlotter::ScopePlotter(QWidget *parent) : QFrame(parent) //Constructor
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen,false);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setMouseTracking(true);

//added 24 jan 
//FIXME JUST BODGE NUMBERS
    m_FreqUnits = 100;
    m_StartFreqAdj = 5000000 - 250000; //Center - (FreqPerDiv * (HorDivs/2))
    m_FreqPerDiv = 25000; //Sample rate/HorDivs

    m_FftCenter = 0;
    //m_CenterFreq = 144500000;
    m_DemodCenterFreq = 144500000;

    m_HorDivs = 20;
    m_VerDivs = 13;
    m_PandMaxdB = m_WfMaxdB = 0.f;
    m_PandMindB = m_WfMindB = -150.f;

    
  //  m_CursorCaptured = NOCAP;
    m_Running = false;
    m_DrawOverlay = true;


    m_MaxdB = -30;
    m_MindB = -130;
    m_dBStepSize = 10; //abs(m_MaxdB-m_MindB)/m_VerDivs;
//end 24 jan

    m_Running = false;
    m_DrawOverlay = true;
    m_2DPixmap = QPixmap(0,0);
    m_OverlayPixmap = QPixmap(0,0);
  
    m_Percent2DScreen = 77;	//FINDME  //percent of screen used for 2D display

    m_FontSize = 9;
    m_VdivDelta = 100;
    HdivDelta = 20;

   setPlotColor(QColor(0x00,0xff,0x00,0xff));
}

ScopePlotter::~ScopePlotter() //destructor
{
}


QSize ScopePlotter::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize ScopePlotter::sizeHint() const
{
    return QSize(180, 180);
}


void ScopePlotter::mouseMoveEvent(QMouseEvent* event)
{
int mx,my;	

return;

QPoint pt = event->pos();
mx = pt.x();
my = pt.y();

printf(" x: %d y: %d \n",mx,my);

}

void ScopePlotter::mousePressEvent(QMouseEvent * event)
{
int mx,my;	
QPoint pt = event->pos();
mx = pt.x();
my = pt.y();
printf(" x: %d y: %d Ln: %d \n",mx,my,__LINE__);

freqFromX(mx);

}


// Called when a mouse button is released
void ScopePlotter::mouseReleaseEvent(QMouseEvent * event)
{
QPoint pt = event->pos();

return; 

if (!m_OverlayPixmap.rect().contains(pt))
    { 
	printf(" NOT in rect /");
	} 
 
else
    {
    printf(" YES in rect \n");
	}
}


// Called when a mouse wheel is turned
void ScopePlotter::wheelEvent(QWheelEvent * event)
{
QPoint pt = event->pos();
printf("Mouse wheelie workie \n");
}


// Called when screen size changes so must recalculate bitmaps
void ScopePlotter::resizeEvent(QResizeEvent* )
{
	
//	printf(" *** RESIZE IN PROGRESS  *** \n");

    if (!size().isValid())
        return;

    if (m_Size != size())
    {	//if changed, resize pixmaps to new screensize
        m_Size = size();
        m_OverlayPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
        m_OverlayPixmap.fill(Qt::black);
        m_2DPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
        m_2DPixmap.fill(Qt::black);

        int height = (100-m_Percent2DScreen)*m_Size.height()/100;
        if (m_WaterfallPixmap.isNull()) {
            m_WaterfallPixmap = QPixmap(m_Size.width(), height);
            m_WaterfallPixmap.fill(Qt::black);
        } else {
            m_WaterfallPixmap = m_WaterfallPixmap.scaled(m_Size.width(), height,
                                                         Qt::IgnoreAspectRatio,
                                                         Qt::SmoothTransformation);
        }
  //      m_PeakHoldValid=false;
    }     
    drawOverlay();
}


// Called by QT when screen needs to be redrawn
void ScopePlotter::paintEvent(QPaintEvent *)
{
QPainter painter(this);

painter.drawPixmap(0,0,m_2DPixmap);
painter.drawPixmap(0, m_Percent2DScreen*m_Size.height()/100,m_WaterfallPixmap);
}

//---

void ScopePlotter::draw_trace(int * trace_buf,int left,int num_points,int lower,int upper)
{
QPoint LineBuf[MAX_SCREENSIZE];
int plot_width = PLOT_WIDTH ; 
int plot_height = PLOT_HEIGHT ; 
double LinePoint;
int i,l,n;
int w;
int h;
int xmin, xmax;
double y_scale;
double ph,lw;



//set color table FIXME - move or loose this
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


if (m_DrawOverlay)
	{
    drawOverlay();
    m_DrawOverlay = false;
    }
   
w = m_2DPixmap.width();
h = m_2DPixmap.height();

if ((w != 0) || (h != 0))
    {
	// redraw 2Dbitmap with overlay bitmap.
  	m_2DPixmap = m_OverlayPixmap.copy(0,0,w,h);

    //scroll the wfall down
    
    m_WaterfallPixmap.scroll(0, 1, 0, 0, w, h);
    
    QPainter painter1(&m_WaterfallPixmap);
    painter1.setPen(QColor(0x00,0x00,0x00));

w=900;
h=40;

xmin = 10;
xmax = 1010;

    //if fft & WF timer
    {

        {
        for (i = xmin; i < xmax; i++)
            {

            int inx = 100+trace_buf[i];
            //col_inx *=-2;
           // printf(" %d \n",inx);

            painter1.setPen(QColor(turbo[inx][0],turbo[inx][1],turbo[inx][2]));

           // painter1.setPen(QColor(turbo[i]));
            painter1.drawPoint(i,0);
            }
        }
    } //fft & waterfall timer

update();
painter1.end();

QPainter painter2(&m_2DPixmap);

xmin = 0;
xmax = 4000;

ph=(double) plot_height;
lw=(double) lower-upper; 
y_scale =  ph/lw;


    painter2.setPen(m_FftColor); //fft trace colour
    n = num_points; //xmax - xmin;
    l=left;         
    
    for (i = l; i < n; i++)
        {
		LinePoint = (double) trace_buf[i] * 10;
		LinePoint = LinePoint * y_scale;	
        LineBuf[i].setX(i + xmin);
        LineBuf[i].setY((int)LinePoint); 
        show();
        }    
         
 //   update();
    painter2.drawPolyline(LineBuf, n); //paint a line with n points from LineBuf[n]
    painter2.end();
    }
update(); // trigger a new paintEvent
}

//---

// Create frequency division strings based on start frequency, span frequency,
// and frequency units.
// Places in QString array m_HDivText
// Keeps all strings the same fractional length
//FIXME THIS IS ALL A BIG BODGE

void ScopePlotter::makeFrequencyStrs()
{
    qint64  StartFreq = m_StartFreqAdj;
    float   freq;
    int     i,j;


printf(" StartFrq: %d Units: %d h_divs: %d \n",StartFreq,m_FreqDigits,m_HorDivs);


    if (0)//((1 == m_FreqUnits) || (m_FreqDigits == 0)) //FIXM BODGE
    {
        // if units is Hz then just output integer freq
        for (int i = 0; i <= m_HorDivs; i++)
        {
            freq = (float)StartFreq/(float)m_FreqUnits;
            m_HDivText[i].setNum((int)freq);
            StartFreq += m_FreqPerDiv;
        }
        return;
    }
    // here if is fractional frequency values
    // so create max sized text based on frequency units
    for (int i = 0; i <= m_HorDivs; i++)
    {
        freq = (float)StartFreq / (float)1000000; //(float)m_FreqUnits;
        m_HDivText[i].setNum(freq,'f', m_FreqDigits);
        StartFreq += m_FreqPerDiv;
    }
    // now find the division text with the longest non-zero digit
    // to the right of the decimal point.
    int max = 0;
    for (i = 0; i <= m_HorDivs; i++)
    {
        int dp = m_HDivText[i].indexOf('.');
        int l = m_HDivText[i].length()-1;
        for (j = l; j > dp; j--)
        {
            if (m_HDivText[i][j] != '0')
                break;
        }
        if ((j - dp) > max)
            max = j - dp;
    } max = 3;
    // truncate all strings to maximum fractional length
    StartFreq = m_StartFreqAdj;
    for (i = 0; i <= m_HorDivs; i++)
    {
        freq = (float)StartFreq/(float)m_FreqUnits;
        m_HDivText[i].setNum(freq/10000,'f', max); //FIXME BODGE
        StartFreq += m_FreqPerDiv;
    }
}//make freq strs

//---


// Convert from frequency to screen coordinate
int ScopePlotter::xFromFreq(qint64 freq)
{
    int w = m_OverlayPixmap.width();
    qint64 StartFreq = m_CenterFreq + m_FftCenter - m_Span/2;
    int x = (int) w * ((float)freq - StartFreq)/(float)m_Span;
    if (x < 0)
        return 0;
    if (x > (int)w)
        return m_OverlayPixmap.width();
    return x;
}

// Convert from screen coordinate to frequency
qint64 ScopePlotter::freqFromX(int x)
{
int w = m_OverlayPixmap.width();
qint64 StartFreq = m_CenterFreq + m_FftCenter - m_Span / 2;
qint64 f = (qint64)(StartFreq + (float)m_Span * (float)x / (float)w);

//printf(" CenterF %d FftCenter %d Span %d ",m_CenterFreq,m_FftCenter, m_Span);
//printf(" W %d StartF %d Frq %d Ln %d \n",w,StartFreq,f,__LINE__);

printf(" G sample rate: %d \n",g_sample_rate);

return f;
}

/** Calculate time offset of a given line on the waterfall */
quint64 ScopePlotter::msecFromY(int y)
{
    // ensure we are in the waterfall region
    if (y < m_OverlayPixmap.height())
        return 0;

    int dy = y - m_OverlayPixmap.height();

    if (msec_per_wfline > 0)
        return tlast_wf_ms - dy * msec_per_wfline;
    else
        return tlast_wf_ms - dy * 1000 / fft_rate;
}

// Round frequency to click resolution value
qint64 ScopePlotter::roundFreq(qint64 freq, int resolution)
{
    qint64 delta = resolution;
    qint64 delta_2 = delta / 2;
    if (freq >= 0)
        return (freq - (freq + delta_2) % delta + delta_2);
    else
        return (freq - (freq + delta_2) % delta - delta_2);
}

// Clamp demod freqeuency limits of m_DemodCenterFreq
void ScopePlotter::clampDemodParameters()
{
    if(m_DemodLowCutFreq < m_FLowCmin)
        m_DemodLowCutFreq = m_FLowCmin;
    if(m_DemodLowCutFreq > m_FLowCmax)
        m_DemodLowCutFreq = m_FLowCmax;

    if(m_DemodHiCutFreq < m_FHiCmin)
        m_DemodHiCutFreq = m_FHiCmin;
    if(m_DemodHiCutFreq > m_FHiCmax)
        m_DemodHiCutFreq = m_FHiCmax;
}

void ScopePlotter::setDemodRanges(int FLowCmin, int FLowCmax,
                              int FHiCmin, int FHiCmax,
                              bool symetric)
{
    m_FLowCmin=FLowCmin;
    m_FLowCmax=FLowCmax;
    m_FHiCmin=FHiCmin;
    m_FHiCmax=FHiCmax;
    m_symetric=symetric;
    clampDemodParameters();
    updateOverlay();
}

void ScopePlotter::setCenterFreq(quint64 f)
{
m_CenterFreq = f;

 //   if((quint64)m_CenterFreq == f)
  //      return;
 //   qint64 offset = m_CenterFreq - m_DemodCenterFreq;
 //   
 //   m_DemodCenterFreq = m_CenterFreq - offset;
 //   updateOverlay();
 //   m_PeakHoldValid = false;
}


void ScopePlotter::setFftRange(float min, float max)
{
    setWaterfallRange(min, max);
    setPandapterRange(min, max);
}

void ScopePlotter::setPandapterRange(float min, float max)
{
  //  if (out_of_range(min, max))
    //    return;

    m_PandMindB = min;
    m_PandMaxdB = max;
    updateOverlay();
    m_PeakHoldValid = false;
}

void ScopePlotter::setWaterfallRange(float min, float max)
{
   // if (out_of_range(min, max))
    //    return;

    m_WfMindB = min;
    m_WfMaxdB = max;
    // no overlay change is necessary
}


// Make a single zoom step on the X axis.
void ScopePlotter::zoomStepX(float step, int x)
{
    // calculate new range shown on FFT
    float new_range = qBound(10.0f, m_Span * step, m_SampleFreq * 10.0f);

    // Frequency where event occured is kept fixed under mouse
    float ratio = (float)x / (float)m_OverlayPixmap.width();
    float fixed_hz = freqFromX(x);
    float f_max = fixed_hz + (1.0 - ratio) * new_range;
    float f_min = f_max - new_range;

    // ensure we don't go beyond the rangelimits
    if (f_min < m_CenterFreq - m_SampleFreq / 2.f)
        f_min = m_CenterFreq - m_SampleFreq / 2.f;

    if (f_max > m_CenterFreq + m_SampleFreq / 2.f)
        f_max = m_CenterFreq + m_SampleFreq / 2.f;
    new_range = f_max - f_min;

    qint64 fc = (qint64)(f_min + (f_max - f_min) / 2.0);

    setFftCenterFreq(fc - m_CenterFreq);
    setSpanFreq((quint32)new_range);

    float factor = (float)m_SampleFreq / (float)m_Span;
    emit newZoomLevel(factor);
    qDebug() << QString("Spectrum zoom: %1x").arg(factor, 0, 'f', 1);

    m_PeakHoldValid = false;
}

void ScopePlotter::zoomOnXAxis(float level)
{
    float current_level = (float)m_SampleFreq / (float)m_Span;

    zoomStepX(current_level / level, xFromFreq(m_DemodCenterFreq));
}




// Ensure overlay is updated by either scheduling or forcing a redraw
//void ScopePlotter::updateOverlay()
//{
 //   if (m_Running)
  //      m_DrawOverlay = true;
   // else
    //    drawOverlay();
//}

/** Reset horizontal zoom to 100% and centered around 0. */
void ScopePlotter::resetHorizontalZoom(void)
{
    setFftCenterFreq(0);
    setSpanFreq((qint32)m_SampleFreq);
}

/** Center FFT plot around 0 (corresponds to center freq). */
void ScopePlotter::moveToCenterFreq(void)
{
    setFftCenterFreq(0);
    updateOverlay();
    m_PeakHoldValid = false;
}

/** Center FFT plot around the demodulator frequency. */
void ScopePlotter::moveToDemodFreq(void)
{
    setFftCenterFreq(m_DemodCenterFreq-m_CenterFreq);
    updateOverlay();

    m_PeakHoldValid = false;
}

/** Set FFT plot color. */
void ScopePlotter::setFftPlotColor(const QColor color)
{
    m_FftColor = color;
    m_FftFillCol = color;
    m_FftFillCol.setAlpha(0x1A);
    m_PeakHoldColor = color;
    m_PeakHoldColor.setAlpha(60);
}

/** Enable/disable filling the area below the FFT plot. */
void ScopePlotter::setFftFill(bool enabled)
{
    m_FftFill = enabled;
}

/** Set peak hold on or off. */
void ScopePlotter::setPeakHold(bool enabled)
{
    m_PeakHoldActive = enabled;
    m_PeakHoldValid = false;
}

/**
 * Set peak detection on or off.
 * @param enabled The new state of peak detection.
 * @param c Minimum distance of peaks from mean, in multiples of standard deviation.
 */
void ScopePlotter::setPeakDetection(bool enabled, float c)
{
    if(!enabled || c <= 0)
        m_PeakDetection = -1;
    else
        m_PeakDetection = c;
}


//---


// Draw overlay bitmap containing grid and text that does not need every fft data update.
void ScopePlotter::drawOverlay()
{
int plot_width = PLOT_WIDTH ; 
int plot_height = PLOT_HEIGHT ; 
int x,y;
float horiz_Pixperdiv,vert_Pixperdiv;

//if (m_OverlayPixmap.isNull())
//	return;

QRect rect;
QPainter painter(&m_OverlayPixmap);
painter.initFrom(this);

//Grids
plot_VerDivs = m_VerDivs; //PLOT_VER_DIVS ; 
plot_HorDivs = m_HorDivs ; //PLOT_HOR_DIVS ; 

painter.setPen(QPen(QColor(0xF0,0xF0,0xF0,0x30), 1, Qt::DotLine));
//Draw a centre line (later for cursor line)
  for (int i = 1; i < 500; i++)
    {
  //      x = (int)((float)i*horiz_Pixperdiv);
    x=512; y=500; //x cenre is 512 a present
        painter.drawLine(x, 0, x, y);
    }
 update();
//return;
//extra

// create Font to use for scales
QFont Font("Arial");
Font.setPointSize(m_FontSize);
QFontMetrics metrics(Font);

Font.setWeight(QFont::Normal);
painter.setFont(Font);

    // draw vertical grid lines
    horiz_Pixperdiv = (float)plot_width / (float)plot_HorDivs;
    y = plot_height ; 
    
    painter.setPen(QPen(QColor(0xF0,0xF0,0xF0,0x30), 1, Qt::DotLine));
    for (int i = 1; i < plot_HorDivs; i++)
    {
        x = (int)((float)i*horiz_Pixperdiv);
        painter.drawLine(x, 0, x, y);
    }

//printf(" FREQUENCY STAIRS %d \n",__LINE__);
    // draw frequency values
      makeFrequencyStrs();
    painter.setPen(QColor(0xD8,0xBA,0xA1,0xFF));
    y = plot_height - (plot_height/plot_VerDivs);
    m_XAxisYCenter = plot_height - metrics.height()/2;
    for (int i = 1; i < plot_HorDivs; i++)
    {
        x = (int)((float)i*horiz_Pixperdiv - horiz_Pixperdiv/2);
        rect.setRect(x, y, (int)horiz_Pixperdiv, plot_height/plot_VerDivs);
        painter.drawText(rect, Qt::AlignHCenter|Qt::AlignBottom, m_HDivText[i]);
    }

    //m_dBStepSize = abs(m_MaxdB-m_MindB)/(double)plot_VerDivs;
    m_dBStepSize = 10;
    
    vert_Pixperdiv = (float)plot_height / (float)plot_VerDivs;
    painter.setPen(QPen(QColor(0xF0,0xF0,0xF0,0x30), 1,Qt::DotLine));
    for (int i = 1; i < plot_VerDivs; i++)
    {
        y = (int)((float) i*vert_Pixperdiv);
        painter.drawLine(5*metrics.width("0",-1), y, plot_width, y);
    }

    // draw amplitude values
    painter.setPen(QColor(0xD8,0xBA,0xA1,0xFF));
    //Font.setWeight(QFont::Light);
    painter.setFont(Font);
    int dB = m_MaxdB;
    m_YAxisWidth = metrics.width("-120 ");
	for (int i = 1; i < plot_VerDivs; i++)
		{
		
 
        y = (int)((float)i*vert_Pixperdiv);
        rect.setRect(0, y-metrics.height()/2, m_YAxisWidth, metrics.height());
        painter.drawText(rect, Qt::AlignRight|Qt::AlignVCenter, QString::number(dB));
        dB -= m_dBStepSize;  // move to end if want to include maxdb  
	}
		

    if (!m_Running)
    {
        // if not running so is no data updates to draw to screen
        // copy into 2Dbitmap the overlay bitmap.
        m_2DPixmap = m_OverlayPixmap.copy(0,0,plot_width,plot_height);

        // trigger a new paintEvent
        update();
    }
}//drawOverLay


//---


void ScopePlotter::calcDivSize (qint64 low, qint64 high, int divswanted, qint64 &adjlow, qint64 &step, int& divs)
{
#ifdef PLOTTER_DEBUG
    qDebug() << "low: " << low;
    qDebug() << "high: " << high;
    qDebug() << "divswanted: " << divswanted;
#endif

    if (divswanted == 0)
        return;

    static const qint64 stepTable[] = { 1, 2, 5 };
    static const int stepTableSize = sizeof (stepTable) / sizeof (stepTable[0]);
    qint64 multiplier = 1;
    step = 1;
    divs = high - low;
    int index = 0;
    adjlow = (low / step) * step;

    while (divs > divswanted)
    {
        step = stepTable[index] * multiplier;
        divs = int ((high - low) / step);
        adjlow = (low / step) * step;
        index = index + 1;
        if (index == stepTableSize)
        {
            index = 0;
            multiplier = multiplier * 10;
        }
    }
    if (adjlow < low)
        adjlow += step;

#ifdef PLOTTER_DEBUG
    qDebug() << "adjlow: "  << adjlow;
    qDebug() << "step: " << step;
    qDebug() << "divs: " << divs;
#endif
}

void ScopePlotter::setPlotColor(const QColor color)
{
    m_FftColor = color;
    m_FftCol0 = color;
    m_FftCol0.setAlpha(0x00);
    m_FftCol1 = color;
    m_FftCol1.setAlpha(0xA0);
    m_PeakHoldColor=color;
    m_PeakHoldColor.setAlpha(60);
}

//=================================================================

