#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <stdbool.h>

#define PAK_SIZE 2048

// Global variables
int level_2_socket;
struct sockaddr addr;

bool stream_flag;

char l2_video_buf[1024];

// Create raw socket
void create_socket()
{
if ((level_2_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
    printf(" Could not create socket!\n");
    exit(0);
    }
}
/*
void sniffer() // Receive network packet
{
int pak_size;
socklen_t addr_size = sizeof(addr);
memset(&addr, 0, sizeof(addr));
unsigned char pak_buf[PAK_SIZE];
memset(pak_buf, 0, PAK_SIZE);

//while (1)
    {
    if ((pak_size = recvfrom(level_2_socket, pak_buf, PAK_SIZE, 0, &addr, &addr_size)) < 0)
        break;
    else
        {
        printf(" Caught a packet: %d %d %x %x \n",addr_size,pak_size, pak_buf[12], pak_buf[13] );
        }
    }
    
printf("Closing socket. \n");
close(level_2_socket);
exit(0);
}
*/





void * server_callback(void)
{
int i;
unsigned int fft_count;
int rxd_pak_len;
float audio;
int rbi;


int pak_size;
socklen_t addr_size = sizeof(addr);
memset(&addr, 0, sizeof(addr));
unsigned char pak_buf[PAK_SIZE];
memset(pak_buf, 0, PAK_SIZE);


while(1)
    {
    sleep(1);
    printf(" callback lupin \n");
 
    if ((pak_size = recvfrom(level_2_socket, pak_buf, PAK_SIZE, 0, &addr, &addr_size)) < 0)
        {
        printf(" Line 82 \n");
        }
     //   break;
    else
        {
        printf(" Caught a packet: %d %d %x %x \n",addr_size,pak_size, pak_buf[12], pak_buf[13] );
        }


for(int v=0;v<1024;v++)
    l2_video_buf[v] = v;


 stream_flag = true;
    }




/*   
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
        audio_flag = true;
        break;
            
        default:
        break;
        }

    usleep(1000);
    }
*/
}





void xxmain()
{
pthread_t callback_id,audio_cb_id;
int err;
int num_stages;
int ret;
int  freq;


system("clear");
printf(" Level 2 Packet Sniffer \n");
create_socket();


ret=pthread_create(&callback_id,NULL, (void *) server_callback,NULL);

//sniffer();
}

