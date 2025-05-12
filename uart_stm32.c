
/* --- License ------------------------------------------------------------- */


/* --- Module Description -------------------------------------------------- */
/**
 * \file    uart_stm32.c

/* --- Includes ------------------------------------------------------------ */
#include "uart_stm32.h"


/* --- Macro Definitions --------------------------------------------------- */


/* --- Type Definitions ---------------------------------------------------- */


/* --- Variables ----------------------------------------------------------- */
USART_InitTypeDef g_initUART;
NVIC_InitTypeDef g_nvicInitStructure;
GPIO_InitTypeDef g_gPIOInitStruct;

/* --- Local Functions Declaration ----------------------------------------- */


static void uart_putchar(char *s);
/* --- Global Functions Definition ----------------------------------------- */

void uart_init(void)
{

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE );

    GPIO_PinAFConfig( GPIOA, GPIO_PinSource2, GPIO_AF_USART2 );
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2 );
    g_gPIOInitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;

    /*
     * the pins are configured as alternate function so
     * the USART peripheral has access to them
     */
    g_gPIOInitStruct.GPIO_Mode = GPIO_Mode_AF;
    /*
     * this defines the IO speed and has nothing to do with the baud rate!
     *
     */
    g_gPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    /*
     * this defines the output type as push pull mode (as opposed to open drain)
     */
    g_gPIOInitStruct.GPIO_OType = GPIO_OType_PP;
    /*
     * this activates the pullup resistors on the IO pins
     */
    g_gPIOInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    /*
     * now all the values are passed to the GPIO_Init()
     * function which sets the GPIO registers
     */
    GPIO_Init(GPIOA, &g_gPIOInitStruct);

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );

    USART_StructInit( &g_initUART );
    g_initUART.USART_BaudRate             =115200;
    g_initUART.USART_WordLength           =USART_WordLength_8b;
    g_initUART.USART_StopBits             =USART_StopBits_1;
    g_initUART.USART_Parity               =USART_Parity_No;
    g_initUART.USART_HardwareFlowControl  =USART_HardwareFlowControl_None;
    g_initUART.USART_Mode                 =USART_Mode_Tx | USART_Mode_Rx;

    USART_Init( USART2, &g_initUART );
    USART_Cmd( USART2, ENABLE );

}

int _write( int file, char* buf, int len )
{

    int _i = 0;
    //for (i = 0; i < len; i++) {
    if ( buf[_i] == '\n' )
    {
        uart_putchar('\r');
        uart_putchar('\n');
    }
    else
    {
        buf[_i] = ( char ) buf[_i];
        uart_putchar( &( buf[_i] ));
    }
//  }

    return len;
}

/* --- Local Functions Definition ----------------------------------------- */
/**
 * \brief     Print the characters passed
 *
 * \param     char *s pointer to the characters to be printed
 * \return    void
 */
static void uart_putchar(char *s)
{
    uint8_t _count=0;
    while(*s)
    {
        // wait until data register is empty
        while( !( USART2 -> SR & 0x00000040 ))
        {
            _count++;
        }
        if( *s == '\n' )
        {
            USART_SendData ( USART2, '\n' );
            break;

        }
        USART_SendData( USART2, *s );
        s++;
    }
}

/* --- EOF ----------------------------------------------------------------- */




//static void Error_Handler(void);

//UART_HandleTypeDef UartHandle_DBG;




