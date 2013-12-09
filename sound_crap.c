void *SoundThread(void *v)
{
        int dsp;
        int iCur;
        int iCur2;
        int iCharIn;
        unsigned int SampleRate;
        int arg;
        char quit=0;
        int i,j;


        dsp = open("/dev/dsp",O_WRONLY);

        if(dsp <0)
                fprintf(stderr,"Failed to open the sound card.\n");

        // EXPERIMENTAL, shrink the buffer to improve response time!
        arg = 0x00800004;
        ioctl(dsp,SNDCTL_DSP_SETFRAGMENT,&arg);        // 128 16byte chunks. about 1/8 sec

        if(!SetStereo(dsp,1))
//                return((void *) -1);
                printf("no stereo, might as well quit\n");

        SampleRate = SetSampleRate(dsp,SAMPLE_RATE);

        if(!SampleRate) {
                fprintf(stderr,"ERROR:\aCannot set a sample rate\n");
                SampleRate=8000;
//                return(0);
        }

        InitWaveTable(SampleRate);

        curtime=curtime2=0;
        
        while(!bQuit) {
                for(j=0;j<SAMPLE_RATE;j++) {

                        IncrementCurtimes(harmonic_curtimeL, nHarmonics, increment, 0.0);
                        IncrementCurtimes(harmonic_curtimeR, nHarmonics, increment, detune);

                        iCur = ComputeSummation(harmonic_curtimeL, nHarmonics, volume);
                        iCur2 = ComputeSummation(harmonic_curtimeR, nHarmonics, volume);

                        write(dsp,&iCur,1);        // left
                        write(dsp,&iCur2,1);        // right!

                        count++;        // bump the sample counter!
                }

                seconds++;

        }        // end while                

return;
}

int SetStereo(int dsp,char Stereo)
{
int stereo = Stereo; /* 0=mono, 1=stereo */

        if (ioctl(dsp, SNDCTL_DSP_STEREO, &stereo)==-1) { /* Fatal error */
                perror("SNDCTL_DSP_STEREO");
                return(0);
        }

        if (stereo != 1) {
                printf("\aNo sterio support\n");
                printf("Stereo support is REQUIRED for this to work!\n");
                return(0);
                // The device doesn't support stereo mode.
        }

        return(1);
}

unsigned int SetSampleRate(int audio_fd, unsigned int rate)
{
unsigned int SampleRate = rate;
        
        if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &SampleRate)==-1) { /* Fatal error */
                perror("SNDCTL_DSP_SPEED");
                return(FALSE);
        }

        return(SampleRate);        // this is the actual sample rate which SHOULD be near the input value!
} 

int InitWaveTable(unsigned int SampleRate)
{
unsigned int i;
double increment = (2*M_PI) / SampleRate;
double Current=0;

        WaveTable = (int *) calloc(SampleRate*2,sizeof(int));

        for(i=0;i<SampleRate;i++,Current += increment) {
                WaveTable[i]= (int) floor( sin(Current) * 127);
        }

        return(1);
} 

void IncrementCurtimes(double timeval[], int harmonics, double increment, double detune)
{
        int i;

        for(i=0;i<harmonics; i++) {
                timeval[i] += increment * pow(2,i) + detune;
                timeval[i] = fmod(timeval[i],SAMPLE_RATE);
        }
}

int ComputeSummation(double timeval[], int harmonics, double Volume)
{
        int i;
        int sigma=0;

        for(i=0; i<harmonics; i++) {
                sigma += (int) (WaveTable[ (int) floor(timeval[i])] /(1<<i));
        }

        sigma /=2;

        sigma +=128;

        return( floor( (Volume*sigma)/100));
}
