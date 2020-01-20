/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
  Copyright 2019  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the PAD program. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "PAD.h"


#define debut_rcpt 0x40 //valeur arbitraire pour détecter un début de trame

uint8_t boutons;
uint8_t joystick_x;
uint8_t joystick_y;


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
    
    //LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
    {
	  USB_USBTask();
      //HID_Task();
      PAD_Task();
    }
}

/** Configures the board hardware and chip peripherals for the project's functionality. */
void SetupHardware(void)
{
	#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
	#endif
    
	/**INITIALISATIONS*/
	/* Hardware Initialization */
	USB_Init();
	Serial_Init(9600,0); 	//9600 Bauds, configuration

	/* Initialize Relays */
	//DDRD  |=  ALL_RELAYS;		// Port pour la liaison série
	//PORTD &= ~ALL_RELAYS;		
}


/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the keyboard device endpoints.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup HID Report Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(KEYBOARD_OUT_EPADDR, EP_TYPE_INTERRUPT, LED_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(KEYBOARD_IN_Bout_EPADDR, EP_TYPE_INTERRUPT, BOUTONS_EPSIZE, 1);	
	ConfigSuccess &= Endpoint_ConfigureEndpoint(KEYBOARD_IN_Joy_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	

	/* Turn on Start-of-Frame events for tracking HID report period expiry */
	USB_Device_EnableSOFEvents();

	/* Indicate endpoint configuration success or failure */
	//LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}



/** Sends the next PAD report to the host, via the buttons and joystick endpoint. */
void Send_EP_IN_Report(void)
{
	//static USB_KeyboardReport_Data_t PrevKeyboardReportData;
	//USB_KeyboardReport_Data_t        KeyboardReportData;
	//bool                             SendReport = false;

	/* Create the next keyboard report for transmission to the host */
	//CreateKeyboardReport(&KeyboardReportData);

	/* Check if the idle period is set and has elapsed */
	//if (IdleCount && (!(IdleMSRemaining)))
	//{
		/* Reset the idle time remaining counter */
	//	IdleMSRemaining = IdleCount;

		/* Idle period is set and has elapsed, must send a report to the host */
		//SendReport = true;
	//}
	//else
	//{
		/* Check to see if the report data has changed - if so a report MUST be sent */
		//SendReport = (memcmp(&PrevKeyboardReportData, &KeyboardReportData, sizeof(USB_KeyboardReport_Data_t)) != 0);
	//}
	
	
	/** Envoi sur INTERFACE BOUTONS**/
	/* Select the  Buttons Report Endpoint */
	Endpoint_SelectEndpoint(KEYBOARD_IN_Bout_EPADDR);

	/* Check if Keyboard Endpoint Ready for Read/Write and if we should send a new report */
	if (Endpoint_IsReadWriteAllowed() && Endpoint_IsINReady())
	{
		/* Save the current report data for later comparison to check for changes */
		//PrevKeyboardReportData = KeyboardReportData;

		/* Write Keyboard Report Data */
		//Endpoint_Write_Stream_LE(&KeyboardReportData, sizeof(KeyboardReportData), NULL);
		Endpoint_Write_8(boutons);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();
	}
	else printf("Buttons endpoint not ready to send\n");
	
		/** Envoi sur INTERFACE Joystick**/
	/* Select the Keyboard Report Endpoint */
	Endpoint_SelectEndpoint(KEYBOARD_IN_Joy_EPADDR);

	/* Check if Keyboard Endpoint Ready for Read/Write and if we should send a new report */
	if (Endpoint_IsReadWriteAllowed() && Endpoint_IsINReady())
	{
		/* Save the current report data for later comparison to check for changes */
		//PrevKeyboardReportData = KeyboardReportData;

		/* Write Keyboard Report Data */
		//Endpoint_Write_Stream_LE(&KeyboardReportData, sizeof(KeyboardReportData), NULL);
		Endpoint_Write_8(joystick_x);
		Endpoint_Write_8(joystick_y);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();
	}
	else printf("Joystick endpoint not ready to send\n");
}



/** Reads the next LED status report from the host from the LED data endpoint, if one has been sent. */
void Receive_EP_OUT_Report(void)	//Réception Liaison Série
{
	/* Select the Keyboard LED Report Endpoint */
	Endpoint_SelectEndpoint(KEYBOARD_OUT_EPADDR);

	/* Check if Keyboard LED Endpoint contains a packet */
	if (Endpoint_IsOUTReceived())
	{
		/* Check to see if the packet contains data */
		if (Endpoint_IsReadWriteAllowed())
		{
			/* Read in the LED report from the host */
			uint8_t LEDReport = Endpoint_Read_8();

			/* On envoie l'octet de commande de led sur la liaison série */
			Serial_SendByte(LEDReport);
		}
		else printf("LED endpoint not ready to receive\n");

		/* Handshake the OUT Endpoint - clear endpoint and ready for next report */
		Endpoint_ClearOUT();
	}
}



void Reception_Serie(void){
	if(Serial_IsCharReceived()){ // A la réception d'un caractère...
	/* on vérifie qu'on a bien un début de trame*/
    	unsigned char caractere = Serial_ReceiveByte();
    	
    	/* si oui, on récupère les trois octets suivants*/
        if (caractere==debut_rcpt){
                boutons = Serial_ReceiveByte();
                joystick_x = Serial_ReceiveByte();
                joystick_y = Serial_ReceiveByte();
        }
	}
}


/** Function to manage PAD report generation and transmission to the host, when in report mode. */
void PAD_Task(void)
{
	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;
	
    Reception_Serie();	//Réception des données de la liaison série
    
	/* Send the next button/joystick report to the host */
	Send_EP_IN_Report();

	/* Process the LED report sent from the host */
	Receive_EP_OUT_Report(); 
}


