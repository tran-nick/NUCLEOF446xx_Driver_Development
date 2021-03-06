/*
 * 009i2c_master_tx_testing.c
 *
 *  Created on: Jul 11, 2020
 *      Author: nhon_tran
 */


#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



#define MY_ADDR		0x61
#define SLAVE_ADDR	0xAA	//given by arduino sketch

//global variable, so we can access in main function
I2C_Handle_t I2C1handle;


void I2C_GPIOInits(void)
{
		GPIO_Handle_t I2CPins;

		I2CPins.pGPIOx = GPIOB;
		I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
		I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
		I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
		I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
		I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

		//SCLK
		I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
		GPIO_Init(&I2CPins);

		//SDA
		I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
		GPIO_Init(&I2CPins);


}


void I2C_Inits(void)
{

	I2C1handle.pI2Cx = I2C1;
	I2C1handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
	I2C1handle.I2C_Config.I2C_DeviceAddress = MY_ADDR;		//reference I2C specification for reserved addresses
	I2C1handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;	//not used, placeholder value
	I2C1handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;	//standard mode, 100 kHz


	I2C_Init(&I2C1handle);

}

void GPIO_ButtonInit(void)
{
	//blue user button on NUCLUEO-446RE board
	GPIO_Handle_t GPIOBtn;

	GPIOBtn.pGPIOx = GPIOC;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GPIOBtn);
}


//sample data to send
//arduino wire library does not accept more than 32 bytes per I2C transaction
uint8_t test_data[] = "Hello World!\n";

int main(void)
{

	//GPIO pin initialization
	I2C_GPIOInits();

	//Initialize user button on Nucleo board
	GPIO_ButtonInit();


	//peripheral initialization
	I2C_Inits();


	while(1){
	//waiting for button press
	while(GPIO_ReadFromInputPin(GPIOC , GPIO_PIN_NO_13));

	//button debouncing
	delay();

	//Sending data to the slave
	//note: remember we can use uint_8 pointer and array variable name interchangeably
	//because they both abstract to first element address in memory
	I2C_MasterSendData(&I2C1handle , test_data , strlen((char*)test_data) ,SLAVE_ADDR );



	}
}
