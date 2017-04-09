	#include <stdint.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <fcntl.h>
	#include <linux/types.h>
	#include <linux/spi/spidev.h>
	#include <time.h>
	#include <assert.h>


	#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
	#define MAX_SAMPLES 65536

	volatile int rate[10];                    // array to hold last ten IBI values
	volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
	volatile unsigned long lastBeatTime = 0;           // used to find IBI
	volatile int P =512;                      // used to find peak in pulse wave, seeded
	volatile int T = 512;                     // used to find trough in pulse wave, seeded
	volatile int thresh = 530;                // used to find instant moment of heart beat, seeded
	volatile int amp = 0;                   // used to hold amplitude of pulse waveform, seeded
	volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
	volatile int Signal;                // holds the incoming raw data

	volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
	volatile boolean secondBeat = false;      // used to seed rate array so we startup with reasonable BPM

	int IBI = 600;
	volatile boolean Pulse = false;          # "True" when User's live heartbeat is detected. "False" when not a "live beat". 
    lastTime = int(time.time()*1000);
		
	void HeartRate::run()
	{
		
		running = true;
		
		fprintf(stderr,"We are running!\n");
		
		
		while(running){
			  // set up ringbuffer
			  samples = new int[MAX_SAMPLES];
			  // pointer for incoming data
			  pIn = samples;
			  // pointer for outgoing data
			  pOut = samples;


			 Signal = ADCSetup.adcRead(pulsePin);    //#TODO Fix the adcRead          // read the Pulse Sensor
			 currentTime = int(time.time()*1000);
			 sampleCounter += currentTime - lastTime;
			 lastTime = currentTime;                         // keep track of the time in mS with this variable
			 int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

			 //  find the peak and trough of the pulse wave
			 if(Signal < thresh && N > (IBI/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
			 if (Signal < T){                        // T is the trough
						 T = Signal;             // keep track of lowest point in pulse wave
					}
			 }

			 if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
				P = Signal;                             // P is the peak
			 }                                        // keep track of highest point in pulse wave

			 //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
			 // signal surges up in value every time there is a pulse
			 if (N > 250){                                   // avoid high frequency noise
				if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){
				  Pulse = true;                               // set the Pulse flag when we think there is a pulse
				  digitalWrite(blinkPin,HIGH);                // turn on pin 13 LED
				  IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
				  lastBeatTime = sampleCounter;               // keep track of time for next pulse

				  if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
					secondBeat = false;                  // clear secondBeat flag
					for(int i=0; i<=9; i++){             // seed the running total to get a realisitic BPM at startup
					  rate[i] = IBI;
					}
				  }

				  if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
					firstBeat = false;                   // clear firstBeat flag
					secondBeat = true;                   // set the second beat flag
					sei();                               // enable interrupts again
					return;                              // IBI value is unreliable so discard it
				  }


				  // keep a running total of the last 10 IBI values
				  word runningTotal = 0;                  // clear the runningTotal variable

				  for(int i=0; i<=8; i++){                // shift data in the rate array
					rate[i] = rate[i+1];                  // and drop the oldest IBI value
					runningTotal += rate[i];              // add up the 9 oldest IBI values
				  }

				  rate[9] = IBI;                          // add the latest IBI to the rate array
				  runningTotal += rate[9];                // add the latest IBI to runningTotal
				  runningTotal /= 10;                     // average the last 10 IBI values
				  BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
				  *pIn = BPM;
					if (pIn == (&samples[MAX_SAMPLES-1])) 
					  pIn = samples;
					else
					  pIn++;
				}
			}

			 if (Signal < thresh && Pulse == true){   // when the values are going down, the beat is over
				digitalWrite(blinkPin,LOW);            // turn off pin 13 LED
				Pulse = false;                         // reset the Pulse flag so we can do it again
				amp = P - T;                           // get amplitude of the pulse wave
				thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
				P = thresh;                            // reset these for next time
				T = thresh;
			 }

			 if (N > 2500){                           // if 2.5 seconds go by without a beat
				thresh = 530;                          // set thresh default
				P = 512;                               // set P default
				T = 512;                               // set T default
				lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
				firstBeat = true;                      // set these to avoid noise
				secondBeat = false;                    // when we get the heartbeat back
				BPM = 0;
			 }
		}// end isr
	}

	int HeartRate::getSample()
	{
	  assert(pOut!=pIn);
	  int value = *pOut;
	  if (pOut == (&samples[MAX_SAMPLES-1])) 
		pOut = samples;
	  else
		pOut++;
	  return value;
	}


	int HeartRate::hasSample()
	{
	  return (pOut!=pIn);
	}
	  

	void HeartRate::quit()
	{
		running = false;
		exit(0);
	}

	void HeartRate::pabort(const char *s)
	{
			perror(s);
			abort();
	}