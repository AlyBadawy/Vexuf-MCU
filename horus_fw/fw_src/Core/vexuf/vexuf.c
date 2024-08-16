/*
 * vexuf.c
 *
 *  Created on: Jul 27, 2024
 *      Author: Aly Badawy
 */


#include "vexuf.h"
#include "vexuf_helpers.h"
#include "vexuf_config.h"
#include "vexuf_sd_card.h"
#include "vexuf_indicators.h"
#include "vexuf_timers.h"
#include "vexuf_adc_avs.h"
#include "vexuf_triggers.h"
#include "vexuf_pwm.h"
#include "vexuf_output.h"



extern ADC_HandleTypeDef hadc1;
extern uint32_t adcBuffer[5];



char serialNumber[SERIAL_NUMBER_LENGTH];
uint32_t registrationNumber;
char callsign[CALLSIGN_LENGTH];
SerialConfiguration serialInterface;
SpiType spiType;
LcdConfiguration lcdConfig;
I2CConfiguration i2cConfig;
OutputConfiguration outputConfig;
ActuatorsValues actuatorsDefaults;
AlarmConfiguration alarmConfig[2];
PwmConfiguration pwmConfig;

TriggerConfiguration triggers[TRIGS_COUNT];
AvSensor avSensors[NUMBER_OF_AVS];

VexufStatus vexuf_status;
IndStatus ind_status;



void VexUF_Init(void) {

	IND_setStatus(IndWarn, IndON);

	VexUF_GenerateSerialNumber();


	//	 Check if the EEPROM has configuration, otherwise halt!
	if (!CONFIG_IsConfigured()) {
		CONFIG_HandleNoConfig();
	}
	CONFIG_WriteSerialNumber();

	CONFIG_LoadSettingsFromEEPROM();

	PWM_init();

	HAL_Delay(500);

	ACTUATORS_Test(); // TODO: remove before release

 	HAL_ADC_Start_DMA(&hadc1, adcBuffer, 5);
	HAL_Delay(20);
	TIMERS_Start();

	OUTPUT_BuzzOnStartUp();
	IND_setStatus(IndWarn, IndOFF);

}

void VEXUF_run(void) {
	ADC_Scan();
	SDCard_checkCard();

	if (vexuf_status.sd_card_error && outputConfig.error_on_no_sd) SDCard_HandleError();

	if (vexuf_status.timer_0d1hz_ticked == 1) {
		TRIGGERS_runAll();
		vexuf_status.timer_0d1hz_ticked = 0;
	}
}


