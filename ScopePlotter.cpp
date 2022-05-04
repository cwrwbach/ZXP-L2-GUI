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

// Colors of type QRgb in 0xAARRGGBB format (unsigned int)
#define PLOTTER_BGD_COLOR           0xFF1F1F1D
#define WFALL_BGD_COLOR             0xFF1f2F1D
#define ROUNDING_VAL 1000

int g_sample_rate = 512000;
int g_fft_range = 255;
int g_center_frequency=9000000;

//---

ScopePlotter::ScopePlotter(QWidget *parent) : QFrame(parent) //Constructor
{
setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
setFocusPolicy(Qt::StrongFocus);
setAttribute(Qt::WA_PaintOnScreen,false);
setAutoFillBackground(false);
setAttribute(Qt::WA_OpaquePaintEvent, false);
setAttribute(Qt::WA_NoSystemBackground, true);
setMouseTracking(true);

g_SampleFreq = 512000;

m_FreqUnits = 100;
m_StartFreqAdj = 5120000 - 256000; //Center - (FreqPerDiv * (HorDivs/2))
m_FreqPerDiv = 25000; //Sample rate/HorDivs
m_FftCenter = 0;

m_HorDivs = 10;
m_VerDivs = 13;
m_PandMaxdB = m_WfMaxdB = 0.f;
m_PandMindB = m_WfMindB = -150.f;

//m_Running = false;
//m_DrawOverlay = true;

m_MaxdB = -10;
m_MindB = -160;
m_dBStepSize = 10; //abs(m_MaxdB-m_MindB)/m_VerDivs;

m_Running = false;
m_DrawOverlay = true;
m_FftPixmap = QPixmap(0,0);
m_OverlayPixmap = QPixmap(0,0);
  
m_Percent2DScreen = 85;	//FINDME  //percent of screen used for 2D display

m_FontSize = 9;
m_VdivDelta = 100;
m_FreqDigits = 4;

HdivDelta = 20;

setPlotColor(QColor(0x00,0xff,0x00,0xff));
}

ScopePlotter::~ScopePlotter() //destructor
{
}

//---

void ScopePlotter::draw_trace(int * trace_buf,int left,int num_points)
{
QPoint LineBuf[MAX_SCREENSIZE];
double LinePoint;
int i;
int wide,high;
double y_scale;
float inbuf[2048];
int outbuf[2048];

if (m_DrawOverlay)
	{
    drawOverlay();
    m_DrawOverlay = false;
    }
   
wide = m_FftPixmap.width();
high = m_FftPixmap.height();

if ((wide != 0) || (high != 0))
    {
	// redraw 2Dbitmap with overlay bitmap.
  	m_FftPixmap = m_OverlayPixmap.copy(0,0,wide,high);
    m_WaterfallPixmap.scroll(0, 1, 0, 0, wide, high); //scroll the wfall down
    QPainter painter_wf(&m_WaterfallPixmap);
    painter_wf.setPen(QColor(0x00,0x00,0x00));

    g_fftDataSize = num_points;
    
    wide = m_WaterfallPixmap.width(); 
 
    for(int i=0; i<num_points;i++)
        {
        inbuf[i] = (float) trace_buf[i]; //trace_buf is our input data main
        inbuf[i] *= -1; //units are half-dB
        }    

    int xmin,xmax;
    getScreenIntegerFFTData(g_fft_range, wide,
                        0, -255 , //max and min dB
                        g_sample_rate/-2,
                        g_sample_rate/2,
                        inbuf, outbuf,
                        &xmin,&xmax);
    
    for (i = left; i < wide; i++)
        {
        int inx = 50+outbuf[i]; //100+trace_buf[i];
        painter_wf.setPen(QColor(turbo[inx][0],turbo[inx][1],turbo[inx][2]));
        painter_wf.drawPoint(i,0);
        }
    update();
    painter_wf.end();

    QPainter painter_fft(&m_FftPixmap);
    y_scale = (double) high/g_fft_range;
    y_scale *= (float) m_Percent2DScreen/100;
    painter_fft.setPen(m_FftColor); //fft trace colour
    for (i = left; i < wide; i++)
        {
		LinePoint = (float) outbuf[i]* y_scale;
        LineBuf[i].setX(i + left);
        LineBuf[i].setY((int)LinePoint); 
        show();
        }    
    painter_fft.drawPolyline(LineBuf,wide); //paint a line with n points from LineBuf[n]
    painter_fft.end();
    }
update(); // trigger a new paintEvent
}//draw_trace


//---

// Called by QT when screen needs to be redrawn
void ScopePlotter::paintEvent(QPaintEvent *)
{
QPainter painter(this);
painter.drawPixmap(0,0,m_FftPixmap);
painter.drawPixmap(0, m_Percent2DScreen*m_Size.height()/100,m_WaterfallPixmap);
}
//---

void ScopePlotter::resizeEvent(QResizeEvent* ) //recalculate bitmaps
{
int height;

if (!size().isValid())
    return;

if (m_Size != size()) //resize pixmaps to new screensize
    {
    m_Size = size();
    m_OverlayPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
    m_OverlayPixmap.fill(PLOTTER_BGD_COLOR ); //The FFT screen backround
    m_FftPixmap = QPixmap(m_Size.width(), m_Percent2DScreen*m_Size.height()/100);
    m_FftPixmap.fill(Qt::red); //???

    height = (100-m_Percent2DScreen)*m_Size.height()/100;
    if (m_WaterfallPixmap.isNull()) 
        {
        m_WaterfallPixmap = QPixmap(m_Size.width(), height);
        m_WaterfallPixmap.fill(WFALL_BGD_COLOR);
        } 
        else 
        {
        m_WaterfallPixmap = m_WaterfallPixmap.scaled(m_Size.width(), height,
                            Qt::IgnoreAspectRatio,
                            Qt::SmoothTransformation);
        }
    // m_PeakHoldValid=false;
    }     
drawOverlay();
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
qint32 m_FFTSize = g_fftDataSize;
float *pFFTAveBuf = inBuf;
//float  dBGainFactor = 1.0; //((float)plotHeight) / fabs(maxdB - mindB); printf(" GFAC %f \n",dBGainFactor);
auto* m_pTranslateTbl = new qint32[qMax(m_FFTSize, plotWidth)];

/** FIXME: qint64 -> qint32 **/
m_BinMin = (qint32)((float)startFreq * (float)m_FFTSize / g_SampleFreq);
m_BinMin += (m_FFTSize/2);
m_BinMax = (qint32)((float)stopFreq * (float)m_FFTSize / g_SampleFreq);
m_BinMax += (m_FFTSize/2);

minbin = m_BinMin < 0 ? 0 : m_BinMin;
if (m_BinMin > m_FFTSize)
    m_BinMin = m_FFTSize - 1;
if (m_BinMax <= m_BinMin)
    m_BinMax = m_BinMin + 1;
maxbin = m_BinMax < m_FFTSize ? m_BinMax : m_FFTSize;

bool largeFft = (m_BinMax-m_BinMin) > plotWidth; // true if more fft than plot points

//setup a FFT points to screen width translate table
if (largeFft)
    {
    // more FFT points than plot points
    for (i = minbin; i < maxbin; i++)
        m_pTranslateTbl[i] = ((qint64)(i-m_BinMin)*plotWidth) / (m_BinMax -m_BinMin);
    *xmin = m_pTranslateTbl[minbin];
    *xmax = m_pTranslateTbl[maxbin - 1] + 1;
    }
else
    {
    // more plot points than FFT points
    for (i = 0; i < plotWidth; i++)
            m_pTranslateTbl[i] = m_BinMin + (i*(m_BinMax - m_BinMin)) / plotWidth;
    *xmin = 0;
    *xmax = plotWidth;
    }

//now do the width and heights
if (largeFft)
    {
    // more FFT points than plot points
    for (i = minbin; i < maxbin; i++ ) //table is No of FFT ppoints wide
        {
        //y = (qint32)(dBGainFactor*(maxdB-pFFTAveBuf[i]));

        y = maxdB-pFFTAveBuf[i];

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
    // more plot points than FFT points
    for (x = 0; x < plotWidth; x++ ) //table is the size of screen width
        {
        i = m_pTranslateTbl[x]; // get plot to fft bin coordinate transform
        if(i < 0 || i >= m_FFTSize)
            y = plotHeight;
        else
          //  y = (qint32)(dBGainFactor*(maxdB-pFFTAveBuf[i]));
y = (qint32)maxdB - pFFTAveBuf[i];

        if (y > plotHeight)
            y = plotHeight;
        else if (y < 0)
            y = 0;
        outBuf[x] = y;
        }
    }
delete [] m_pTranslateTbl;
}

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

//---

qint64 ScopePlotter::freqFromX(int x) // convert screen coordinate to frequency
{
float  f_freq;
int freq;
float width = (float) m_OverlayPixmap.width();
int start_freq = g_center_frequency - g_sample_rate/2;

f_freq = ((float) g_sample_rate/width*x) +( float) start_freq;
f_freq /=ROUNDING_VAL;
freq = (int) roundf(f_freq);
freq *=ROUNDING_VAL;

g_center_frequency = freq; //???
printf(" emit of freq %d %d \n",g_center_frequency,__LINE__);
emit    newFrequency(freq); //send to world
return freq;
}

//---



//---
void ScopePlotter::setCenterFreq(quint64 f)
{
printf(" Set CF %d %d \n",f,__LINE__);
m_DrawOverlay = true;
g_center_frequency = f;
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

void ScopePlotter::drawOverlay() //bitmap grid and text that doesn't need every fft update.
{
int pw = m_WaterfallPixmap.width();
int ph = m_WaterfallPixmap.height() * 10;
int x,y;
float pix_per_bin;
float horiz_grid;
int high = m_FftPixmap.height();
double y_scale = (double) high/g_fft_range;
float fy;

y_scale *= (float) m_Percent2DScreen/100;

QRect rect;
QRect rect_c;
QPainter painter(&m_OverlayPixmap);
painter.initFrom(this);

plot_VerDivs = m_VerDivs;  
plot_HorDivs = m_HorDivs; 

QFont Font("Arial"); // create Font to use for scales
Font.setPointSize(m_FontSize);
QFontMetrics metrics(Font);
Font.setWeight(QFont::Normal);
painter.setFont(Font);

pix_per_bin = (float) pw/1024;
horiz_grid = pix_per_bin * 100;

y = ph ; //plot_height ; 
painter.setPen(QPen(QColor(0xF0,0xF0,0xF0,0x30), 1, Qt::DotLine));

//Draw centre line
for (int i = 1; i < 500; i++)
    {
    x = m_WaterfallPixmap.width()/2; 
    y=500;
    painter.drawLine(x, 0, x, y);
    }

//Draw freq grid from center to left and right
for (int i = 1,xl=pw/2, xr=pw/2; i < 6; i++)
    {
    xr += (int)horiz_grid;
    painter.drawLine(xr, 0, xr, y);
    xl -= (int)horiz_grid;
    painter.drawLine(xl, 0, xl, y);
    }

m_dBStepSize = 10;

//draw amplitude grid
for (int i = 1; i < 14; i++)
    {
    fy=  i*20*y_scale;
    y = (int) fy;
    painter.drawLine(5*metrics.horizontalAdvance("0",-1), y,m_FftPixmap.width(), y);
    }


//Draw freq values


painter.setPen(QColor(0xff,0xBA,0xA1,0xFF)); //d8,ba,a1
rect_c.setRect(pw/2-25,y+10 ,50,15); // (int)horiz_Pixperdiv, ph/plot_VerDivs);


painter.fillRect(rect_c,QColor(0,0,0,0xff));


//center digit
m_HDivText[20].setNum((float) g_center_frequency/1000000,'f', m_FreqDigits);
//painter.drawText(rect_c, Qt::AlignHCenter|Qt::AlignBottom, "    ");






painter.drawText(rect_c, Qt::AlignHCenter|Qt::AlignBottom, m_HDivText[20]);

//Draw freq strings from center to right
for (int i = 1,x=pw/2 ; i < 6; i++)
    {
    x += (int)horiz_grid;
    m_HDivText[i].setNum((float) g_center_frequency/1000000 + i*0.1 ,'f', m_FreqDigits);
    rect.setRect(x-25,y-30,50,50); 
    painter.drawText(rect, Qt::AlignHCenter|Qt::AlignBottom, m_HDivText[i]);
    }

//draw freq strings from center to left
for (int i = 1,x=pw/2   ; i < 6; i++)
    {
    x -= (int)horiz_grid;
    m_HDivText[i].setNum((float) g_center_frequency/1000000 - i * 0.1 ,'f', m_FreqDigits);
    rect.setRect(x-25,y-30 ,50,50);
    painter.drawText(rect, Qt::AlignHCenter|Qt::AlignBottom, m_HDivText[i]);
    }

// draw amplitude values
painter.setPen(QColor(0xD8,0xBA,0xA1,0xFF));
painter.setFont(Font);
int dB = m_MaxdB;

m_YAxisWidth = metrics.horizontalAdvance("-999 "); //a dummy to get width of chars
for (int i = 1; i < 14; i++)
    {
    fy=  i*20*y_scale;
    y = (int) fy;
    rect.setRect(0, y-metrics.height()/2, m_YAxisWidth, metrics.height());
    painter.drawText(rect, Qt::AlignRight|Qt::AlignVCenter, QString::number(dB));
    dB -= m_dBStepSize;  // move to end if want to include maxdb  
	}
		
if (!m_Running)
    {
    m_FftPixmap = m_OverlayPixmap.copy(0,0,pw,ph); //copy 2Dbitmap overlay bitmap.
    update(); // trigger a new paintEvent
    }
}//drawOverLay

//---


void ScopePlotter::mousePressEvent(QMouseEvent * event)
{

int mx;
//int my;
QPoint pt = event->pos();
mx = pt.x();
//my = pt.y();

freqFromX(mx); //updates the Freq from mouse x-pos
}




void ScopePlotter::mouseMoveEvent(QMouseEvent* event)
{
//int mx,my;	
//return;
//QPoint pt = event->pos();
//mx = pt.x();
//my = pt.y();
//printf(" x: %d y: %d \n",mx,my);
}

//---


// Called when a mouse wheel is turned
void ScopePlotter::wheelEvent(QWheelEvent * event)
{
//int mx,my;	
//QPoint pt = event->pos();
//mx = pt.x();
//my = pt.y();
//printf(" x: %d y: %d Ln: %d \n",mx,my,__LINE__);
//printf("Mouse wheelie workie \n");
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

 
