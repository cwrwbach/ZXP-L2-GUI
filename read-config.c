#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//these are the global variables to be updated (will be extern)
int l_freq;
int sr;
extern int g_audio_sample_rate;

char vname[256][100];
char value[256][100];

void read_conf()
{
static FILE *conf_file;
char str[200];
int index;
char delim_char;
char * sf;
FILE *fp = fopen("gui.cnf", "r");

printf(" \n Config update:\n");

//testing only ... remove
l_freq = 123456;
sr = 2048;
//audio = 16000;


index = 0;

//get one line at a time and parse for strings
while(fgets(str, sizeof(str), fp) != NULL) 
    {
    if(str[0] == '[') continue; //ignore commented lines
    if(str[0] == '#') continue;
    if(str[0] == '/') continue;

    for(int sp =0;sp < strlen(str); sp++)
        {
        delim_char = str[sp];
        
        //split string into deliminated parts 
        if(delim_char == '=') //equals sign is delim
            {
            strncpy(vname[index],str,sp);
            strcpy(value[index],str+sp+1);
            break;
            }
        }   
 
    index ++;
    }//while fgets

//Test each of the variable names aginst index
for(int i=0 ; i<index;i++)
    {
    if(strcmp(vname[i],"l_freq")==0)
       l_freq = strtol(value[i],&sf,10); 
    
    if(strcmp(vname[i],"sr")==0)
        sr = strtol(value[i],&sf,10);
    
    if(strcmp(vname[i],"g_audio_sample_rate")==0)
        g_audio_sample_rate = strtol(value[i],&sf,10);
    }

printf("--------------\n");

printf(" Freq: %d \n",l_freq);
printf(" SR: %d \n",sr);
printf(" Audio: %d \n",g_audio_sample_rate);



}

