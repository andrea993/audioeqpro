
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


#include <stdint.h>

/* Windows wave files (.WAV), 16 bit/sample, 44.1KHz, 2 channels */

#ifndef MMLIB_DATATYPES_DEFINED
typedef  uint8_t       byte;
typedef  uint16_t      word;
typedef  uint32_t      dword;
#define MMLIB_DATATYPES_DEFINED
#endif


#define FMTPCM        1
#define SAMPLINGRATE  44100
#define CHANNELS      2
#define BITSPERSAMPLE 16

#define LEFT  0
#define RIGHT 1

#define RIFF_ID  "RIFF"
#define WAV_ID   "WAVE"
#define FMT_ID   "fmt "
#define DATA_ID  "data"


#ifdef GENERIC_COMPILER

/* header structure definitions */

typedef struct tagRIFFHEADER
{
   char  riffid[4];
   dword FileSize;
   char  waveid[4];
} RIFFHEADER;

typedef struct tagFMTHEADER
{
   char  fmtid[4];
   dword fmtsize;
   word  format;
   word  channels;
   dword SampleRate;
   dword BytesPerSecond;
   word  BlockAlign;
   word  BitsPerSample;
} FMTHEADER;

#endif


#ifdef __GNUC__

/* header structure definitions for GCC on Intel architectures,
   to avoid 32-bit alignement of 16-bit words */

typedef struct tagRIFFHEADER
{
   char  riffid[4];
   dword FileSize                  __attribute__ ((packed));
   char  waveid[4];
} RIFFHEADER;

typedef struct tagFMTHEADER
{
   char  fmtid[4];
   dword fmtsize                   __attribute__ ((packed));
   word  format                    __attribute__ ((packed));
   word  channels                  __attribute__ ((packed));
   dword SampleRate                __attribute__ ((packed));
   dword BytesPerSecond            __attribute__ ((packed));
   word  BlockAlign                __attribute__ ((packed));
   word  BitsPerSample             __attribute__ ((packed));
} FMTHEADER;

#endif


typedef struct tagWAVEDATA
{
   char  dataid[4];
   dword DataSize;
   signed short int *sample;
} WAVEDATA;


typedef struct tagWAVE
{
   RIFFHEADER riffheader;
   FMTHEADER fmtheader;
   unsigned long int numofstereosamples;
   WAVEDATA wavedata;
} WAVE;



/* useful macro */

#define SAMPLE(wave, channel, offset)  \
        (wave).wavedata.sample [2 * (offset) + (channel)]

/* functions prototipes */

void    WriteWave (WAVE wave, FILE *fp);
WAVE    ReadWave (FILE *fp);
WAVE    CreateEmptyCDaudioWave (unsigned long int numofstereosamples);
void    ReleaseWaveData (WAVE *wave);

