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
static void ConstructPacket(uint8_t DestMSB, uint8_t DestLSB, uint8_t PacketType);
static void InterpretPacket(uint8_t SizeOfData); 

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;

static uint8_t* DataPacket_Rx;
static uint8_t DataPacket_Tx[40];


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
			ConstructPacket(0x20, 0x8B, PacketType);

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
static void ConstructPacket(uint8_t DestMSB, uint8_t DestLSB, uint8_t PacketType) {
		printf("--------------CONSTRUCTING-----------\n\r");
					ES_Event NewEvent;
					//Header Construction
					printf("Constructing Datapacket (Comm_Service) \n\r");
					DataPacket_Tx[START_BYTE_INDEX] = START_DELIMITER;
					DataPacket_Tx[LENGTH_MSB_BYTE_INDEX] = 0x00; 
					DataPacket_Tx[API_IDENT_BYTE_INDEX_TX] = API_IDENTIFIER_Tx;
					DataPacket_Tx[FRAME_ID_BYTE_INDEX] = FRAME_ID;
					DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = DestMSB; 
					DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = DestLSB; 
					DataPacket_Tx[OPTIONS_BYTE_INDEX_TX] = OPTIONS;
					switch (PacketType) {
						case FARMER_DOG_REQ_2_PAIR:
							printf("Req 2 Pair Construction (Comm Service) \n\r");
						  /*************REDO ALL VALUES HERE************/
							//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = 7;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = FARMER_DOG_REQ_2_PAIR;
							//ADD DATA
							// add check sum
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+1] = 0x4F;
						  //set the frame length as event param
							NewEvent.EventParam = 7;						
							break;
						case FARMER_DOG_ENCR_KEY:
							/*************REDO ALL VALUES HERE************/
							printf("Encryption Key Construction (Comm Service) \n\r");
							//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = 7;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = FARMER_DOG_ENCR_KEY;
							// add check sum
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+1] = 0x4F;
						  //set the frame length as event param
							NewEvent.EventParam = 7;	
							break;
						case FARMER_DOG_CTRL:
							/*************REDO ALL VALUES HERE************/
							printf("Farmer Control Construction \n\r");
								//add the unique frame length
							DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = 7;
							//add the packet type
							DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = FARMER_DOG_CTRL;
							//add in data from IMU SERVICE
							// add check sum
							//DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+13] = ???; NEED TO CALCULATE THE CHECKSUM CORRECTLY!!!
						  //set the frame length as event param
							NewEvent.EventParam = 7;	
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
					break;
				case DOG_ACK :
					NewEvent.EventType = ES_DOG_ACK_RECEIVED;
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


