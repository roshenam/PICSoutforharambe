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

/*----------------------------- Module Defines ----------------------------*/



/*---------------------------- Module Functions ---------------------------*/
static void ConstructPacket(uint8_t DestMSB, uint8_t DestLSB, uint8_t SizeOfData);
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
			uint8_t PacketLength = ThisEvent.EventParam;
			ConstructPacket(0x20, 0x8B, PacketLength);

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
static void ConstructPacket(uint8_t DestMSB, uint8_t DestLSB, uint8_t SizeOfData) {
		DataPacket_Tx[START_BYTE_INDEX] = START_DELIMITER;
		DataPacket_Tx[LENGTH_MSB_BYTE_INDEX] = 0x00;
		DataPacket_Tx[LENGTH_LSB_BYTE_INDEX] = 0x07;
		DataPacket_Tx[API_IDENT_BYTE_INDEX_TX] = API_IDENTIFIER_Tx;
		DataPacket_Tx[FRAME_ID_BYTE_INDEX] = FRAME_ID;
		//uint8_t PairedFarmer_MSB = GetPairedFarmerMSB();
		DataPacket_Tx[DEST_ADDRESS_MSB_INDEX] = DestMSB; //PairedFarmer_MSB;
		//uint8_t PairedFarmer_LSB = GetPairedFarmerLSB();
		DataPacket_Tx[DEST_ADDRESS_LSB_INDEX] = DestLSB; //PairedFarmer_LSB;
		DataPacket_Tx[OPTIONS_BYTE_INDEX_TX] = 0x00;
		DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX] = FARMER_DOG_REQ_2_PAIR;
		// dog tag ID 
		DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+1] = 0x00; 
		DataPacket_Tx[PACKET_TYPE_BYTE_INDEX_TX+2] = 0x51;  // check sum (FF-running sum)

		ES_Event NewEvent;
		NewEvent.EventType = ES_START_XMIT;
		NewEvent.EventParam = 11; //param is length of data packet
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
	/** TESTING CHECKPOINT1: will receive a */
}


/******GETTER FUNCTIONS************/
uint8_t* GetDataPacket_Tx (void) {
  return &DataPacket_Tx[0];
}


