
/*****************************************************************

 Copyright 2001   PIER LUCA MONTESSORO

 University of Udine
 ITALY

 montessoro@uniud.it
 www.montessoro.it

 This file is part of a freeware open source software package.
 It can be freely used (as it is or modified) as long as this
 copyright note is not removed.

******************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"


void  WriteWave (WAVE wave, FILE *fp)
{
   unsigned long int i;

   fwrite (&wave.riffheader, sizeof (RIFFHEADER), 1, fp);
   fwrite (&wave.fmtheader, sizeof (FMTHEADER), 1, fp);
   fwrite (&wave.wavedata.dataid, 4, 1, fp);
   fwrite (&wave.wavedata.DataSize, 4, 1, fp);

   for (i = 0; i < wave.numofstereosamples; i++)
   {
      fwrite (&SAMPLE(wave, LEFT, i), sizeof (signed short int), 1, fp);
      fwrite (&SAMPLE(wave, RIGHT, i), sizeof (signed short int), 1, fp);
   }

#ifdef WAVSHOWALL
   printf ("%d stereo samples written\n", i);
#endif

   return;
}


WAVE  ReadWave (FILE *fp)
{
   WAVE wave;
   unsigned long int i;

   fread (&wave.riffheader, sizeof (RIFFHEADER), 1, fp);
   fread (&wave.fmtheader, sizeof (FMTHEADER), 1, fp);
   fread (&wave.wavedata.dataid, 4, 1, fp);
   fread (&wave.wavedata.DataSize, 4, 1, fp);

   if (strncmp (wave.riffheader.riffid, RIFF_ID, 4))
   {
      printf ("Wrong RIFF identifier\n");
      exit (EXIT_FAILURE);
   }

   if (strncmp (wave.riffheader.waveid, WAV_ID, 4))
   {
      printf ("Wrong WAVE identifier\n");
      exit (EXIT_FAILURE);
   }

   if (strncmp (wave.fmtheader.fmtid, FMT_ID, 4))
   {
      printf ("Wrong FORMAT identifier\n");
      exit (EXIT_FAILURE);
   }

   if (wave.fmtheader.format != FMTPCM)
   {
      printf ("Unsupported format type\n");
      exit (EXIT_FAILURE);
   }

   if ((wave.fmtheader.channels != 2) ||
       (wave.fmtheader.SampleRate != SAMPLINGRATE) ||
       (wave.fmtheader.BitsPerSample != BITSPERSAMPLE))
   {
      printf ("Only CD audio wave files (44.1 KHz, 16 bit/sample, stereo)\n"
              "are currently supported\n");
      exit (EXIT_FAILURE);
   }

   if (strncmp (wave.wavedata.dataid, DATA_ID, 4))
   {
      printf ("Wrong DATA identifier\n");
      exit (EXIT_FAILURE);
   }

   wave.numofstereosamples = wave.wavedata.DataSize / (CHANNELS * BITSPERSAMPLE / 8);

   wave.wavedata.sample = (signed short int *) malloc (wave.wavedata.DataSize);
   if (wave.wavedata.sample == NULL)
   {
      printf ("Memory allocation error\n");
      exit (EXIT_FAILURE);
   }

   for (i = 0; i < wave.numofstereosamples; i++)
   {
      fread (&SAMPLE(wave, LEFT, i), sizeof (signed short int), 1, fp);
      fread (&SAMPLE(wave, RIGHT, i), sizeof (signed short int), 1, fp);
   }

#ifdef WAVSHOWALL
   printf ("%d stereo samples read\n", i);
#endif

   return wave;
}
   

void ReleaseWaveData (WAVE *wave)
{
   free ((*wave).wavedata.sample);
   (*wave).riffheader.FileSize = 0;
   (*wave).wavedata.DataSize = 0;
   return;
}


WAVE  CreateEmptyCDaudioWave (unsigned long int numofstereosamples)
{
   WAVE wave;

#ifdef WAVSHOWALL
   printf ("Creating %lf seconds empty wave\n", 
           numofstereosamples / (double) SAMPLINGRATE);
#endif

   /* RIFF header */

   wave.riffheader.riffid[0] = 'R';
   wave.riffheader.riffid[1] = 'I';
   wave.riffheader.riffid[2] = 'F';
   wave.riffheader.riffid[3] = 'F';
   wave.riffheader.FileSize = 12 + 24 + 8 + numofstereosamples * 2 * 2;
   wave.riffheader.waveid[0] = 'W';
   wave.riffheader.waveid[1] = 'A';
   wave.riffheader.waveid[2] = 'V';
   wave.riffheader.waveid[3] = 'E';

   /* FMT (format) header */

   wave.fmtheader.fmtid[0] = 'f';
   wave.fmtheader.fmtid[1] = 'm';
   wave.fmtheader.fmtid[2] = 't';
   wave.fmtheader.fmtid[3] = ' ';
   wave.fmtheader.fmtsize = 16;
   wave.fmtheader.format = FMTPCM;
   wave.fmtheader.channels = 2;
   wave.fmtheader.SampleRate = SAMPLINGRATE;
   wave.fmtheader.BytesPerSecond = SAMPLINGRATE * CHANNELS * BITSPERSAMPLE / 8;  
                                   /* sample rate * block align */
   wave.fmtheader.BlockAlign = CHANNELS * BITSPERSAMPLE / 8;
                               /* channels * bits/sample / 8 */
   wave.fmtheader.BitsPerSample = BITSPERSAMPLE;

   /* WAVEDATA */

   wave.wavedata.dataid[0] = 'd';
   wave.wavedata.dataid[1] = 'a';
   wave.wavedata.dataid[2] = 't';
   wave.wavedata.dataid[3] = 'a';
   wave.wavedata.DataSize = numofstereosamples * CHANNELS * BITSPERSAMPLE / 8;

   wave.numofstereosamples = numofstereosamples;
   wave.wavedata.sample = (signed short int *)
                  malloc (numofstereosamples * CHANNELS * BITSPERSAMPLE / 8);
   if (wave.wavedata.sample == NULL)
   {
      printf ("Memory allocation error\n");
      exit (EXIT_FAILURE);
   }

   return wave;
}
   
