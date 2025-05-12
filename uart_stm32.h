/* --- License ------------------------------------------------------------- */


/* --- Module Description -------------------------------------------------- */
/**
 * \file    uart_stm32.h
/* --- Includes ------------------------------------------------------------ */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"

/* --- Macro Definitions --------------------------------------------------- */

#define  USART2_TX_PIN          GPIO_Pin_2
#define  USART2_TX_GPIO_PORT    GPIOA
#define  USART2_TX_AF           GPIO_AF7_USART2
#define  USART2_RX_PIN          GPIO_Pin_3
#define  USART2_RX_GPIO_PORT    GPIOA
#define  USART2_RX_AF           GPIO_AF_USART2            //GPIO_AF7_USART2

/* --- Type Definitions ---------------------------------------------------- */


/* --- Variables ----------------------------------------------------------- */


/* --- Global Functions Declaration ---------------------------------------- */
/**
 * \brief     to initialize the uart
 *
 * \param     void
 * \return    none
 */
void uart_init( void );

/* --- EOF ---------------------------------------------------------------- */



//static void Error_Handler(void);

