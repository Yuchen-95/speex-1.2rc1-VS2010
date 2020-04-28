#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"


#define NN 128
#define TAIL 2048

int main(int argc, char **argv)
{
   FILE *echo_fd, *ref_fd, *e_fd, *status_f;
   short echo_buf[NN], ref_buf[NN], e_buf[NN];
   SpeexEchoState *st;
   SpeexPreprocessState *den;
   int sampleRate = 8000;
   int status;
   int frz_flag = 0;
   int max_process_fr_num = 20 * 8000 / NN;
   int max_update_fr_num = 5 * 8000 / NN;
   int max_stop_fr_num = 10;

   //printf("%d ", max_process_fr_num);
   //printf("%d ", max_stop_fr_num);
   if (argc != 4)
   {
      fprintf(stderr, "testecho mic_signal.sw speaker_signal.sw output.sw\n");
      exit(1);
   }
   echo_fd = fopen(argv[2], "rb");
   ref_fd  = fopen(argv[1],  "rb");
   e_fd    = fopen(argv[3], "wb");

   //status_f = fopen("status.txt", "w");

   st = speex_echo_state_init(NN, TAIL);
   den = speex_preprocess_state_init(NN, sampleRate);
   speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
   speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);

   short gabbage[320];
   //fread(gabbage, sizeof(short), 320, ref_fd);

   int total_count = 0; //track total number of frames processed
   int count_update = 0; 
   int count_stop = 0;
   int stop_flag = 0;
   while (!feof(ref_fd) && !feof(echo_fd))
   {
      fread(ref_buf, sizeof(short), NN, ref_fd);
      fread(echo_buf, sizeof(short), NN, echo_fd);
	  frz_flag = 0;
      status = speex_echo_cancellation(st, ref_buf, echo_buf, e_buf, frz_flag);
	  printf("%d ",status);

	  total_count += 1;
	  count_update += status;

	  if (frz_flag == 0)
	  {
		  if (!(total_count < max_process_fr_num && count_update < max_update_fr_num))
		  {
			  stop_flag = 1;
		  }

		  if (stop_flag)
		  {
			  if (status == 1) {
				  count_stop = 0;
			  }
			  else {
				  count_stop += 1;
			  }
		  }

		  if (count_stop > max_stop_fr_num)
		  {
			  frz_flag = 1;
		  }
	  }
	  
	  //fwrite(&status, sizeof(int), 1, status_f);

	  //status_f = fopen("status.txt", "a");
	  //fprintf(status_f, "%d ", status);
	  //fclose(status_f);
	  //fclose(fp);
	  //putw(status, status_f);
      speex_preprocess_run(den, e_buf);
      fwrite(e_buf, sizeof(short), NN, e_fd);
   }
   speex_echo_state_destroy(st);
   speex_preprocess_state_destroy(den);
   fclose(e_fd);
   fclose(echo_fd);
   fclose(ref_fd);
   //fclose(status_f);
   return 0;
}
