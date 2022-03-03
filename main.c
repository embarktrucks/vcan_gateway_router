
#include "datatypes.h"
#include "can.h"
#include "can_user.h"
#include "hardware.h"
#include "crc_data.h"


# define TRUE  1
# define FALSE 0

/*
* VCAN Router
* Copyright: Embark Trucks 2022
* Author: Austin Robinson
* Two way router to forward messages (minus blacklist) for VCAN
*/


// identifier is needed by PCANFlash.exe -> do not delete
const b8_t Ident[] __attribute__ ((used)) = { "PCAN-Router"};


// info data for PCANFlash.exe
const crc_array_t  C2F_Array __attribute__((section(".C2F_Info"), used)) = {

	.Str = CRC_IDENT_STRING,
	.Version = 0x21,
	.Day = 5,
	.Month = 5,
	.Year = 6,
	.Mode = 1,

	// crc infos are patched during link time by flash.ld
	// crc value is patched by PCANFlash.exe
};


// variables for LED toggle
static u8_t CAN1_LED_Toggle;
static u8_t CAN2_LED_Toggle;

// Message Mask
static u32_t mask_29 = 0x1FFFFFFF;


// Blacklisted Message IDs...
// ZF_d2xx_gear_mount_steering.dbc
#define SWFDBKA_D2XX     0x18FFF013 & mask_29  //18FFF013
#define SWFDBKB_D2XX     0x18FFF113 & mask_29  //18FFF113
#define RQST_SC_D2XX     0x18EAFFF9 & mask_29  //18EAFFF9
#define SWDM_D2XX        0x18FFF713 & mask_29  //18FFF713
#define SWCMD_D2XX       0x18FFEF65 & mask_29  //18FFEF65
#define SOFT_SC          0x18FEDA13 & mask_29  //18FEDA13
#define DM2_SC           0x18FECB13 & mask_29  //18FECB13
#define DM1_SC           0x18FECA13 & mask_29  //18FECA13

// brake_actuator_2C2_messages.dbc
#define HRW_2C2          0x8FE6E30 & mask_29  //8FE6E30
#define Prop2C2          0x18FF4330 & mask_29 //18FF4330
#define EBC5_2C2         0x18FDC430 & mask_29 //18FDC430
#define EBC1_2C2         0x18F00130 & mask_29 //18F00130
#define XPR_2C2          0xCFF5665 & mask_29  //CFF5665
#define XPR_2C2_INTERNAL 0xCFF5642 & mask_29  //CFF5642


static int check_IDs(u32_t id)
{
	if((id == SWFDBKA_D2XX) || \
		 (id == SWFDBKB_D2XX) || \
		 (id == RQST_SC_D2XX) || \
		 (id == SWDM_D2XX) || \
		 (id == SWCMD_D2XX) || \
		 (id == SOFT_SC) || \
		 (id == DM2_SC) || \
		 (id == DM1_SC) || \
		 (id == HRW_2C2) || \
		 (id == Prop2C2) || \
		 (id == EBC5_2C2) || \
		 (id == EBC1_2C2) || \
		 (id == XPR_2C2) || \
		 (id == XPR_2C2_INTERNAL)){
				return TRUE;
			} else {
				return FALSE;
			}
}


/*
Forward all CAN traffic from CAN1 (AV Actuators VCAN) to CAN2 (OEM Truck VCAN)
minus blacklisted messages.
If ID = Blacklisted:
	Then don't forward message
else: forward message to CAN2
*/
static void CAN1_to_CAN2(void){
	CANMsg_t  RxMsg, TxMsg;
	int id_match;
	id_match = TRUE; // default to passing messages through, unless ID is a match

	// process messages from CAN1
	if ( CAN_UserRead ( CAN_BUS1, &RxMsg) != 0){
		// Forward CAN1 to CAN2
		u32_t masked_id = RxMsg.Id & mask_29;
		id_match = check_IDs(masked_id);
		if(!id_match){ // if not a match to an ID in the blacklist, then forward it
			// copy message
			TxMsg.Id   = RxMsg.Id;
			TxMsg.Type = RxMsg.Type;
			TxMsg.Len  = RxMsg.Len;
			TxMsg.Data32[0]	= RxMsg.Data32[0];
			TxMsg.Data32[1]	= RxMsg.Data32[1];

			// send
			CAN_UserWrite ( CAN_BUS2, &TxMsg);

			// Toggle LEDs
			CAN2_LED_Toggle ^= 1;
			if ( CAN2_LED_Toggle ){
				HW_SetLED ( HW_LED_CAN2, HW_LED_OFF);
			}else{
				HW_SetLED ( HW_LED_CAN2, HW_LED_ORANGE);
			}
		}
	}
}


/*
Forward all CAN traffic from CAN2 (OEM Truck VCAN) to CAN1 (AV Actuators VCAN)
*/
static void CAN2_to_CAN1(void){
	CANMsg_t  RxMsg, TxMsg;

	// process messages from CAN1
	if ( CAN_UserRead ( CAN_BUS2, &RxMsg) != 0){
		// Forward CAN1 to CAN2
		TxMsg.Id   = RxMsg.Id;
		TxMsg.Type = RxMsg.Type;
		TxMsg.Len  = RxMsg.Len;
		TxMsg.Data32[0]	= RxMsg.Data32[0];
		TxMsg.Data32[1]	= RxMsg.Data32[1];


		// send
		CAN_UserWrite ( CAN_BUS1, &TxMsg);

		// Toggle LEDs
		CAN1_LED_Toggle ^= 1;
		if ( CAN1_LED_Toggle ){
			HW_SetLED ( HW_LED_CAN1, HW_LED_OFF);
		} else{
			HW_SetLED ( HW_LED_CAN1, HW_LED_ORANGE);
		}
	}
}




// main()
// entry point from crt0.S
int main(void){
	// init hardware
	HW_Init();

	// init CAN
	// DEFAULT BAUD RATE = 500K for CAN1 and CAN2
	CAN_UserInit();

	// Set green LEDs for CAN1 and CAN2
	HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);

	// main loop
	while (1){
		CAN1_to_CAN2();
		CAN2_to_CAN1();
	}
}
