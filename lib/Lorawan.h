#ifndef LORAWAN_H
#define LORAWAN_H

#include "mbed.h"
#include "Sensors.h"
#include "board.h"
#include "radio.h"
#include "LoRaMac.h"
#include "Commissioning.h"
#include "LoRaMacTest.h"

#define APP_TX_DUTYCYCLE		 5000
#define APP_TX_DUTYCYCLE_RND	 1000
#define LORAWAN_DEFAULT_DATARATE DR_0
#define LORAWAN_CONFIRMED_MSG_ON true
#define LORAWAN_ADR_ON			 1

#define LORAWAN_DUTYCYCLE_ON false

#define LC4	 { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5	 { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6	 { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7	 { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8	 { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9	 { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10 { 868300000, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#define LORAWAN_APP_PORT		  15
#define LORAWAN_APP_DATA_SIZE	  15
#define LORAWAN_APP_DATA_MAX_SIZE 64

static void PrepareTxFrame (uint8_t port);
static bool SendFrame (void);
static void McpsConfirm (McpsConfirm_t * mcpsConfirm);
static void McpsIndication (McpsIndication_t * mcpsIndication);
static void MlmeConfirm (MlmeConfirm_t * mlmeConfirm);
void initLoraWan ();
void loraWanAction ();

#endif	// ifndef LORAWAN_H