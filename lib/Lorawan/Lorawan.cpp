#include "Lorawan.h"

static uint8_t DevEui[] = { 0x27, 0x77, 0x0B, 0x13, 0x50, 0x3E, 0xC7, 0xE1 };
static uint8_t AppKey[] = { 0x55, 0xE8, 0xED, 0x0D, 0xE8, 0xF8, 0x01, 0x12, 0x9A, 0x75, 0x54, 0x57, 0x4C, 0xB5, 0xA3, 0x82 };
static uint8_t AppEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static uint8_t AppPort	   = LORAWAN_APP_PORT;
static uint8_t AppDataSize = LORAWAN_APP_DATA_SIZE;
static uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];

static uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;

static enum eDeviceState
{
	DEVICE_STATE_INIT,
	DEVICE_STATE_JOIN,
	DEVICE_STATE_SEND,
	DEVICE_STATE_CYCLE,
	DEVICE_STATE_SLEEP
} DeviceState;

struct ComplianceTest_s
{
	bool	  Running;
	uint8_t	  State;
	bool	  IsTxConfirmed;
	uint8_t	  AppPort;
	uint8_t	  AppDataSize;
	uint8_t * AppDataBuffer;
	uint16_t  DownLinkCounter;
	bool	  LinkCheck;
	uint8_t	  DemodMargin;
	uint8_t	  NbGateways;
} ComplianceTest;

static bool IsNetworkJoinedStatusUpdate = false;

struct sLoRaMacUplinkStatus
{
	uint8_t	  Acked;
	int8_t	  Datarate;
	uint16_t  UplinkCounter;
	uint8_t	  Port;
	uint8_t * Buffer;
	uint8_t	  BufferSize;
} LoRaMacUplinkStatus;

volatile bool UplinkStatusUpdated = false;

struct sLoRaMacDownlinkStatus
{
	int16_t	  Rssi;
	int8_t	  Snr;
	uint16_t  DownlinkCounter;
	bool	  RxData;
	uint8_t	  Port;
	uint8_t * Buffer;
	uint8_t	  BufferSize;
} LoRaMacDownlinkStatus;

volatile bool DownlinkStatusUpdated = false;

static void PrepareTxFrame (uint8_t port)
{
	int temp, humi;

	AppData[0] = 1;

	if (IsTxConfirmed == true)
	{
		sensors.getSensorsIntValue (&temp, &humi);

		// Temperature roof
		AppData[1] = (temp >> 8);
		AppData[2] = (uint8_t) temp;

		// Humidity roof
		AppData[3] = 2;
		AppData[4] = (uint8_t) humi;
	}
}	// PrepareTxFrame

static bool SendFrame (void)
{
	McpsReq_t mcpsReq;
	LoRaMacTxInfo_t txInfo;

	if (LoRaMacQueryTxPossible (AppDataSize, &txInfo) != LORAMAC_STATUS_OK)
	{
		// Send empty frame in order to flush MAC commands
		mcpsReq.Type						= MCPS_UNCONFIRMED;
		mcpsReq.Req.Unconfirmed.fBuffer		= NULL;
		mcpsReq.Req.Unconfirmed.fBufferSize = 0;
		mcpsReq.Req.Unconfirmed.Datarate	= LORAWAN_DEFAULT_DATARATE;

		LoRaMacUplinkStatus.Acked	   = false;
		LoRaMacUplinkStatus.Port	   = 0;
		LoRaMacUplinkStatus.Buffer	   = NULL;
		LoRaMacUplinkStatus.BufferSize = 0;
	}
	else
	{
		LoRaMacUplinkStatus.Acked	   = false;
		LoRaMacUplinkStatus.Port	   = AppPort;
		LoRaMacUplinkStatus.Buffer	   = AppData;
		LoRaMacUplinkStatus.BufferSize = AppDataSize;

		if (IsTxConfirmed == false)
		{
			mcpsReq.Type						= MCPS_UNCONFIRMED;
			mcpsReq.Req.Unconfirmed.fPort		= AppPort;
			mcpsReq.Req.Unconfirmed.fBuffer		= AppData;
			mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
			mcpsReq.Req.Unconfirmed.Datarate	= LORAWAN_DEFAULT_DATARATE;
		}
		else
		{
			mcpsReq.Type					  = MCPS_CONFIRMED;
			mcpsReq.Req.Confirmed.fPort		  = AppPort;
			mcpsReq.Req.Confirmed.fBuffer	  = AppData;
			mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
			mcpsReq.Req.Confirmed.NbTrials	  = 8;
			mcpsReq.Req.Confirmed.Datarate	  = LORAWAN_DEFAULT_DATARATE;
		}
	}

	if (LoRaMacMcpsRequest (&mcpsReq) == LORAMAC_STATUS_OK)
	{
		return false;
	}
	return true;
}	// SendFrame

static void McpsConfirm (McpsConfirm_t * mcpsConfirm)
{
	if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
	{
		switch (mcpsConfirm->McpsRequest)
		{
			case MCPS_UNCONFIRMED: {
				// Check Datarate
				// Check TxPower
				break;
			}
			case MCPS_CONFIRMED: {
				// Check Datarate
				// Check TxPower
				// Check AckReceived
				// Check NbTrials
				LoRaMacUplinkStatus.Acked = mcpsConfirm->AckReceived;
				break;
			}
			case MCPS_PROPRIETARY: {
				break;
			}
			default:
				break;
		}
		LoRaMacUplinkStatus.Datarate	  = mcpsConfirm->Datarate;
		LoRaMacUplinkStatus.UplinkCounter = mcpsConfirm->UpLinkCounter;

		UplinkStatusUpdated = true;
	}
}	// McpsConfirm

static void McpsIndication (McpsIndication_t * mcpsIndication)
{
	if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK)
	{
		return;
	}

	switch (mcpsIndication->McpsIndication)
	{
		case MCPS_UNCONFIRMED: {
			break;
		}
		case MCPS_CONFIRMED: {
			break;
		}
		case MCPS_PROPRIETARY: {
			break;
		}
		case MCPS_MULTICAST: {
			break;
		}
		default:
			break;
	}

	// Check Multicast
	// Check Port
	// Check Datarate
	// Check FramePending
	// Check Buffer
	// Check BufferSize
	// Check Rssi
	// Check Snr
	// Check RxSlot
	LoRaMacDownlinkStatus.Rssi = mcpsIndication->Rssi;
	if (mcpsIndication->Snr & 0x80)	// The SNR sign bit is 1
	{
		// Invert and divide by 4
		LoRaMacDownlinkStatus.Snr = ( ( ~mcpsIndication->Snr + 1 ) & 0xFF ) >> 2;
		LoRaMacDownlinkStatus.Snr = -LoRaMacDownlinkStatus.Snr;
	}
	else
	{
		// Divide by 4
		LoRaMacDownlinkStatus.Snr = ( mcpsIndication->Snr & 0xFF ) >> 2;
	}
	LoRaMacDownlinkStatus.DownlinkCounter++;
	LoRaMacDownlinkStatus.RxData	 = mcpsIndication->RxData;
	LoRaMacDownlinkStatus.Port		 = mcpsIndication->Port;
	LoRaMacDownlinkStatus.Buffer	 = mcpsIndication->Buffer;
	LoRaMacDownlinkStatus.BufferSize = mcpsIndication->BufferSize;

	if (ComplianceTest.Running == true)
	{
		ComplianceTest.DownLinkCounter++;
	}

	if (mcpsIndication->RxData == true)
	{
		switch (mcpsIndication->Port)
		{
			case 224:
				if (ComplianceTest.Running == false)
				{
					// Check compliance test enable command (i)
					if ( ( mcpsIndication->BufferSize == 4 ) &&
					  ( mcpsIndication->Buffer[0] == 0x01 ) &&
					  ( mcpsIndication->Buffer[1] == 0x01 ) &&
					  ( mcpsIndication->Buffer[2] == 0x01 ) &&
					  ( mcpsIndication->Buffer[3] == 0x01 ) )
					{
						IsTxConfirmed				   = false;
						AppPort						   = 224;
						AppDataSize					   = 2;
						ComplianceTest.DownLinkCounter = 0;
						ComplianceTest.LinkCheck	   = false;
						ComplianceTest.DemodMargin	   = 0;
						ComplianceTest.NbGateways	   = 0;
						ComplianceTest.Running		   = true;
						ComplianceTest.State		   = 1;

						MibRequestConfirm_t mibReq;
						mibReq.Type			   = MIB_ADR;
						mibReq.Param.AdrEnable = true;
						LoRaMacMibSetRequestConfirm (&mibReq);

						LoRaMacTestSetDutyCycleOn (false);
					}
				}
				else
				{
					ComplianceTest.State = mcpsIndication->Buffer[0];
					switch (ComplianceTest.State)
					{
						case 0:	// Check compliance test disable command (ii)
							IsTxConfirmed				   = LORAWAN_CONFIRMED_MSG_ON;
							AppPort						   = LORAWAN_APP_PORT;
							AppDataSize					   = LORAWAN_APP_DATA_SIZE;
							ComplianceTest.DownLinkCounter = 0;
							ComplianceTest.Running		   = false;

							MibRequestConfirm_t mibReq;
							mibReq.Type			   = MIB_ADR;
							mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
							LoRaMacMibSetRequestConfirm (&mibReq);
							LoRaMacTestSetDutyCycleOn (LORAWAN_DUTYCYCLE_ON);
							break;
						case 1:	// (iii, iv)
							AppDataSize = 2;
							break;
						case 2:	// Enable confirmed messages (v)
							IsTxConfirmed		 = true;
							ComplianceTest.State = 1;
							break;
						case 3:	// Disable confirmed messages (vi)
							IsTxConfirmed		 = false;
							ComplianceTest.State = 1;
							break;
						case 4:	// (vii)
							AppDataSize = mcpsIndication->BufferSize;

							AppData[0] = 4;
							for (uint8_t i = 1; i < AppDataSize; i++)
							{
								AppData[i] = mcpsIndication->Buffer[i] + 1;
							}
							break;
						case 5:	// (viii)
						{
							MlmeReq_t mlmeReq;
							mlmeReq.Type = MLME_LINK_CHECK;
							LoRaMacMlmeRequest (&mlmeReq);
						}
						break;
						case 6:	// (ix)
						{
							MlmeReq_t mlmeReq;

							// Disable TestMode and revert back to normal operation
							IsTxConfirmed				   = LORAWAN_CONFIRMED_MSG_ON;
							AppPort						   = LORAWAN_APP_PORT;
							AppDataSize					   = LORAWAN_APP_DATA_SIZE;
							ComplianceTest.DownLinkCounter = 0;
							ComplianceTest.Running		   = false;

							MibRequestConfirm_t mibReq;
							mibReq.Type			   = MIB_ADR;
							mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
							LoRaMacMibSetRequestConfirm (&mibReq);
							LoRaMacTestSetDutyCycleOn (LORAWAN_DUTYCYCLE_ON);

							mlmeReq.Type = MLME_JOIN;

							mlmeReq.Req.Join.DevEui	  = DevEui;
							mlmeReq.Req.Join.AppEui	  = AppEui;
							mlmeReq.Req.Join.AppKey	  = AppKey;
							mlmeReq.Req.Join.NbTrials = 3;

							LoRaMacMlmeRequest (&mlmeReq);
							DeviceState = DEVICE_STATE_SLEEP;
						}
						break;
						case 7:	// (x)
						{
							if (mcpsIndication->BufferSize == 3)
							{
								MlmeReq_t mlmeReq;
								mlmeReq.Type			 = MLME_TXCW;
								mlmeReq.Req.TxCw.Timeout = (uint16_t) ( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
								LoRaMacMlmeRequest (&mlmeReq);
							}
							else if (mcpsIndication->BufferSize == 7)
							{
								MlmeReq_t mlmeReq;
								mlmeReq.Type			   = MLME_TXCW_1;
								mlmeReq.Req.TxCw.Timeout   = (uint16_t) ( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
								mlmeReq.Req.TxCw.Frequency = (uint32_t) ( ( mcpsIndication->Buffer[3] << 16 ) | ( mcpsIndication->Buffer[4] << 8 ) | mcpsIndication->Buffer[5] ) * 100;
								mlmeReq.Req.TxCw.Power	   = mcpsIndication->Buffer[6];
								LoRaMacMlmeRequest (&mlmeReq);
							}
							ComplianceTest.State = 1;
						}
						break;
						default:
							break;
					}
				}
				break;
			default:
				break;
		}
	}

	DownlinkStatusUpdated = true;
}	// McpsIndication

static void MlmeConfirm (MlmeConfirm_t * mlmeConfirm)
{
	switch (mlmeConfirm->MlmeRequest)
	{
		case MLME_JOIN:
			if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
			{
				// Status is OK, node has joined the network
				IsNetworkJoinedStatusUpdate = true;
				DeviceState					= DEVICE_STATE_SEND;
			}
			else
			{
				// Join was not successful. Try to join again
				DeviceState = DEVICE_STATE_JOIN;
			}
			break;

		case MLME_LINK_CHECK:
			if (mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
			{
				// Check DemodMargin
				// Check NbGateways
				if (ComplianceTest.Running == true)
				{
					ComplianceTest.LinkCheck   = true;
					ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
					ComplianceTest.NbGateways  = mlmeConfirm->NbGateways;
				}
			}
			break;

		default:
			break;
	}
	UplinkStatusUpdated = true;
}	// MlmeConfirm

void initLoraWan ()
{
	sensors.initSensors();

	BoardInit();

	DeviceState = DEVICE_STATE_INIT;
}

LoRaMacPrimitives_t LoRaMacPrimitives_;
LoRaMacCallback_t LoRaMacCallbacks_;
MibRequestConfirm_t mibReq_;

void loraWanAction ()
{
	if (IsNetworkJoinedStatusUpdate == true)
	{
		IsNetworkJoinedStatusUpdate = false;
		mibReq_.Type				= MIB_NETWORK_JOINED;
		LoRaMacMibGetRequestConfirm (&mibReq_);
	}

	if (UplinkStatusUpdated == true)
	{
		UplinkStatusUpdated = false;
	}

	if (DownlinkStatusUpdated == true)
	{
		DownlinkStatusUpdated = false;
	}

	switch (DeviceState)
	{
		case DEVICE_STATE_INIT:
			printf ("State : INIT\n");
			LoRaMacPrimitives_.MacMcpsConfirm	 = McpsConfirm;
			LoRaMacPrimitives_.MacMcpsIndication = McpsIndication;
			LoRaMacPrimitives_.MacMlmeConfirm	 = MlmeConfirm;
			LoRaMacCallbacks_.GetBatteryLevel	 = BoardGetBatteryLevel;
			LoRaMacInitialization (&LoRaMacPrimitives_, &LoRaMacCallbacks_);

			mibReq_.Type			= MIB_ADR;
			mibReq_.Param.AdrEnable = LORAWAN_ADR_ON;
			LoRaMacMibSetRequestConfirm (&mibReq_);

			mibReq_.Type					  = MIB_PUBLIC_NETWORK;
			mibReq_.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
			LoRaMacMibSetRequestConfirm (&mibReq_);

			LoRaMacTestSetDutyCycleOn (LORAWAN_DUTYCYCLE_ON);

			LoRaMacChannelAdd (3, (ChannelParams_t) LC4);
			LoRaMacChannelAdd (4, (ChannelParams_t) LC5);
			LoRaMacChannelAdd (5, (ChannelParams_t) LC6);
			LoRaMacChannelAdd (6, (ChannelParams_t) LC7);
			LoRaMacChannelAdd (7, (ChannelParams_t) LC8);
			LoRaMacChannelAdd (8, (ChannelParams_t) LC9);
			LoRaMacChannelAdd (9, (ChannelParams_t) LC10);

			mibReq_.Type					= MIB_RX2_DEFAULT_CHANNEL;
			mibReq_.Param.Rx2DefaultChannel = (Rx2ChannelParams_t){ 869525000, DR_3 };
			LoRaMacMibSetRequestConfirm (&mibReq_);

			mibReq_.Type			 = MIB_RX2_CHANNEL;
			mibReq_.Param.Rx2Channel = (Rx2ChannelParams_t){ 869525000, DR_3 };
			LoRaMacMibSetRequestConfirm (&mibReq_);

			LoRaMacDownlinkStatus.DownlinkCounter = 0;

			DeviceState = DEVICE_STATE_JOIN;
			break;

		case DEVICE_STATE_JOIN:
			printf ("State : JOIN\n");
			MlmeReq_t mlmeReq;

			mlmeReq.Type = MLME_JOIN;

			mlmeReq.Req.Join.DevEui	  = DevEui;
			mlmeReq.Req.Join.AppEui	  = AppEui;
			mlmeReq.Req.Join.AppKey	  = AppKey;
			mlmeReq.Req.Join.NbTrials = 100;

			LoRaMacMlmeRequest (&mlmeReq);

			DeviceState = DEVICE_STATE_SEND;

			IsNetworkJoinedStatusUpdate = true;
			break;

		case DEVICE_STATE_SEND:

			MibRequestConfirm_t mibReq;
			LoRaMacStatus_t status;

			mibReq.Type = MIB_NETWORK_JOINED;
			status		= LoRaMacMibGetRequestConfirm (&mibReq);

			if (status == LORAMAC_STATUS_OK)
			{
				if (mibReq.Param.IsNetworkJoined == true)
				{
					printf ("State : SEND\n");
					PrepareTxFrame (AppPort);

					SendFrame();

					DeviceState = DEVICE_STATE_SLEEP;
				}
			}
			else
			{
				DeviceState = DEVICE_STATE_JOIN;
			}
			break;

		default:
			DeviceState = DEVICE_STATE_INIT;
			break;
	}
}	// loraWanAction