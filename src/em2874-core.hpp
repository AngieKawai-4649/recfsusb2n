// EM2874 core

#ifndef _EM2874CORE_HPP_
#define _EM2874CORE_HPP_

//#include <boost/thread.hpp>
#include <pthread.h>

#include <inttypes.h>
#include <iostream>
#include "IoThread.hpp"

#define EM28XX_REG_I2C_CLK		0x06
#define EM2874_REG_TS_ENABLE	0x5F
#define EM28XX_REG_GPIO		0x80

#define EM2874_EP_TS1		0x84

#define DEMOD_ADDR	0x20
#define EEPROM_ADDR	0xa0
#define TUNER_ADDR	0xc0

/* EM2874 TS Enable Register (0x5f) */
#define EM2874_TS1_CAPTURE_ENABLE 0x01
#define EM2874_TS1_FILTER_ENABLE  0x02
#define EM2874_TS1_NULL_DISCARD   0x04

class EM2874Device
{
public:
	EM2874Device ();
	~EM2874Device ();
	static void setLog(std::ostream *plog) {
		log = plog;
	};

	bool openDevice (const char *devfile);
	static EM2874Device* AllocDevice(const char *devfile);
	bool initDevice2 ();

	uint8_t readReg (const uint8_t idx);
	int readReg (const uint8_t idx, uint8_t *val);
	int writeReg (const uint8_t idx, const uint8_t val);
	bool readI2C (const uint8_t addr, const uint16_t size, uint8_t *data, const bool isStop);
	bool writeI2C (const uint8_t addr, const uint16_t size, uint8_t *data, const bool isStop);

	bool resetICC ();
	bool writeICC (const size_t size, const void *data);
	bool readICC (size_t *size, void *data);
	int waitICC();
	int getCardStatus();

	int getDeviceID();

	bool isCardReady;

#ifdef DB_CARDREADER
	bool cardDev;
#endif

	void startStream();
	void stopStream();
	int getStream(const void **ptr);
	int getFd();

	static void *thread_proc( void *args)
	{
		reinterpret_cast<EM2874Device*>(args)->ts_func->tsReceive();
		return (void*)NULL;
	}


private:
	bool resetICC_1 ();
	bool resetICC_2 ();

	static std::ostream *log;
	int fd;
	uint8_t cardPCB;

	bool isStream;
	TsIoThread *ts_func;
	pthread_t  ts_thread;
};

#endif

