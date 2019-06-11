/*
 * / _____)             _              | |
 * ( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 * _____) ) ____| | | || |_| ____( (___| | | |
 * (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *  (C)2015 Semtech
 *
 * Description: End device commissioning parameters
 *
 * License: Revised BSD License, see LICENSE.TXT file include in the project
 *
 * Maintainer: Miguel Luis and Gregory Cristian
 */
#ifndef __LORA_COMMISSIONING_H__
#define __LORA_COMMISSIONING_H__

/*!
 * When set to 1 the application uses the Over-the-Air activation procedure
 * When set to 0 the application uses the Personalization activation procedure
 */
#define OVER_THE_AIR_ACTIVATION 1

/*!
 * Indicates if the end-device is to be connected to a private or public network
 */
#define LORAWAN_PUBLIC_NETWORK true

/*!
 * IEEE Organizationally Unique Identifier ( OUI ) (big endian)
 * \remark This is unique to a company or organization
 */
#define IEEE_OUI 0x11, 0x22, 0x23

/*!
 * Mote device IEEE EUI (big endian)
 */
#define LORAWAN_DEVICE_EUI { 0x0E, 0xA0, 0xE1, 0x0D, 0xD0, 0x4A, 0x0A, 0x80 }

/*!
 * Application IEEE EUI (big endian)
 */
#define LORAWAN_APPLICATION_EUI { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x01, 0xD5, 0x5E }

/*!
 * AES encryption/decryption cipher application key
 */
#define LORAWAN_APPLICATION_KEY { 0x05, 0x51, 0xFB, 0xB8, 0xE8, 0x8F, 0x61, 0xE0, 0x51, 0xC3, 0xB5, 0xD4, 0x2D, 0xE3, 0xB1, 0x48 }

/*!
 * Current network ID
 */
#define LORAWAN_NETWORK_ID (uint32_t) 0

/*!
 * Device address on the network (big endian)
 */
#define LORAWAN_DEVICE_ADDRESS (uint32_t) 0x0067a6f4

/*!
 * AES encryption/decryption cipher network session key
 */
#define LORAWAN_NWKSKEY { 0x14, 0x93, 0xD6, 0x4A, 0x48, 0xE7, 0X9F, 0x05, 0x5C, 0x31, 0xB5, 0xC7, 0xB3, 0x60, 0x3E, 0x74 }

/*!
 * AES encryption/decryption cipher application session key
 */
#define LORAWAN_APPSKEY { 0x31, 0xE2, 0xE1, 0x1A, 0x65, 0x1E, 0xAC, 0x91, 0xEE, 0x1A, 0x1F, 0xE2, 0x3A, 0x4E, 0xE1, 0xB1 }
#endif	// __LORA_COMMISSIONING_H__