Return-path: <CHENP@CSMC.EDU>
Received: from mvax1.csmc.edu by auckland.ac.nz (PMDF #2864 ) id
 <01H38EKPKP7K8Y62SP@auckland.ac.nz>; Wed, 22 Sep 1993 02:40:58 +1200
Received: from CSMC.EDU by CSMC.EDU (PMDF V4.2-11 #3833) id
 <01H37ANS8K4G8ZDZJK@CSMC.EDU>; Tue, 21 Sep 1993 07:38:02 PST
Date: 21 Sep 1993 07:38:01 -0800 (PST)
From: "Peng Chen, M.D. x4851 Cardiology" <CHENP@CSMC.EDU>
Subject: conversion file you requested
To: d.bullivant@auckland.ac.nz
Cc: CHENP@CSMC.EDU
Message-id: <01H37ANSA6028ZDZJK@CSMC.EDU>
X-VMS-To: IN%"d.bullivant@aukuni.ac.nz"
X-VMS-Cc: CHENP
MIME-version: 1.0
Content-type: TEXT/PLAIN; CHARSET=US-ASCII
Content-transfer-encoding: 7BIT

#include <iostream.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>


#define WINDOWLENGTH 1040000L   // Number of bytes in a BARD window.dat file
#define MAX_SAMPLES   8000      // Maximum number of samples for BARD file
#define MAX_CHANNELS  64        // Maximum number of data channels for BARD
#define BARD_CHANNELS 65        // Number of channels in BARD window.dat file
#define BUFFER_SIZE   16384     // Size of the output file disk buffer
#define NEXT          128       // Bytes to next data (BARD_CHANNELS-1)*sizeof(int)

FILE *infile,*outfile;

union convert {
      float f;
      long  l;
      int   i[2];
      char  c[4];
};

union convert2 {
      int  i;
      char c[2];
};

void load_short(union convert2 *value);
void load(union convert *value);
void output(union convert2 *value);


void load_emap (void) {

     long           i,j;
     unsigned int   temp;
     union convert  rig_type,length,regions,devices,device_type;
     union convert  pages,signals,samples,freq;
     float          msec_samples;
     char           direct[40],direct2[40],string[300];
     union convert2 data;

     char           buffer[BUFFER_SIZE];


     cout << "Please enter the name of the EMAP data file: ";
     cin >> direct;

     if ((infile=fopen(direct,"rb"))==NULL) {
        cout << "Error opening file named " << string;
        exit(1);
     }
     cout << "Please specify an empty directory or disk to write to.\n";
     cout << "Use '.' for the current directory: ";
     cin >> direct;


     load(&rig_type);                                  // 0=sock, 1=patch
     load(&length);                                    // length for name
     j=fread(string,1,(int) length.l,infile);                // read name
     strcpy(direct2,direct);
     strcat(direct2,"\\patient.nam");
     if ((outfile=fopen(direct2,"wt")) == NULL) {
        cout << "Error opening file named 'patient.nam'.  Program aborted.\n";
        exit(1);
     }
     fwrite(string,1,(int) length.l,outfile);             // Write patient.nam file
     strcpy(string," no number");
     fwrite(string,1,strlen(string),outfile);
     fclose(outfile);

     // If rig_type==sock==0 then read focus
     if (!rig_type.l) load(&freq);

     load(&regions);                                // number of regions
     for (i=0;i<regions.l;i++) {
         load(&length);                             // Length of region name
         fread(string,1,(int) length.l,infile);           // region name
         string[(int) length.l]=NULL;
         load(&devices);                            // # of devices in region
         for (j=0;j<devices.l;j++) {
             load(&device_type);                  // device type
             load(&length);                       // device number
             load(&length);                       // length of device name
             fread(string,1,(int) length.l,infile);     // device name
             string[(int) length.l]=NULL;
             load(&length);                       // channel_number
             if (!device_type.l) {                // if device is an electrode
                load(&freq);                      // read x pos
                load(&freq);                      // read y pos
                if (!rig_type.l)                  // if rig is a sock
                   load(&freq);                   // read z pos
             }
         }
     }

     load(&pages);                                // number of pages
     for (i=0;i<pages.l;i++) {
         load(&length);                           // length of page name
         fread(string,1,(int) length.l,infile);         // page name
         string[(int) length.l]=NULL;
         load(&devices);                          // number of devices
         for (j=0;j<devices.l;j++)
             load(&length);                       // device number
     }

     load(&signals);                              // number of signals
     load(&samples);                              // number of samples
     load(&freq);                                 // sampling frequency
                                   // *** ASSUMING samples per second
     for (i=0;i<samples.l;i++)
         load(&pages);                            // actual sample times

     msec_samples=1000/freq.f;                    // milliseconds per sample

     strcpy(string,direct);
     strcat(string,"\\window.dat");               // Open output file
     outfile=fopen(string,"wb");
     if (setvbuf(outfile,buffer,_IOFBF,BUFFER_SIZE)!=0) {
        cout << "Error - not enough memory.\n";
        exit(1);
     }

     cout << "Initializing window.  One moment.";
     temp=2048;    // Initialize output file to 2048s which are zeros in BARD
     for (i=0;i<WINDOWLENGTH/2L;i++)
         fwrite((char *)&temp,2,1,outfile);
     rewind(outfile);

     temp=0xC000;
     for (i=0;i<MAX_SAMPLES;i++) {
        // Set error channel to 0xC000 which means no error.
        fwrite(&temp,2,1,outfile);
        fseek(outfile,(long) NEXT,SEEK_CUR);
     }
     cout << "\nDone.\n";


     /******************** READ/WRITE DATA ********************/
     fseek(outfile,2,SEEK_SET);
     for (i=0;i<samples.l;i++) {           // Cycle through samples
        fseek(outfile,2+130*i,SEEK_SET);
        for (j=0;j<signals.l;j++) {        // Cycle through signals
           load_short(&data);
           if (j<MAX_CHANNELS) {           // Use only first 64 signals
              cprintf("sample %ld, signal %ld \r",i,j);
              output(&data);
           }
        }
     }
     /**************************************************/

     cout << "\n\nConversion finished.\n\n";
     fclose(infile);
     fclose(outfile);
}



        // Reads a 2 byte number from the input file
        // and swaps the bytes
void load_short(union convert2 *value) {
     fread(&value->i,2,1,infile);
     swab(&value->c[0],&value->c[0],2);
}


         // Reads a 4 byte number from the input file
         // Swaps the bytes
void load(union convert *value) {
     int hold;

     fread(&value->l,4,1,infile);
     swab(&value->c[0],&value->c[0],4);
     hold=value->i[0];
     value->i[0]=value->i[1];
     value->i[1]=hold;
}


      // Writes data to the output (window.dat) file
void output(union convert2 *value) {
     unsigned int out;

     value->i=value->i >> 5;
     value->i+=2048;
     if (value->i<0) value->i=0;
     out=(unsigned) value->i;
     if (fwrite((char *)&out,2,1,outfile)!=1) {
        cout << "Error writing to output file.\nProgram aborted.\n\n";
        exit(1);
     }
}
