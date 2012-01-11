#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include <stdio.h>

#define FIRMWAREVERSION 12 // 1.1  , version number needs to fit in byte (0~255) to be able to store it into config
//#define FACTORY_SELFTEST
//#define DEBUG 

//return values 0: ok, -1: error, 1: nothing , 2: modified
enum {
	OK = 0,
	ERR = -1,
	NOTHING = 1,
	MODIFIED = 2
};

#endif
