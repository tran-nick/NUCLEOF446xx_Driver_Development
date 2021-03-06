/*
 * 011i2C_master_rx_testingIT.c
 *
 *  Created on: Jul 15, 2020
 *      Author: nhon_tran
 */



#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MY_ADDR	0x61
#define SLAVE_ADDR 0x68

//prototyping semihosting function
extern void initialise_monitor_handles();


//global variable, so we can access in main function
I2C_Handle_t I2C1handle;
uint8_t RxComplete = RESET;


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
uint8_t rcv_buffer[32];
uint8_t rcv_len;

int main(void)
{

	initialise_monitor_handles();	//use printf statements after this line.

	printf("Application is running\n");

	//GPIO pin initialization
	I2C_GPIOInits();

	//Initialize user button on Nucleo board
	GPIO_ButtonInit();

	//peripheral initialization
	I2C_Inits();

	//IRQ configurations
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV , ENABLE);
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER , ENABLE);


	while(1)
	{
		//waiting for button press
		while(GPIO_ReadFromInputPin(GPIOC , GPIO_PIN_NO_13));

		//button debouncing
		delay();

		//Sending data to the slave
		uint8_t cmd_code = 0x51;
		while( I2C_MasterSendDataIT(&I2C1handle , &cmd_code ,  1 ,SLAVE_ADDR , I2C_ENABLE_SR) != I2C_READY) ;	//len is 1, only sending 1 byte

		//receive length of data to be received
		while( I2C_MasterReceiveDataIT(&I2C1handle , &rcv_len , 1 , SLAVE_ADDR , I2C_ENABLE_SR) != I2C_READY);	//store data len in buffer, specified length as 1 byte

		RxComplete = RESET;	//reset from ApplicationCallBack

		//sending next command
		cmd_code = 0x52;
		while( I2C_MasterSendDataIT(&I2C1handle , &cmd_code ,  1 ,SLAVE_ADDR , I2C_ENABLE_SR)!= I2C_READY);

		//receive data
		while( I2C_MasterReceiveDataIT(&I2C1handle , rcv_buffer , rcv_len , SLAVE_ADDR , I2C_DISABLE_SR)!= I2C_READY);
		/*	compiler implicit conversion, thats why we can pass uint8 as uint32 parameter
		*	get processed as MI2C_MasterReceiveData(param1,param2, (uint32_t) rcv_len ,param4);
		*/
		RxComplete = RESET;	//reset from ApplicationCallBack

		//waiting until Rx is complete
		while(RxComplete != SET);


		rcv_buffer[rcv_len + 1] = '\0';	//adding null character for c style string

		printf("Data : %s" , rcv_buffer);	//semihosting to print data. %s

		RxComplete = RESET;

	}
}



void I2C1_EV_IRQHandler(void)
{
	IRQ_EV_IRQHandling(&I2C1handle);
}


void I2C1_ER_IRQHandler(void)
{
	IRQ_ERR_IRQHandling(&I2C1handle);
}


void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{
	if(AppEv == I2C_EV_TX_CMPLT)
	{
		printf("Tx is completed\n");
	}
	else if(AppEv == I2C_EV_RX_CMPLT)
	{
		printf("Rx is completed\n");

		RxComplete = SET;
	}
	else if(AppEv == I2C_ERROR_AF)
	{
		printf("Error: Ack failure\n");

		I2C_CloseSendData(pI2CHandle);

		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

		while(1);	//hang in loop for ACK failure
	}

}
