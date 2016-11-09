/* Copyright (c) 2016 Andrea Drius
 * This file is under MIT License
 *
 * Read the LICENSE file for more information*/



#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "wav.h"

#define M 2
#define M2 4
#define N 10
#define SR 44100
#define DB 12
#define R 2
#define F_MIN 30


void preprocessing(double c[N][10]);
double filt(double u,double par[],double c[N][10],double x[N][M2]);


int main(int argc, char **argv)
{
	double c[N][10];
	double xL[N][M2]={0};
	double xR[N][M2]={0};
	double par[N]={-1,0,1,0,-1,0,1,0,-1,0};

	WAVE inp,out;
	FILE *finp, *fout;
	char outfname[100];
	unsigned i;


	if(argc != 2)
	{
		puts("filename of a valid stereo wav file is required, sampling rate must be 44100Hz");
		return EXIT_FAILURE;
	}

	preprocessing(c); /*initialize filters*/
	
	finp=fopen(argv[1],"rb");

	sprintf(outfname,"%s_out.wav",argv[1]);
	fout=fopen(outfname,"wb");

	inp=ReadWave(finp);
	out=CreateEmptyCDaudioWave(inp.numofstereosamples);

	for(i=0;i<inp.numofstereosamples;i++)
	{
		SAMPLE(out,LEFT,i)=filt(SAMPLE(inp,LEFT,i),par,c,xL);
		SAMPLE(out,RIGHT,i)=filt(SAMPLE(inp,RIGHT,i),par,c,xR);
	}


	WriteWave(out,fout);
	fclose(fout);
	fclose(finp);

	ReleaseWaveData(&inp);
	ReleaseWaveData(&out);

	return EXIT_SUCCESS;
}


void preprocessing(double c[N][10])
{
	double v,g,cw,wcross,wc_n,fc_n,f_max,bw_n,T,tbw,c_m,d;
	double a[3], b[3];
	int n;

	T=1.0/SR;

	g=pow(10,DB/20.0);
	wcross=pow(g,1.0/2.0/M);
	v=pow(g,1.0/M)-1;
	f_max=F_MIN*pow(R,N-1);

	for(n=0;n<N;n++)
	{
		fc_n=round(exp(log(F_MIN)+log(f_max/(double)F_MIN)*(n-1)/(double)(N-1)));
		wc_n=2*M_PI*fc_n;
		bw_n=wc_n*(sqrt(R)-1.0/sqrt(R))/wcross;

		cw=cos(wc_n*T);
		tbw=T*bw_n;
		c_m=cos(M_PI*(0.5-0.5/M));

		a[0]=4+4*c_m*tbw+tbw*tbw;
		a[1]=a[2]=1.0/a[0];
		a[1]*=2*tbw*tbw-8;
		a[2]*=a[0]-8*c_m*tbw;

		b[0]=b[1]=b[2]=tbw*v/a[0];
		b[0]*=2*tbw+4*c_m+tbw*v;
		b[1]*=2*tbw*(v+2);
		b[2]*=2*tbw-4*c_m+tbw*v;

		d=b[0]+1;
		c[n][0]=cw*(1-a[1]);
		c[n][1]=cw;
		c[n][2]=cw*(b[1]-a[1]*b[0]);
		c[n][3]=b[1]-a[1]*b[0];
		c[n][4]=-a[1];
		c[n][5]=cw*(b[2]-a[2]*b[0]);
		c[n][6]=-a[2]*cw;
		c[n][7]=b[2]-a[2]*b[0];
		c[n][8]=-a[2];
		c[n][9]=d;
	}

}

double filt(double u,double par[],double c[N][10],double x[N][M2])
{
	double gg,gg1,gg2,den,u0,xn2cn1;
	double xn[M2];
	int n,i;
	double y=u;

	for(n=0;n<N;n++)
	{
		gg=2*par[n];
		gg1=par[n]+1;
		gg2=par[n]-1;

		den=gg1-gg2*c[n][9];
		u0=(gg2*x[n][0]+2*y)/den;
		y=(gg*x[n][0]+(gg1*c[n][9]-gg2)*y)/den;

		xn2cn1=x[n][2]*c[n][1];

		xn[0]=x[n][0]*c[n][0]-x[n][1]+xn2cn1+c[n][2]*u0;
		xn[1]=x[n][0]*c[n][4]+x[n][2]+c[n][3]*u0;
		xn[2]=x[n][0]*c[n][6]+xn2cn1-x[n][3]+u0*c[n][5];
		xn[3]=x[n][0]*c[n][8]+u0*c[n][7];


		for (i=0;i<M2;i++)
			x[n][i]=xn[i];
	}
	return y;
}

