#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <pthread.h>
#include <liquid.h>
#include <fftw3.h>
#include <alsa/asoundlib.h>
#include <time.h>

#define FREQ 4
#define SRATE 5
#define ARATE 6
#define RFG 7
#define AFG 8
#define DMOD 9
#define SSEL 10


//#define SERVER "45.66.38.105"
#define SERVER "192.168.2.2" //222"
#define PORT 11361	
#define FFT_SIZE 1024

#define G711_SIZE 1024

#define BLOCK_SIZE 1024
#define CIRC_SIZE BLOCK_SIZE*4
#define AUDIO_SR 7812
#define AR_DELTA -10

#define MAX_PAK_LEN 1024+42
#define PAK_LEN 1024
#define FFT_HEAD_LEN 16
//#define AUDIO_HEAD_LEN 18
#define G711_HEAD_LEN 18
#define IQ_HEAD_LEN 20
#define FFT_LEN PAK_LEN+FFT_HEAD_LEN
//#define AUDIO_LEN PAK_LEN+AUDIO_HEAD_LEN
#define IQ_LEN PACK_LEN+IQ_HEAD_LEN

#define G711_LEN PAK_LEN+G711_HEAD_LEN

int g_audio_sample_rate;
int g_sample_rate;
int g_fft_size;
int g_center_frequency;

//char fft_circ_buf[FFT_SIZE];

int cb_in_ptr;
int cb_out_ptr;

bool stream_flag;
bool audio_flag;

extern double centerFreqVal;
static char *alsa_device = "default";  
snd_pcm_t *audio_device;
short audio_buffer[2048];

int control_packet[32];
struct sockaddr_in si_other;
int slen=sizeof(si_other);

char in_pak_buf[MAX_PAK_LEN];
char fft_video_buf[FFT_SIZE];
short g711_xfer_buf[G711_SIZE];
int status[10];
int audio_sr;
int audio_sr_delta;

//Timing
struct timespec begin_time, end_time; 

//adpcm
char g721_buf[BLOCK_SIZE * 4];
extern struct g72x_state enc_state;
extern struct g72x_state dec_state;

int sock_fd;

int audio_rxd_count;

//===


void send_control_packet(int type, int val)
{


control_packet[type] = val;

if (sendto(sock_fd, control_packet, sizeof(control_packet) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    die("control message");



}

//---

void * do_audio_pak(void)
{
while(1)
    {
    usleep(1000);
    if(audio_flag ==true)
        { //printf(" *\n");
        audio_flag = false;
        int snd_err = snd_pcm_writei(audio_device, g711_xfer_buf, 1024);	
        if(snd_err < 0 )
        {      
            printf(" Under run \n");
            snd_pcm_recover(audio_device, snd_err, 1); //catch underruns (silent flag set)
            usleep(1000);
            }
        }
    }
}

void * server_callback(void)
{
int i;
unsigned int fft_count;
int rxd_pak_len;
float audio;
int rbi;
static int local_count;
int rxd_count;
union c4
{
int iii;
char ccc[4];
}c4;
   
//get incoming samples from stream 
while(1) 
    {
    rxd_pak_len=recv(sock_fd, in_pak_buf ,MAX_PAK_LEN ,0); //CPX_DATA_SIZE
    switch(rxd_pak_len)
        {
        case FFT_LEN:

        for(int i=0; i<1024;i++)
            fft_video_buf[i] = in_pak_buf[i];
  
        stream_flag = true; //if I don't flag the FFT the CPU usage becomes 100% FIXME
        break;
           
        case G711_LEN:

        for(int i=0 ;i<1024;i++)
            {
            g711_xfer_buf[i] = alaw2linear(in_pak_buf[i]);
            }

c4.ccc[0] = in_pak_buf[1030];
c4.ccc[1] = in_pak_buf[1031];
c4.ccc[2] = in_pak_buf[1032];
c4.ccc[3] = in_pak_buf[1033];

//printf(" rxd pak: %d local: %d \n",c4.iii,local_count++);


        audio_flag = true;
        break;
            
        default:
        break;
        }

    usleep(1000);
    }
}

//---


void update_pitaya_cf(int cf)
{
float ppm_factor, freq;
int new_data;

freq = cf;
freq = (int)floor(freq*(1.0 + ppm_factor *1.0e-6) + 0.5);
printf(" new freq: %f \n",freq);
new_data = (int) freq;

send_control_packet(FREQ,new_data);


printf(" done changing F \n");
}	

void update_pitaya_sr(int sr)
{

printf(" update pitaya sample rate: %d  \n ",sr);	

send_control_packet(SRATE,sr);
}	

//---

void update_pitaya_ar(int ar)
{

send_control_packet(ARATE,ar);

printf(" Audio rate = %d\n",ar);
}


void update_pitaya_demod(int demod)
{

send_control_packet(DMOD,demod);

printf(" Demod type = %d\n",demod);

}

void update_pitaya_rfg(int gain)
{
send_control_packet(RFG,gain);
printf(" rf gain = %d\n",gain);

}

void update_pitaya_afg(int gain)
{
send_control_packet(AFG,gain);
printf(" AF gain = %d\n",gain);

}


//---




void die(char *s)
{
	perror(s);
	exit(1);
}

//---

int setup_network() 
{ 
int new_data;

char message[80];
int pkt_size;

strcpy(message,"CLIENT calling ");
if ( (sock_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	die("socket");
	
memset((char *) &si_other, 0, sizeof(si_other));
si_other.sin_family = AF_INET;
si_other.sin_port = htons(PORT);

//convert ip address to network byte order	
if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

if (sendto(sock_fd, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    die("sendto()");

printf(" Connected to the server:\n"); 
return 0;
} 

//---

void start_server_stream()
{
pthread_t callback_id,audio_cb_id;
int err;
int num_stages;
int ret;
int  freq;

read_conf();

g_sample_rate = 500000;
g_fft_size = FFT_SIZE;

freq = 198000;
audio_flag = false;
audio_sr = g_audio_sample_rate;
audio_sr_delta = AR_DELTA; //correction to let audio run a tad slower to keep its buffer filled.

err = snd_pcm_open(&audio_device, alsa_device, SND_PCM_STREAM_PLAYBACK, 0);
if(err !=0)
    printf("Error opening Sound Device\n");
err = snd_pcm_set_params(audio_device,SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,1,audio_sr,1,400000); //latency in uS - Could be dynamic to reduce (unwanted) latency?, 400 ok, 200 ng.
if(err !=0)
    printf("Error with Audio parameters\n"); //audio 

setup_network();
printf(" Network started \n");
usleep(10000);	

//Create a callback thread
ret=pthread_create(&callback_id,NULL, (void *) server_callback,NULL);

ret=pthread_create(&audio_cb_id,NULL, (void *)do_audio_pak,NULL);

if(ret==0)
	printf("Thread created successfully.\n");
else
	printf("Thread not created.\n");
}

//=======


