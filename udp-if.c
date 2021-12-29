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

#define SERVER "45.66.38.105"
//#define SERVER "192.168.2.2"
#define PORT 11361	
#define FFT_SIZE 1024
#define BLOCK_SIZE 1024
#define CIRC_SIZE BLOCK_SIZE*4
#define AR_DELTA -10

char fft_circ_buf[FFT_SIZE];

int cb_in_ptr;
int cb_out_ptr;

bool stream_flag;
extern double centerFreqVal;
static char *alsa_device = "default";  
snd_pcm_t *audio_device;
short audio_buffer[2048];
char fft_video_buf[FFT_SIZE];
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

//===

void * server_callback(void)
{
int i;
unsigned int fft_count;
int sock_ret;
float audio;
float _Complex lq_y;
double see_p, see_q,see,log_out, log_fft[FFT_SIZE];
int decim_phase;
int rbi;
double fmax,show_max,fmax_hold,fmax_avg;

decim_phase=0;
fft_count = 0;
rbi=0;

while(1)
    {
   
    //get incoming samples from stream 
    while(1) 
        {
        sock_ret=recv(sock_fd, fft_video_buf ,FFT_SIZE ,0); //CPX_DATA_SIZE
        //printf("sockret = %d \n",sock_ret);
       
        if(sock_ret==1024)
            {
           // printf(" Incoming FFT\n");
            stream_flag = true;
            }
        }
     }
}

/*
    //reorder the FFT
    for(int i = 0;i<FFT_SIZE/2;i++)
        fft_video_buf[i] = -100 +(short) log_fft[(FFT_SIZE/2)+i];
    for(int i = FFT_SIZE/2,j=0; i<FFT_SIZE;i++,j++)
       fft_video_buf[i] = -100 +(short)log_fft[j];
*/

//temp insertion of fft video compression

//G721 encode
  //      for(int i=0;i<FFT_SIZE;i++)
  //          {
   //         g721_buf[i] = (char) g721_encoder((fft_video_buf[i]*256), & enc_state);
   //         }

//G721 decode
    //    for(int i=0;i<FFT_SIZE;i++)
     //       {
      //      fft_video_buf[i] = (short)( g721_decoder(g721_buf[i], & dec_state))/256;
     //       }

//end of temp




/*
        {     
        rbi=0;   
 	    //audio demod
	    for(i=0;i<BLOCK_SIZE; i++) 
            {
	        lq_y =  resamp_buf[i] * 32768;
		    ampmodem_demodulate(demod, lq_y, &audio); //demodulate to audio
		    audio_buffer[i] = (short)(audio * 1024) ;
	        }
 */
/*a law encode/decode
        char a_byte;
        for(int i=0;i<BLOCK_SIZE;i++)
            {
            a_byte = linear2alaw(audio_buffer[i]);
            audio_buffer[i] = alaw2linear(a_byte);
            }
*/

/*
//temporary insertion of G721 encoder and decode to test
//G721 encode
        for(int i=0;i<BLOCK_SIZE;i++)
            {
            g721_buf[i] = (char) g721_encoder(audio_buffer[i], & enc_state);
            }

//G721 decode
        for(int i=0;i<BLOCK_SIZE;i++)
            {
            audio_buffer[i] = (short) g721_decoder(g721_buf[i], & dec_state);
            }
//end of g721 test
*/

/*
        int snd_err = snd_pcm_writei(audio_device, audio_buffer, BLOCK_SIZE);	
        if(snd_err < 0 )
            {
            snd_pcm_recover(audio_device, snd_err, 1); //catch underruns (silent flag set)
	        usleep(1000);
		    }
        }//decim phase		
*/
    //if(fft_count >1) //timer needed? or rely on MainWindow timer?
        
 
    //Show via GUI stream
 //   status[1] = (int) show*1000;
 
  //  stream_flag = true;
  //  }//while 1
//

//---


void update_pitaya_cf(int cf)
{
float ppm_factor, freq;
int new_data;

freq = cf; //8000000; //5505000;
ppm_factor = 0; 
freq = (int)floor(freq*(1.0 + ppm_factor *1.0e-6) + 0.5);
printf(" new freq: %f \n",freq);
new_data = (int) freq;

printf(" done changing F \n");
}	

void update_pitaya_sr(int sr)
{
int new_data;

printf(" update pitaya sample rate: %d  \n ",sr);	
new_data = 0x10000000;
new_data |= (sr & 0x0007);
printf (" --> 0x%0x \n",new_data);
}	

//---

void update_pitaya_decim(int decim)
{
int new_data;
new_data = decim;
//no setting for Harware decimation is available (as yet?)
printf(" Nothing to do %d\n",new_data);
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
struct sockaddr_in si_other;
int slen=sizeof(si_other);
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
pthread_t callback_id;
int err;
int num_stages;
int ret;
int  freq;
freq = 198000;

audio_sr_delta = AR_DELTA; //correction to let audio run a tad slower to keep its buffer filled.
err = snd_pcm_open(&audio_device, alsa_device, SND_PCM_STREAM_PLAYBACK, 0);
if(err !=0)
    printf("Error opening Sound Device\n");
err = snd_pcm_set_params(audio_device,SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,1,audio_sr+audio_sr_delta,1,400000); //latency in uS - Could be dynamic to reduce (unwanted) latency?
if(err !=0)
    printf("Error with Audio parameters\n"); //audio 

setup_network();
printf(" Network started \n");
usleep(10000);	

//Create a callback thread
ret=pthread_create(&callback_id,NULL, (void *) server_callback,NULL);
if(ret==0)
	printf("Thread created successfully.\n");
else
	printf("Thread not created.\n");
}

//=======
/*
cb_in(int num_samples)
{
int n;
for(n=0;n<num_samples;n++)
    {
    cb_in_ptr++;
    cb_in_ptr &= CIRC_SIZE-1; //roll over
    }
}

cb_out(int num_samples)
{
int n;
for(n=0;n<BLOCK_SIZE;n++)
    {
    cb_out_ptr++;
    cb_out_ptr &= CIRC_SIZE-1; //roll over

fft_video_buf[n] = fft_circ_buf[cb_out_ptr];
printf(" cb_out %d \n",cb_out_ptr);

    }
}

int cb_delta()
{
int ret;
ret = cb_in_ptr - cb_out_ptr;
if ( ret < 0) 
    ret += CIRC_SIZE;
printf(" cb_delta %d \n",ret);

return ret;
}

*/


