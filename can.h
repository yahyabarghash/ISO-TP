#ifndef __CAN_H__
#define __CAN_H__

/*============================================================================*/

/*============================================================================*/

/*============================================================================*/
/*                            Includes                                    */
/*============================================================================*/
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "stm32f4xx_can.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
/*============================================================================*/
/*                            ENUMERATIONS                                    */
/*============================================================================*/
/**
 * Baudrate definition for a CAN interface
 */
typedef enum en_canBrate
{
		en_canBrate_62p5KBPS,
		en_canBrate_125KBPS,
		en_canBrate_250KBPS,
		en_canBrate_500KBPS,
		en_canBrate_1000KBPS,
}en_canBrate_t;

/**
 * CAN interfaces which can be managed by this module
 */
typedef enum en_canIf
{
    en_canIf_iF0,
    en_canIf_iF1,
    en_canIf_iF2,
} en_canIf_t;

typedef enum en_canIoctl
{
    en_canIoctl_filter

} en_canIoctl_t;

typedef enum en_mailBox
{
    /* Mailbox 1 */
    en_mailBox_mailBox1,
    /* Mailbox 2 */
    en_mailBox_mailBox2,
    /* Mailbox 3 */
    en_mailBox_mailBox3,
    /* No Mailbox */
    en_mailBox_noMailBox,
} en_mailBox_t;

/*============================================================================*/
/*                         STRUCTURES AND TYPEDEFS                            */
/*============================================================================*/
typedef int8_t (* can_fp_callback_t)(en_canIf_t e_if, uint16_t c_id,
								uint8_t* pc_pload, uint8_t pc_ploadLen);


/*============================================================================*/
/*                          FUNCTION PROTOTYPES                               */
/*============================================================================*/

/*============================================================================*/
/*! \fn int8_t can_in(e_can_if_t e_if,e_can_brate_t e_brate,
 *                      can_callback_t pf_rxCallback)
    \brief  CAN module initialisation.
    \param  e_if            CAN interface to initialize @ref E_CAN_IF
    \param  e_brate         Enumeration for possible baud rates
                            @ref E_CAN_BRATE_1000KBPS for 1 Mbit/s
                            @ref E_CAN_BRATE_500KBPS for 500 kbit/s
                            @ref E_CAN_BRATE_250KBPS for 250 kbit/s
                            @ref E_CAN_BRATE_125KBPS for 125 kbit/s
    \param  pf_rxCallback   A callback which should be invoked whenever a new
                            frame called and satisfy all selected parameters
                            put NULL here if you want to work in a poll mode
    \return int8_t          0 - init OK, non 0 - init FAILED
*/
/*============================================================================*/
int8_t can_init(en_canIf_t e_if, en_canBrate_t e_brate, can_fp_callback_t pf_rxCallback);

/*============================================================================*/
/*! \fn int8_t can_send(uint16_t i_id, uint8_t* pc_pload, uint8_t c_ploadLen);
    \brief Send a packet via initialised
    \param e_if             CAN interface via which CAN frame should be
                            transmitted, limited by @ref E_CAN_IF
    \param i_id             CAN ID which should be used for a transmission.
    \param pc_pload         Pointer to a buffer where CAN frame payload should
                            be stored
    \param c_ploadLen       CAN frame payload length (max 8 bytes)
    //@retval The number of the mailbox that is used for transmission or
     *        en_mailBox_noMailBox
*/
/*============================================================================*/
en_mailBox_t can_send(en_canIf_t e_if, int16_t i_id,
                uint8_t* pc_pload, uint8_t c_ploadLen);

/*============================================================================*/
/*! \fn can_receive(uint16_t* pi_id, uint8_t* pc_pload, uint8_t* pc_loadLen)
    \brief  If callback parameter for @ref can_init was NULL, then this function
            should be used in order to receive a packet. It is usually used for a
            poll mode.
    \param e_if             CAN interface ID listed in @ref E_CAN_IF from which
                            a CAN frame shall be received.
    \param pi_id            Pointer to memory where CAN ID number should be
                            stored.
    \param pc_pload         Pointer to a memory where a CAN frame payload
                            should be stored
    \param pc_ploadLen      Pointer to a memory where a CAN frame payload
                            length should be stored
    \return int8_t          0 - message reception OK, non 0 - message reception FAILED
*/
/*============================================================================*/
int8_t can_receive(en_canIf_t e_if, uint16_t* pi_id,
		           uint8_t* pc_pload, uint8_t* pc_ploadLen);

/*============================================================================*/
/*! \fn can_poll(e_can_if_t e_if)
    \brief Check if a received message is awaiting processing and call
    	   reception callback if so.
    \param e_if             CAN interface ID listed in @ref E_CAN_IF to be
    			            checked for pending messages
    \param int8_t           0 - poll OK, non 0 - poll FAILED
*/
/*============================================================================*/
int8_t can_poll(en_canIf_t e_if);

/*============================================================================*/
/*! \fn can_ioctl(uint16_t i_id, e_can_ioctl_t e_parType, void* p_ctlValue)
    \brief  Function to control parameters of a CAN interface
            ("input/output control").
    \param e_if             CAN interface ID listed in @ref E_CAN_IF whose
                            parameters shall be set/changed.
    \param e_ctlType        Type of a parameter to control
    \param p_ctlValue       Pointer to memory where parameter value is stored
                            Also can be used to get parameters
    \param int8_t           C0 - init OK, non 0 - init FAILED
*/
/*============================================================================*/
int8_t can_ioctl(en_canIf_t e_if, en_canIoctl_t e_ctlType, void* p_ctlValue);

/*============================================================================*/
/*! \fn int8_t can_deinit(e_can_if_t e_if)
    \brief  CAN module deinitialisation.
    \param  e_if            CAN interface to de-initialize @ref E_CAN_IF
    \return int8_t          0 - de-init OK, non 0 - de-init FAILED
*/
/*============================================================================*/
int8_t can_deinit(en_canIf_t e_if);


/**
 * \brief   reset the RX led
 * \param   void
 * \return  void
 */
void can_resetRxLED( void );
/**
 * \brief   Reset the Tx Led
 * \param   void
 * \return  void
 */
void can_resetTxLED( void );


/**
 * \brief   Set the Tx Led
 * \param   void
 * \return  void
 */
void can_setTxLED( void );

/**
 * \brief   Set the Rx Led
 * \param   void
 * \return  void
 */
void can_setRxLED( void );

/**
 * \brief   initialize the clk
 * \param   void
 * \return  void
 */
void can_initCLK( void );

/**
 * \brief   reset the Tx ticks
 * \param   void
 * \return  void
 */
void can_resetTicksTx( void );

/**
 * \brief   reset the Rx ticks
 * \param   void
 * \return  void
 */
void can_resetTicksRx( void );


/**
 * \brief   get the general ticks
 * \param   void
 * \return  void
 */
uint32_t can_getTicksGeneral( void );
/**
 * \brief   reset the general ticks
 * \param   void
 * \return  void
 */
void can_resetTicksGeneral( void );

/**
 * \brief   get the TX ticks
 * \param   void
 * \return  void
 */
uint32_t can_getTicksTx( void );
/**
 * \brief   get the RX ticks
 * \param   void
 * \return  void
 */
uint32_t can_getTicksRx( void );


#endif








