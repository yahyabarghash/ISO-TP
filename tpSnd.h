/* --- License ------------------------------------------------------------- */


/* --- Module Description -------------------------------------------------- */
/**
 * \file    tpSend.h
cludes ------------------------------------------------------------ */
#include "can.h"
//#include "tpReceive.h"

/* --- Macro Definitions --------------------------------------------------- */
#define MAXISOTPMESSAGESIZE 4096

/* --- Type Definitions ---------------------------------------------------- */
typedef enum en_fSInf
{
    en_fSInf_invalid,
    en_fSInf_cTS,
    en_fSInf_wait,
    en_fSInf_ovFlw,
} en_fSInf_t;



typedef struct st_FCConf
{
    uint8_t blockSize;
    uint32_t sTmin; // in milli seconds
    en_fSInf_t fS;
    uint16_t srcID;
    uint16_t destID;
    en_canIf_t e_if;
    // TODO timer callback for multi frame
} st_FCConf_t;

typedef enum en_pckStatus
{
    en_pckStatus_txStatusPending,
    en_PckStatus_txStatusFailed,
    en_PckStatus_txStatusLoaded,
    en_PacketStatus_txStatusOk,
    en_PckStatus_txStatusProccessing,
} en_pckStatus_t;



typedef enum en_nResultTx
{
    en_nResultTx_okTx,
    /*
     * when N_Ar or N_As is passed out its timout value
     * ( any N_PDU on Tx or Rx sides )
     */
    en_nResultTx_timeoutATx,
    //when N_Bs is passed out ... ( reception of FC on Tx side only )
    en_nResultTx_timeoutBsTx,
    /*
     * when N_Cr is passed out its max value
     * ( Reception of CF on the Rx side only )
     */
    en_nResultTx_timeoutCrTx,
    //when the sequence number is wrong ( on the Rx side only )
    en_nResultTx_wrongSNTx,
    //when FS is wrong ( on the Tx side only )
    en_nResultTx_invalidFSTx,
    //when unexpected PDU received ( on the Rx side only )
    en_nResultTx_unexpPDUTx,
    /*
     * when FlowStatus = OVFLW when the FF_DL is more than
     * the receiver side's buffer ( on the Tx side only )
     */
    en_nResultTx_buffOvFlwTx,
    en_nResultTx_errorTx,     //any general error in the network layer ( both Tx or Rx )
} en_nResultTx_t;

/* --- Variables ----------------------------------------------------------- */
extern uint8_t g_fCRx;
/* --- Global Functions Declaration ---------------------------------------- */


/*
* \brief     sending one isotp frame and it is assigned as the callback
*            to the can module
*
* \param     e_if the can interface
* \param     *pload the payload of the message
* \param     arbitration_id  CAN ID which should be used for a transmission.
* \param     size the size of the message
* \return   The number of the mailbox that is used for transmission or
*           CAN_TxStatus_NoMailBox if there is no empty mailbox
*/
en_mailBox_t tpSnd_isoTpsend ( en_canIf_t e_if,uint8_t *pload, uint32_t arbitration_id,
                   uint8_t size );

/**
  * @brief  This call back function should make the configurations of BS,
  *  Stmin or others associated with Flow control frame
  \param e_if             CAN interface ID listed in @ref E_CAN_IF whose
                            parameters shall be set/changed.
  * @param  TransmitMailbox: the number of the mailbox that is used for
  * transmission.
  * @retval CAN_TxStatus_Ok if the CAN driver transmits the message,
  *         CAN_TxStatus_Failed in an other case.
  */
en_nResultTx_t tpSnd_fC_callback( uint8_t* pc_pload );
/**
 * @brief This is called in isoTpEntryTx to get the FC configuration
 *
 */
st_FCConf_t tpSnd_getFC( void );

/*
 * @brief to check if there is a FC received or not
 */
uint8_t tpSnd_getFCRxFlag();
/*
 * @brief To reset the flag associated with the reception of the FC
 */
void tpSnd_resetFCRxFlag();


/* --- EOF ---------------------------------------------------------------- */
