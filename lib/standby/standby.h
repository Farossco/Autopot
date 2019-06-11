#ifndef _STANDBY_H
#define _STANDBY_H

#include "mbed.h"
// #include "stm32l073xx.h"

#define _CLEAR_BIT(a, b) (a &= ~b)
#define _SET_BIT(a, b)	 (a |= b)

#define BKP0 1
#define BKP1 2
#define BKP2 3
#define BKP3 4
#define BKP4 5

static uint32_t RTC_bkp_value[5];

// function prototypes
void standby_mode (const uint16_t time);					// time in second
uint32_t read_RTC_bkp_reg (uint32_t bkp_reg);				// read the BKPx register
void write_RTC_bkp_reg (uint32_t bkp_reg, uint32_t value);	// write the BKPx register
void reset_RTC_bkp_reg (void);								// reset all the BKPx registers
void refresh_RTC_bkp_reg (void);							// refresh all the BKPx registers so that they cant't lose their previous data
void standby_verification (void);							// verify if the board woke up from standby mode or power cycle

// other functions
void enable_RTC_reg_access (void);
void disable_RTC_reg_access (void);
int config_RTC (const uint16_t time);
void enable_bkp_access (void);
void config_Standby (void);
void enter_Standby (void);
void _write_RTC_bkp_reg (void);
void config_GPIO (void);

#endif // ifndef _STANDBY_H