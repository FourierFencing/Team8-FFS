#ifndef IIRTHREAD //other code doesn't need _H
#define IIRTHREAD

#include <QThread> //needed for multithreading
#include <stdint.h> //because we're using uint8_t

//The class is started in main program as a thread
// adcreader -> start();
//which is then running until the function
// adcreader -> quit(); is called

//then the apparent standard method is: 
// adcreader->wait();
// so that the thread can terminate without breaking

class iirThread : public QThread
{

public: 
  iirThread(); //sets-up stuff
	
	//ring buffer functiony stuff
  int hasIIRSample();
	int getIIRSample();
	// thread stuff
	void quit();
	void run();
	
protected: 
		//nothing.. it's useless right?
private: 
	bool running;
	void pabort(const char *s); //don't think we use this.
	//don't currently have any private functions
	//probably include fft stuff here
	
	//data collected
	int *samplesIIR; //actually is used to store each data value
	//this is our ring buffer

	// pointer to ringbuffer
	int *pInIIR; 
	//pIn is the address of the current input index of the ring buffer
	//*pIn is the value at that address

	int *pOutIIR;
	//pOut is the address of the current output index of the ring buffer
	//*pOut is the value at that address (output value) 


	//if pIn = pOut, then our current output index has caught up to our input index
	// this is why we use hasSample() as a condition
	
	//set up filter constants
	const float samplingrate = 100000; // Sample rate in Hz
  const float centre_frequency_L = 10500; //The centre frequency of the fencer's lame
  const float centre_frequency_W = 13500; //The centre frequency of the fencer's weapon guard
  const float frequency_width = 1000;  //Width of both frequencies
	
	
};

#endif
	