/* SunPTV-USB   (c) 2016 trinity19683
  util functions for recfsusb2n (Linux OSes)
  utils.h
  2016-02-04
*/
#pragma once

#include <time.h>

/* options, parameters */
struct Args {
	unsigned int flags;
	unsigned int freq;
	unsigned int recsec;
	unsigned int splitter;
	unsigned int waitcnt;
	char sid_list[32];
	char* devfile;
	char* destfile;
};


#define u_gettime(tp)  clock_gettime(CLOCK_MONOTONIC, tp)

struct OutputBuffer {
	void *buffer;
	struct OutputBuffer* pOutput;
	int bufSize;
	int length;
	int (* process)(struct OutputBuffer* const, void* const buf);
	int (* release)(struct OutputBuffer* const);
};

#ifdef __cplusplus
extern "C" {
#endif
void u_difftime(struct timespec *, struct timespec *, int *, int *);
void parseOption(int argc, char * const argv[], struct Args *);
void setSignalHandler(const int mode, void (*func_handler)(int));
int OutputBuffer_release(struct OutputBuffer* const);
int OutputBuffer_put(struct OutputBuffer* const, void *buf, unsigned length);
int OutputBuffer_flush(struct OutputBuffer* const);
struct OutputBuffer* create_FileBufferedWriter(unsigned  bufSize, const char* const);
struct OutputBuffer* create_TSParser(unsigned  bufSize, struct OutputBuffer* const  pOutput, const unsigned  mode, void *usbDev);
int set_ch_table(void);
#ifdef __cplusplus
}
#endif


/*EOF*/
