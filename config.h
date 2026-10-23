    
#ifndef CONFIG_H
#define	CONFIG_H

#define _XTAL_FREQ 2000000

#ifdef _18F452
#include"config_452.h"
#endif
#ifdef _18F4520
#include"config_4520.h"
#endif
#ifdef _18F4620
#include"config_4620.h"
#endif
#ifdef _18F4550
#include"config_4550.h"
#endif
#if defined(_16F877A) || defined(__16F877A__)
#include "config_877A.h"
#endif
#ifdef _16F777
#include"config_777.h"
#endif
#ifdef _18F45K50
#include"config_45k50.h"
#endif
#ifdef _18F47K40
#include"config_47k40.h"
#endif

#endif	/* CONFIG_H */
