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
#include "Rxr.h"

#define PLOT_WIDTH 1200 //1024
#define PLOT_HEIGHT 500

#define CUR_CUT_DELTA 5		//cursor capture delta in pixels

#define FFT_MIN_DB     -160.f
#define FFT_MAX_DB      0.f

// Colors of type QRgb in 0xAARRGGBB format (unsigned int)
#define PLOTTER_BGD_COLOR           0xFF1F1D1D
//#define PLOTTER_BGD_COLOR           0xFF00002f
#define PLOTTER_GRID_COLOR          0xFF444242
#define PLOTTER_TEXT_COLOR          0xFFDADADA
#define PLOTTER_CENTER_LINE_COLOR   0xFF788296
#define PLOTTER_FILTER_LINE_COLOR   0xFFFF7171
#define PLOTTER_FILTER_BOX_COLOR    0xFFA0A0A4
#define VERT_DIVS_MIN 5

//extern int g_audio_sample_rate;
//extern int g_sample_rate;
//extern int g_fft_size;
//extern int g_center_frequency;

int g_audio_sample_rate = 8000;
int g_sample_rate = 500000;
int g_fft_size = 1024;
int g_center_frequency=10000000;

ScopePlotter::ScopePlotter(QWidget *parent) : QFrame(parent) //Constructor
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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

    m_HorDivs = 16;
    m_VerDivs = 13;
    m_PandMaxdB = m_WfMaxdB = 0.f;
    m_PandMindB = m_WfMindB = -150.f;

    m_Running = false;
    m_DrawOverlay = true;

    m_MaxdB = -30;
    m_MindB = -130;
    m_dBStepSize = 10; //abs(m_MaxdB-m_MindB)/m_VerDivs;

    m_Running = false;
    m_DrawOverlay = true;
    m_FftPixmap = QPixmap(0,0);
    m_OverlayPixmap = QPixmap(0,0);
  
    m_Percent2DScreen = 85;	//FINDME  //percent of screen used for 2D display

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

//-=-=-=-

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
	
	//printf(" *** RESIZE IN PROGRESS  *** \n");

    if (!size().isValid())
        return;

    if (m_Size != size())
    {	//if changed, resize pixmaps to new screensize
        m_Size = size();
        m_OverlayPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
        m_OverlayPixmap.fill(Qt::black);
        m_FftPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
        m_FftPixmap.fill(Qt::black);

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

painter.drawPixmap(0,0,m_FftPixmap);
painter.drawPixmap(0, m_Percent2DScreen*m_Size.height()/100,m_WaterfallPixmap);
}

//---

void ScopePlotter::getScreenIntegerFFTData(qint32 plotHeight, qint32 plotWidth,
                                       float maxdB, float mindB,
                                       qint64 startFreq, qint64 stopFreq,
                                       float *inBuf, qint32 *outBuf,
                                       int *xmin, int *xmax) const
{
    qint32 i;
    qint32 y;
    qint32 x;
    qint32 ymax = 10000;
    qint32 xprev = -1;
    qint32 minbin, maxbin;
    qint32 m_BinMin, m_BinMax;
    qint32 m_FFTSize = m_fftDataSize;
    float *m_pFFTAveBuf = inBuf;
    float  dBGainFactor = ((float)plotHeight) / fabs(maxdB - mindB);
    auto* m_pTranslateTbl = new qint32[qMax(m_FFTSize, plotWidth)];

//printf("plotHeight: %d, plotWidth %d \n",plotHeight,plotWidth);
//printf("maxdB: %f, mindB: %f \n",maxdB, mindB);
//printf("startFreq: %lld, stopFreq: %lld \n",startFreq,stopFreq);
//printf("dBGainFactor: %f \n",dBGainFactor);
//printf("m_FFTSize: %d, m_SampleFreq: %f \n",m_FFTSize,m_SampleFreq); 

    /** FIXME: qint64 -> qint32 **/
    m_BinMin = (qint32)((float)startFreq * (float)m_FFTSize / m_SampleFreq);
    m_BinMin += (m_FFTSize/2);
    m_BinMax = (qint32)((float)stopFreq * (float)m_FFTSize / m_SampleFreq);
    m_BinMax += (m_FFTSize/2);

    minbin = m_BinMin < 0 ? 0 : m_BinMin;
    if (m_BinMin > m_FFTSize)
        m_BinMin = m_FFTSize - 1;
    if (m_BinMax <= m_BinMin)
        m_BinMax = m_BinMin + 1;
    maxbin = m_BinMax < m_FFTSize ? m_BinMax : m_FFTSize;


//printf("m_BinMax: %d, m_BinMin: %d \n",m_BinMax,m_BinMin);

    bool largeFft = (m_BinMax-m_BinMin) > plotWidth; // true if more fft point than plot points

    if (largeFft)
    {
        // more FFT points than plot points
//    printf("Large FFT: ");
    for (i = minbin; i < maxbin; i++)
            m_pTranslateTbl[i] = ((qint64)(i-m_BinMin)*plotWidth) / (m_BinMax - m_BinMin);
        *xmin = m_pTranslateTbl[minbin];
        *xmax = m_pTranslateTbl[maxbin - 1] + 1;
    }
    else
    {
//    printf("Small FFT: ");
        // more plot points than FFT points
        for (i = 0; i < plotWidth; i++)
            m_pTranslateTbl[i] = m_BinMin + (i*(m_BinMax - m_BinMin)) / plotWidth;
        *xmin = 0;
        *xmax = plotWidth;
    }

    if (largeFft)
    {
//    printf("Squeeze\n");
        // more FFT points than plot points
        for (i = minbin; i < maxbin; i++ )
        {
            y = (qint32)(dBGainFactor*(maxdB-m_pFFTAveBuf[i]));

            if (y > plotHeight)
                y = plotHeight;
            else if (y < 0)
                y = 0;

            x = m_pTranslateTbl[i];	//get fft bin to plot x coordinate transform

            if (x == xprev)   // still mappped to same fft bin coordinate
            {
                if (y < ymax) // store only the max value
                {
                    outBuf[x] = y;
                    ymax = y;
                }
            }
            else
            {
                outBuf[x] = y;
                xprev = x;
                ymax = y;
            }
        }
    }
    else
    {
//    printf("Stretch\n");
        // more plot points than FFT points
        for (x = 0; x < plotWidth; x++ )
        {
            i = m_pTranslateTbl[x]; // get plot to fft bin coordinate transform
            if(i < 0 || i >= m_FFTSize)
                y = plotHeight;
            else
                y = (qint32)(dBGainFactor*(maxdB-m_pFFTAveBuf[i]));

            if (y > plotHeight)
                y = plotHeight;
            else if (y < 0)
                y = 0;
            outBuf[x] = y;
        }
    }

//printf("xmin: %d, xmax: %d \n\n",*xmin,*xmax);
    delete [] m_pTranslateTbl;
}

//---

void ScopePlotter::draw_trace(int * trace_buf,int left,int num_points,int lower,int upper)
{
QPoint LineBuf[MAX_SCREENSIZE];
int plot_height = PLOT_HEIGHT ; 
double LinePoint;
int i,l,plot_width;;
int w;
int h;
int xmin, xmax;
double y_scale;
double ph,lw;

if (m_DrawOverlay)
	{
    drawOverlay();
    m_DrawOverlay = false;
    }
   
w = m_FftPixmap.width();
h = m_FftPixmap.height();

if ((w != 0) || (h != 0))
    {
	// redraw 2Dbitmap with overlay bitmap.
  	m_FftPixmap = m_OverlayPixmap.copy(0,0,w,h);
    m_WaterfallPixmap.scroll(0, 1, 0, 0, w, h); //scroll the wfall down
    
    QPainter painter_wf(&m_WaterfallPixmap);
    painter_wf.setPen(QColor(0x00,0x00,0x00));

m_fftDataSize = 1024;
m_SampleFreq = 500000;

float inbuf[4096];
int outbuf[4096];
int xmin,xmax;

w = m_WaterfallPixmap.width(); 
plot_width = qMin(w, MAX_SCREENSIZE);

for(int i=0; i<1024;i++)
    {
    inbuf[i] = (float) trace_buf[i]; //trace_buf is our input data main... 1024 point in FFT
    }    

getScreenIntegerFFTData(255, plot_width, //255 is plot height - need work on height.
                        -30, -150 , //max and min dB
                        g_sample_rate/-2,
                        g_sample_rate/2,
                        inbuf, outbuf,
                        &xmin,&xmax);

    //if fft & WF timer
    {
        if(0) //FIXME this enables/disables the waterfall, debug only
        {
        for (i = xmin; i < xmax; i++)
            {

            int inx = 100+trace_buf[i];
            painter_wf.setPen(QColor(turbo[inx][0],turbo[inx][1],turbo[inx][2]));
            painter_wf.drawPoint(i,0);
            }
        }
    } //fft & waterfall timer

update();
painter_wf.end();

QPainter painter_fft(&m_FftPixmap);

xmin = 0; //the left-est position on screen
ph=(double) plot_height;
lw=(double) lower-upper; 
y_scale =  ph/lw;

    painter_fft.setPen(m_FftColor); //fft trace colour
    l=left;         

    for (i = l; i < plot_width; i++)
        {
		LinePoint = (float) outbuf[i] *5  ; //(double) trace_buf[i] * 10; //10
        LinePoint = LinePoint * -y_scale;
	    LineBuf[i].setX(i + xmin);
        LineBuf[i].setY((int)LinePoint); 
        show();
        }    
 
    painter_fft.drawPolyline(LineBuf,plot_width); //paint a line with n points from LineBuf[n]
    painter_fft.end();
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


//printf(" StartFrq: %d Units: %d h_divs: %d \n",StartFreq,m_FreqDigits,m_HorDivs);


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
    qint64 StartFreq = g_center_frequency + m_FftCenter - m_Span/2;
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
int freq;
float width = (float) m_OverlayPixmap.width();
float pix_per_bin = width / g_fft_size;
float bin_step = (float) g_sample_rate / g_fft_size;
int start_freq = g_center_frequency - g_sample_rate / 2;

//qint64 freq = (qint64)(StartFreq + (float)m_Span * (float)x / (float)w);

//printf(" CenterF %d FftCenter %d Span %d ",m_CenterFreq,m_FftCenter, m_Span);
//printf(" W %d StartF %d Frq %d Ln %d \n",w,StartFreq,f,__LINE__);

freq = (int) roundf( start_freq + (x * pix_per_bin) * bin_step);


//printf(" G sample rate: %d frq: %d \n",g_sample_rate,freq);


emit    newFrequency(freq);

return freq;
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

void ScopePlotter::setCenterFreq(quint64 f)
{
g_center_frequency = f;


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

/** Center FFT plot around 0 (corresponds to center freq). */
void ScopePlotter::moveToCenterFreq(void)
{
    setFftCenterFreq(0);
    updateOverlay();
    m_PeakHoldValid = false;
}

/** Set peak hold on or off. */
void ScopePlotter::setPeakHold(bool enabled)
{
    m_PeakHoldActive = enabled;
    m_PeakHoldValid = false;
}

/** Set peak detection on or off. */
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
int pw = m_WaterfallPixmap.width();
int ph = m_WaterfallPixmap.height() * 10;
//int plot_height = PLOT_HEIGHT ; 
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

// create Font to use for scales
QFont Font("Arial");
Font.setPointSize(m_FontSize);
QFontMetrics metrics(Font);

Font.setWeight(QFont::Normal);
painter.setFont(Font);

    // draw vertical grid lines
    horiz_Pixperdiv = pw / (float)plot_HorDivs;
    y = ph ; //plot_height ; 
    
    painter.setPen(QPen(QColor(0xF0,0xF0,0xF0,0x30), 1, Qt::DotLine));
    for (int i = 1; i < plot_HorDivs; i++)
    {
        x = (int)((float)i*horiz_Pixperdiv);
        painter.drawLine(x, 0, x, y);
    }

//printf(" FREQUENCY STRINGS %d \n",__LINE__);
    // draw frequency values
    makeFrequencyStrs();
    painter.setPen(QColor(0xD8,0xBA,0xA1,0xFF));
   // y = plot_height - (plot_height/plot_VerDivs);
y = ph - (ph/plot_VerDivs);
    m_XAxisYCenter = ph - metrics.height()/2;
    for (int i = 1; i < plot_HorDivs; i++)
    {
        x = (int)((float)i*horiz_Pixperdiv - horiz_Pixperdiv/2);
        rect.setRect(x, y, (int)horiz_Pixperdiv, ph/plot_VerDivs);
        painter.drawText(rect, Qt::AlignHCenter|Qt::AlignBottom, m_HDivText[i]);
    }

    //m_dBStepSize = abs(m_MaxdB-m_MindB)/(double)plot_VerDivs;
    m_dBStepSize = 10;
    
    vert_Pixperdiv = (float)ph / (float)plot_VerDivs;
    painter.setPen(QPen(QColor(0xF0,0xF0,0xF0,0x30), 1,Qt::DotLine));
    for (int i = 1; i < plot_VerDivs; i++)
    {
        y = (int)((float) i*vert_Pixperdiv);
        painter.drawLine(5*metrics.width("0",-1), y,m_FftPixmap.width(), y);
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
        m_FftPixmap = m_OverlayPixmap.copy(0,0,pw,ph); //!!! //w = m_WaterfallPixmap.width();
        // trigger a new paintEvent
        update();
    }
}//drawOverLay


//---


void ScopePlotter::calcDivSize (qint64 low, qint64 high, int divswanted, qint64 &adjlow, qint64 &step, int& divs)
{

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

 
