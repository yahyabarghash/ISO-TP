/*
 * CAN_Config.h
 *
 *  Created on: 19.08.2015
 *      Author: komlab01
 */

#ifndef CAN_CONFIG_H_
#define CAN_CONFIG_H_


#include "can.h"

#define		GPIOx_CONF			GPIOD
#define		CANTx_pin_CONF 		GPIO_Pin_1
#define		CANRx_pin_CONF 		GPIO_Pin_0


// For CAN_Init(CAN_TypeDef* CANx, CAN_InitTypeDef* CAN_InitStruct)
#define		CAN_Mode_CONF		CAN_Mode_Normal
#define 	CAN_SJW_CONF		CAN_SJW_1tq
#define 	CAN_BS1_CONF		CAN_BS1_14tq
#define		CAN_BS2_CONF		CAN_BS2_6tq
#define		CAN_Prescaler_CONF	16



//For the TxMessage
#define 	IDE_CONF	        CAN_Id_Standard
#define		ExtId_CONF	        0
#define		RTR_CONF	        CAN_RTR_Data
#define		DLC_CONF	        1

//For the Reception
#define     CANRx_FIFO_CONF 	CAN_FIFO0
#define     CANPOLL_FIFO_CONF	CAN_FIFO0



#endif /* CAN_CONFIG_H_ */

