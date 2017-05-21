/****************************************************************************
 Module
   Comm_Service.c

 Description
   This state machine was created by the communications committee for all the hovercrafts 
	 to implement to facilitate bug-free interoperability. It handles the decision making structure 
	 for incoming data packets and actuates the hovercraft's peripherals accordingly.

 History
 When           Who     What/Why
 -------------- ---     --------
 05/14/2017			MCH
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "Hardware.h"
#include "Constants.h"
#include "Transmit_SM.h"
#include "FARMER_SM.h"


/*----------------------------- Module Defines ----------------------------*/



/*---------------------------- Module Functions ---------------------------*/
static void ConstructPacket(uint8_t PacketType);
static void InterpretPacket(uint8_t SizeOfData); 
void ResetEncryptionIndex(void);
uint8_t* GetIMUData(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;

static uint8_t* DataPacket_Rx;
static uint8_t DataPacket_Tx[42];
static uint8_t EncryptionIndex;
static uint8_t IMU_Data[12];
static uint8_t DestMSB;
static uint8_t DestLSB;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitComm_Service

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     Mihika Hemmady
****************************************************************************/
bool InitComm_Service( uint8_t Priority )
{

  MyPriority = Priority;

  return true;
}

/****************************************************************************
 Function
    PostComm_Service

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Mihika Hemmady
****************************************************************************/
bool PostComm_Service( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunComm_Service

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   Mihika Hemmady
****************************************************************************/
ES_Event RunComm_Service( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

	if (ThisEvent.EventType == ES_DATAPACKET_RECEIVED ) {
			// call InterpretPacket
			uint8_t PacketLength = ThisEvent.EventParam;
			InterpretPacket(PacketLength);
	} 
	
	if (ThisEvent.EventType == ES_SENDPACKET ) {
			printf("boutta send some shiiiittt \r\n");
			// call ConstructPacket
			uint8_t PacketType = ThisEvent.EventParam;
			ConstructPacket(PacketType);

	}

  return ReturnEvent;
}

/****************************************************************************
 Function
    ConstructPacket

 Parameters
    DestMSB, DestLSB, SizeOfData (length of data array to send) 

 Returns
   none

 Author
   Sarah Cabreros
****************************************************************************/
static void ConstructPacket(uint8_t PacketType) {
		//printf("--------------CONSTRUCTING-----------\n\r");
					ES_Event NewEvent;
					
					// initialize RunningSum variable
					uint8_t RunningSum = 0;
					uint8_t CheckSum;
	
					// initialize pointer to data
					uint8_t* DataToSend;
					
					//Header Construction
					DataPacket_Tx[START_BYTE_INDEX] = START_DELIMITER; // don't add to RunningSum
					DataPacket_Tx[LENGTH_MSB_BYTE_INDEX] = 0x00; // don't add to RunningSum
		
					DataPacket_Tx[API_IDENT_BYTE_INDEX_TX] = API_IDENTIFIER_Tx;
					RunningSum += API_IDENTIFIER_Tx;
					DataPacket_Tx[FRAME_ID_BYTE_INDEX] = FRAME_ID;
					RunningSum += FRAME_ID;
					DataPacket_Tx[OPTIONS_BYTE_INDEX_TX] = OPTIONS;
					RunningSum += OPTIONS;
	
					switch (PacketType) {
						case FARMER_DOG_REQ_2_PAIR:
							printf("Req 2 Pair Construction (Comm Service) \n\r");
						
							// broadcast to everyone
							DestMSB = 0xFF;
							DestLSB = 0xFF;
							DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = DestMSB; 
							RunningSum += DestMSB;
							DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = DestLSB; 
							RunningSum += DestLSB;
						
						
							//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = REQ_2_PAIR_LENGTH; // don't add to RunningSum
						
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = FARMER_DOG_REQ_2_PAIR;
							RunningSum += FARMER_DOG_REQ_2_PAIR;
						
							//get data
							uint8_t DogTag = GetDogTag();
						
							//add data 
							DataPacket_Tx[DATA_BYTE_INDEX_TX] = DogTag;
							RunningSum += DogTag;
						
							// calculate and add check sum
							CheckSum = 0xFF - RunningSum;
							DataPacket_Tx[DATA_BYTE_INDEX_TX+1] = CheckSum;
						
						  //set the frame length as event param
							NewEvent.EventParam = REQ_2_PAIR_LENGTH;						
							break;
							
						case FARMER_DOG_ENCR_KEY:
							printf("Encryption Key Construction (Comm Service) \n\r");
						
							// add destination address
							DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = DestMSB; 
							RunningSum += DestMSB;
							DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = DestLSB; 
							RunningSum += DestLSB;
						
							//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = ENCR_KEY_LENGTH; // don't add to RunningSum
						
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = FARMER_DOG_ENCR_KEY;
							RunningSum += FARMER_DOG_ENCR_KEY;
							
							// get data (encryption key)
							DataToSend = GetEncryptionKey();
						
							// add data
							for (int i = 0; i < 32; i++) {
								DataPacket_Tx[DATA_BYTE_INDEX_TX+i] = *(DataToSend + i);
								RunningSum += *(DataToSend + i);
							}
						
							// add check sum
							CheckSum = 0xFF - RunningSum;
							DataPacket_Tx[DATA_BYTE_INDEX_TX + 32] = CheckSum;
						
						  //set the frame length as event param
							NewEvent.EventParam = ENCR_KEY_LENGTH;	
							break;
							
						case FARMER_DOG_CTRL:
							printf("Farmer Control Construction \n\r");
						
							// add destination address
							DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = DestMSB; 
							RunningSum += DestMSB;
							DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = DestLSB; 
							RunningSum += DestLSB;				
						
							//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = CTRL_LENGTH; // don't add to RunningSum
							
							// get encryption key
							uint8_t* EncryptionData = GetEncryptionKey();
							//uint8_t EncryptionKey = *(EncryptionData + EncryptionIndex);
						
							//encrypt + add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = FARMER_DOG_CTRL^(*(EncryptionData + EncryptionIndex));
							RunningSum += FARMER_DOG_CTRL^(*(EncryptionData + EncryptionIndex));
							// increment encryption index
							EncryptionIndex++;
							if (EncryptionIndex > 31) EncryptionIndex = 0;
						
							// get data (from sensors)
							DataToSend = GetSensorData(); 
						
							// encrypt data THEN add data
							for (int i = 0; i < 3; i++) {
								uint8_t CurrentDataByte = *(DataToSend + i);
								printf("Sending byte: %i\r\n", CurrentDataByte);
								DataPacket_Tx[DATA_BYTE_INDEX_TX+i] = CurrentDataByte^(*(EncryptionData + EncryptionIndex));
								RunningSum += CurrentDataByte^(*(EncryptionData + EncryptionIndex));
								// increment encryption index
								EncryptionIndex++;
								if (EncryptionIndex > 31) EncryptionIndex = 0;
							}
						
							// add check sum
							CheckSum = 0xFF - RunningSum;
							DataPacket_Tx[DATA_BYTE_INDEX_TX + 3] = CheckSum; 
						
						  //set the frame length as event param
							NewEvent.EventParam = CTRL_LENGTH;	
							break;
					}
		
					NewEvent.EventType = ES_START_XMIT;
					//Post NewEvent to transmit service
					PostTransmit_SM(NewEvent);
}

/****************************************************************************
 Function
    InterpretPacket

 Parameters
    SizeOfData (length of data frame received [API ID -> all data]) 

 Returns
   none

 Author
   Sarah Cabreros
****************************************************************************/
static void InterpretPacket(uint8_t SizeOfData) {
	DataPacket_Rx	= GetDataPacket();
	uint8_t API_Ident = *(DataPacket_Rx + API_IDENT_BYTE_INDEX_RX);
	if (API_Ident == API_IDENTIFIER_Rx) {
			printf("RECEIVED A DATAPACKET (Comm_Service) \n\r");
			ES_Event NewEvent;
			uint8_t PacketType = *(DataPacket_Rx + PACKET_TYPE_BYTE_INDEX_RX);
			switch (PacketType) {
				case DOG_FARMER_REPORT :
					NewEvent.EventType = ES_DOG_REPORT_RECEIVED;
					// get IMU data
					for (int i = 0; i < 12; i++ ) {
						IMU_Data[i] = *(DataPacket_Rx + PACKET_TYPE_BYTE_INDEX_RX + i + 1); // IMU data starts AFTER packet type byte
						//printf("IMU data: %i\r\n", IMU_Data[i]);
					}
					break;
				case DOG_ACK :
					NewEvent.EventType = ES_DOG_ACK_RECEIVED;
					
					// save source address to destination address)
					DestMSB = *(DataPacket_Rx + SOURCE_ADDRESS_MSB_INDEX);
					DestLSB = *(DataPacket_Rx + SOURCE_ADDRESS_LSB_INDEX);
				
					break;
				case DOG_FARMER_RESET_ENCR :
					NewEvent.EventType = ES_DOG_RESET_ENCR_RECEIVED;
					break;
			}
			NewEvent.EventParam = SizeOfData; //the frame length
			PostFARMER_SM(NewEvent);
		} else if (API_Ident == API_IDENTIFIER_Tx_Result) { 
			printf("RECEIVED A TRANSMISSION RESULT DATAPACKET (Comm_Service): ");
			uint8_t TxStatusResult = *(DataPacket_Rx + TX_STATUS_BYTE_INDEX);
			if (TxStatusResult == SUCCESS) {
				printf("SUCCESS\n\r");
			} else {
				printf("FAILURE\n\r");
				//RESEND THE TX DATA PACKET -- ADD CODE IN HERE
			}
		} else if (API_Ident == API_IDENTIFIER_Reset) {
			printf("Hardware Reset Status Message \n\r");
		}
}


/******GETTER FUNCTIONS************/
uint8_t* GetDataPacket_Tx (void) {
  return &DataPacket_Tx[0];
}

uint8_t* GetIMUData (void) {
	return &IMU_Data[0];
}

void ResetEncryptionIndex(void) {
	EncryptionIndex = 0;
}
