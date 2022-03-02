
#include "datatypes.h"
#include "can.h"
#include "can_user.h"
#include "hardware.h"
#include "crc_data.h"

# define TRUE  1
# define FALSE 0

/*
* SCAN Router
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
#define SWFDBKA_D2XX     2566909971 & mask_29
#define SWFDBKB_D2XX     2566910227 & mask_29
#define RQST_SC_D2XX     2565537785 & mask_29
#define SWDM_D2XX        2566911763 & mask_29
#define SWCMD_D2XX       2566909797 & mask_29
#define SOFT_SC          2566838803 & mask_29
#define DM2_SC           2566834963 & mask_29
#define DM1_SC           2566834707 & mask_29

// brake_actuator_2C2_messages.dbc
#define HRW_2C2          2298375728 & mask_29
#define Prop2C2          2566865712 & mask_29
#define EBC5_2C2         2566767664 & mask_29
#define EBC1_2C2         2565865776 & mask_29
#define XPR_2C2          2365544037 & mask_29
#define XPR_2C2_INTERNAL 2365544002 & mask_29

// // others
// #define SWCMD            218099498 & mask_29
// #define SWCMD_FDBK       218099557 & mask_29
// #define SWFDBKA          419426323 & mask_29
// #define SWFDBKB          419426579 & mask_29
// #define Prop2C2          419382064 & mask_29


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
			CAN_UserWrite ( CAN_BUS1, &TxMsg);

			// Toggle LEDs
			CAN2_LED_Toggle ^= 1;
			if ( CAN2_LED_Toggle ){
				HW_SetLED ( HW_LED_CAN2, HW_LED_OFF);
				HW_SetLED ( HW_LED_CAN1b, HW_LED_OFF);
			}else{
				HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
				HW_SetLED ( HW_LED_CAN1b, HW_LED_RED);
			}
		}
	}
}

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
		CAN_UserWrite ( CAN_BUS2, &TxMsg);

		// Toggle LEDs
		CAN1_LED_Toggle ^= 1;
		if ( CAN1_LED_Toggle ){
			HW_SetLED ( HW_LED_CAN1, HW_LED_OFF);
			HW_SetLED ( HW_LED_CAN2b, HW_LED_OFF);
		} else{
			HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
			HW_SetLED ( HW_LED_CAN2b, HW_LED_RED);
		}
	}
}




// main()
// entry point from crt0.S
int  main ( void){

	// init hardware
	HW_Init();

	// init CAN
	CAN_UserInit();


	// Set green LEDs for CAN1 and CAN2
	HW_SetLED ( HW_LED_CAN1, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN2, HW_LED_GREEN);
	HW_SetLED ( HW_LED_CAN1b, HW_LED_ORANGE);
	HW_SetLED ( HW_LED_CAN2b, HW_LED_ORANGE);



	// main loop
	while ( 1){
		CAN1_to_CAN2();
		CAN2_to_CAN1();
	}
}
