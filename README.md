# VCAN Router
Integration testing on Truck 312, with all systems running, showed higher than average bus load (~58%), while most other trucks operate closer to 50%. This is patch to lower the bus load on VCAN bus to mitigate intermittent Form Errors seen on the bus.

# Requirements
- Uses a PCAN Router [link here](https://www.peak-system.com/PCAN-Router.228.0.html?&L=1)
- Flash using PCAN USB to CAN device [link here](https://phytools.com/collections/peak-system-technik/products/pcan-usb-adapter)
- Windows
- Requires `msys2` or `mingw` (aka Git Bash) on windows to compile [link here](https://gitforwindows.org/)
  - With `make`
- Download [pcanrouter_devpack](https://www.peak-system.com/quick/DLP-DevPack) from PEAK for flashing tool, compiler, and examples
  - Also install "PEAK Flash" if you don't already have it [here](https://www.peak-system.com/fileadmin/media/files/PEAK-Flash.zip)
  - To install the compiler run/install both applications in the `Compiler` folder
- Ask Platform team for the harness made to flash the PEAK Router. You'll need to have 8-30VDC powered at CAN1 of the PEAK Router. This harness isn't difficult to make, but is already available.

# Behavior
All except blacklisted messages from CAN1 are forwarded to CAN2. All messages from CAN2 are forwarded to CAN1.
- CAN1 = OEM Truck VCAN
- CAN2 = OEM Truck VCAN + AV Actuators on VCAN (2C2, ZF)


# Blacklisted Messages
- SWFDBKA_D2XX
- SWFDBKB_D2XX
- RQST_SC_D2XX
- SWDM_D2XX
- SWCMD_D2XX
- SOFT_SC
- DM2_SC
- DM1_SC
- HRW_2C2
- Prop2C2
- EBC5_2C2
- EBC1_2C2
- XPR_2C2
- XPR_2C2_INTERNAL



# Building
- Navigate to this repo
- `make all`
- Executable is placed in `.out` folder
- If you have build errors it is likely because of your build environment, not the code. Try msys if you are using mingw and vise-versa
- Example of bad environment:
  ```
  Creating Extended Listing: scan_router.lss
  arm-none-eabi-objdump -h -S -C scan_router.elf > scan_router.lss
      0 [main] sh 5392 sync_with_child: child 2908(0x1A8) died before initialization with status code 0xC0000142
     10 [main] sh 5392 sync_with_child: *** child state waiting for longjmp
  /usr/bin/sh: fork: Resource temporarily unavailable  
  make: *** [scan_router.lss] Error 128
  ```

# Flashing
- Directions on how to put PCAN Router into Boot mode are explained in section 5 in [this doc](https://www.peak-system.com/produktcd/Pdf/English/PCAN-Router_UserMan_eng.pdf)
- Open PCAN flash tool
- Click "Next" to get past the welcome page and onto the "Select Hardware page"
  - Once on the Select Hardware page, select "Modules connected to the CAN-Bus" and the correct baud/bit rate (500k for 300+ series trucks)
  - Select PCAN USB from the "Channels of connected CAN hardware" dropdown
  - Click "Detect" (PCAN Router should become available)
  - Click "Next"
- Select the .bin flash file from the .out folder after running make all
- Click "Flash"
- Once the flash is complete close the PCAN Flash application

# Hook up (Truck Install)
- Connect CAN 1 of the router at the VCAN extension behind the green 9-pin OEM diagnostic port
- Connect CAN 2 into the daisy-chain (twisted pair at the end of the M12 cable connected to the VCAN hub)
