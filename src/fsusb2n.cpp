/*
recfsusb2n for Fedora 12 Linux 2.6
http://tri.dw.land.to/fsusb2n/
2009-09-14 20:22
2011-04-06 23:50
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

#include "ktv.hpp"
#include "utils.h"

static bool caughtSignal = false;
void sighandler(int arg)
{
	caughtSignal = true;
}

int main(int argc, char **argv)
{
	int ret = 0;
	struct Args args = {0,0,0,0,20,"",NULL,NULL};
	parseOption(argc, argv, &args);

	// ログ出力先設定
	std::ostream& log = (args.destfile==NULL) ? std::cerr : std::cout;
	log << "recfsusb2n ver. 0.9.2" << std::endl << "ISDB-T DTV Tuner FSUSB2N" << std::endl;
	EM2874Device::setLog(&log);

	EM2874Device *usbDev = EM2874Device::AllocDevice(args.devfile);
	if(usbDev == NULL){
		if(args.devfile!=NULL){
			free(args.devfile);
		}
		return 1;
	}
	usbDev->initDevice2();

	KtvDevice *pDev;
	if(usbDev->getDeviceID() == 2) {
		pDev = new Ktv2Device(usbDev);
	}else{
		pDev = new Ktv1Device(usbDev);
	}

	pDev->InitTuner();
	pDev->SetFrequency(args.freq);
	pDev->InitDeMod();
	pDev->ResetDeMod();




	//# OutputBuffer
	struct OutputBuffer *pOutputBuffer;
	pOutputBuffer = create_FileBufferedWriter( 768 * 1024, args.destfile );
	if(! pOutputBuffer ) {
		log << "failed to init FileBufferedWriter." << std::endl;
		delete pDev;
		delete usbDev;
		ret = 70;
		return(ret);
	}
	struct OutputBuffer * const  pFileBufferedWriter = pOutputBuffer;
	unsigned b25_flag = (args.flags & 0x1000)? 1 : 0; // 1:b25 decode on  0:b25 decode off
	pOutputBuffer = create_TSParser( 8192, pFileBufferedWriter, b25_flag, (void *)usbDev );
printf("takosuke 2\n");
	if(! pOutputBuffer ) {
		log << "failed to init TS Parser." << std::endl;
		OutputBuffer_release(pFileBufferedWriter);
		delete pDev;
		delete usbDev;
		ret = 71;
		return(ret);
	}

	// チューニング完了・受信安定化待ち
	unsigned int timeout = 100;
	uint8_t     seq_state;
	unsigned int    hi_qua = 0, gt_qua;

	if(args.flags & 0x1){
		log << "Wait for the tuner to stabilize" << std::endl;
	}

	do {
		usleep(100000);
		if (--timeout <= 0) {
			log << "GetSequenceState timeout." << std::endl;
			exit(1);
		}


		seq_state = pDev->DeMod_GetSequenceState();
		gt_qua    = pDev->DeMod_GetQuality();
		if( seq_state>=8 && gt_qua> hi_qua )
			hi_qua = gt_qua;
		if(args.flags & 0x1){
			log << "Sequence = " << (unsigned)seq_state << ", Quality = " << 0.02*gt_qua << std::endl;
		}


	} while(seq_state < 9 && !caughtSignal);

	// 受信開始
	usbDev->startStream();

	uint8_t	*pBuffer = NULL;
	int rlen;

	// 受信安定化待ち
	unsigned int sanity = 0;
	timeout = 0;

	if(args.flags & 0x1){
		log << "Wait for the tuner to stabilize waitcnt :" << args.waitcnt << std::endl;
	}

	do{
		usleep(50000);
		gt_qua    = pDev->DeMod_GetQuality();
		if(args.flags & 0x1){
			log << "Sequence = " << sanity  << ", Quality = " << 0.02*gt_qua << "    " << 0.02*hi_qua << std::endl;
		}
		if( gt_qua> hi_qua ){
			if( hi_qua < gt_qua*8/10 ){
				hi_qua = gt_qua;
				rlen = usbDev->getStream((const void **)&pBuffer);
				sanity  = 0;
				continue;
			}else{
				hi_qua = gt_qua;
			}
		}else{
			if(gt_qua < hi_qua*8/10){
				rlen = usbDev->getStream((const void **)&pBuffer);
				sanity  = 0;
				continue;
			}
		}
		if(++sanity >= args.waitcnt){
			break;
		}
	}while( !caughtSignal && ++timeout<args.waitcnt*2 );

	//# change signal handler
	setSignalHandler(1, sighandler);

	pBuffer = NULL;

	// 録画時間の基準開始時間
	time_t time_start = time(NULL);

	// Main loop
	while(!caughtSignal){
		if(0 < args.recsec){
			if(time(NULL) >= time_start + args.recsec){
				break;
			}
		}

		if(pBuffer!=NULL){
			usleep(500000);
		}
		if((rlen = usbDev->getStream((const void **)&pBuffer)) > 0) {
			if( 0 < rlen && (ret = OutputBuffer_put(pOutputBuffer, pBuffer, rlen) ) < 0){
				log << "TS write failed" << std::endl;
			}else{
				if(args.flags & 0x1){
					log << "Sequence = " << (unsigned)pDev->DeMod_GetSequenceState() << ", Quality = " << 0.02*pDev->DeMod_GetQuality() << ", " << rlen << "bytes wrote." << std::endl;
				}
			}
		}else{
			continue;
		}
	}

	if (caughtSignal) {
		log << "interrupted." << std::endl;
	}

	//# restore signal handler
	setSignalHandler(0, sighandler);

	usbDev->stopStream();

	if( pOutputBuffer ) {
		OutputBuffer_flush(pOutputBuffer);
		OutputBuffer_release(pOutputBuffer);
	}

	// 録画時間の測定
	time_t time_end = time(NULL);

	log << "done." << std::endl;
	log << "Rec time: " << static_cast<unsigned>(time_end - time_start) << " sec." << std::endl;

	delete pDev;
	delete usbDev;
	return 0;
}

/* */

