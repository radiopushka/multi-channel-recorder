#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <signal.h>

//original code by Evan Nikitin

struct Interfaces{//audio interface
 struct Interface *in;
 int file;
 char *name;
 char *fullname;
 struct Interfaces *next;
};

struct Interface{//audio interface data for reading
 snd_pcm_t *capture_handle;
 int size;
 char* buffer;
};


struct Interface* openRec(char* interface){//initialize
  struct Interface *in=malloc(sizeof(struct Interface));
  int err;//generic quick and dirty open the audio interface for recording, if something fails then oh well, it will throw an error later and quit the program.
  unsigned int rate = 44100;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_format_t format = SND_PCM_FORMAT_FLOAT;//from the alsa github recorder demo
   snd_pcm_open (&capture_handle, interface, SND_PCM_STREAM_CAPTURE, 0);
  snd_pcm_hw_params_malloc (&hw_params);
  snd_pcm_hw_params_any (capture_handle, hw_params);
  snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format (capture_handle, hw_params, format);
  snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0);
  snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2);
  snd_pcm_hw_params (capture_handle, hw_params);
  snd_pcm_hw_params_free (hw_params);
  snd_pcm_prepare (capture_handle);
  in->buffer= malloc(128 * snd_pcm_format_width(format) / 8 * 2);
  in->size=128 * snd_pcm_format_width(format) / 8 * 2;
  in->capture_handle=capture_handle;
  return in;
}

void nullify(char *buff,int size){
 char *ptr=buff;
 char *end=ptr+size;
 for(ptr=buff;ptr<end;ptr++){
  *ptr=0;
 }
}
struct Interfaces *intf=NULL;
static volatile int keepRunning = 1;//from a tutorial on interrupts
void intHandler(int dummy) {
  keepRunning=0;//loop interrupter
  printf("exiting\n");
}


char* createName(int value,char *end){
 int length=snprintf(NULL,0,"%d",value);
 char *str=calloc(length+3+strlen(end),sizeof(char));
 snprintf(str,length+3+strlen(end),"._%d%s",value,end);
 return str;
}

char *namelist;//list of wav files in order for mass delete later with rm
int main(int argn,char* argv[]){

   int i=argn-1;
   if(i<2){
    printf("%s <channel 1> <channel 2> ...\n",argv[0]);
    return 0;
   }
   int i2;
   int size;
   for(i2=0;i2<i;i2++){
    printf("%s\n",argv[i2+1]);
    struct Interface *rec=openRec(argv[i2+1]);
    size=rec->size;
    struct Interfaces *ins=malloc(sizeof(struct Interfaces));
    char *name=createName(i2+1,".pcm");
    char *wname=createName(i2+1,".wav");
    FILE *fr=fopen(name,"w");
    fwrite(NULL,0,1,fr);
    fclose(fr);
    int f=open(name,O_WRONLY);
    if(namelist==NULL){
     namelist=calloc(strlen(wname)+1,sizeof(char));
     strcpy(namelist,wname);
    }else{
     char *nnlist=calloc(strlen(namelist)+strlen(wname)+2,sizeof(char));
     strcpy(nnlist,namelist);
     free(namelist);
     strcat(nnlist," ");
     strcat(nnlist,wname);
     namelist=nnlist;
    }
    char *hname=createName(i2+1,"");
    ins->name=hname;
    ins->fullname=name;
    ins->file=f;
    ins->next=intf;
    ins->in=rec;
    intf=ins;
   }
   struct Interfaces *ptr=intf;
   signal(SIGINT, intHandler);
   while(keepRunning==1){
    ptr=intf;

    while(ptr!=NULL){
     struct Interface *rec=ptr->in;
     snd_pcm_readi (rec->capture_handle, rec->buffer, 128);
     write(ptr->file,rec->buffer,size);//write into separate files, I tried to have merged streams but I couldn't figure out how to do it
     ptr=ptr->next;
    }
    

   }

  ptr=intf;
  while(ptr!=NULL){
   struct Interface *rec=ptr->in;
   free(rec->buffer);
   
   snd_pcm_close (rec->capture_handle);
   free(rec);
   struct Interfaces *next=ptr->next;
   close(ptr->file);
   char *pre="ffmpeg -f f32le -ar 44.1k -ac 2 -i ";//use ffmpeg to conver pcm to wav
   char *suffix=".wav";
   char *mid=" -ar 44100 -ac 2 ";
   char *command=calloc(strlen(pre)+strlen(ptr->name)+strlen(ptr->fullname)+strlen(mid)+strlen(suffix)+1,sizeof(char));
   strcpy(command,pre);
   strcat(command,ptr->fullname);
   strcat(command,mid);
   strcat(command,ptr->name);
   strcat(command,suffix);
   system(command);
   free(command);
   remove(ptr->fullname);
   free(ptr->name);
   free(ptr->fullname);
   free(ptr);
   ptr=next;
  }
  char *pre="sox -m ";//merge all the files
  char *suffix=" out.wav";
  char *command=calloc(strlen(pre)+strlen(namelist)+strlen(suffix)+1,sizeof(char));
  strcpy(command,pre);
  strcat(command,namelist);
  strcat(command,suffix);
  system(command);
  free(command);
  pre="rm ";
  command=calloc(strlen(pre)+strlen(namelist)+1,sizeof(char));
  strcpy(command,pre);
  strcat(command,namelist);
  system(command);
  free(command);
  return 0;

}
