/* --- License ------------------------------------------------------------- */

/* --- Module Description -------------------------------------------------- */
/**
 * \file    packet_scheduler.h


/* --- Includes ------------------------------------------------------------ */
#include "can.h"
#include "tpSnd.h"
#include "tpRcv.h"
/* --- Macro Definitions --------------------------------------------------- */

/* --- Type Definitions ---------------------------------------------------- */

typedef struct st_pckInfTx
{
    en_canIf_t e_if;
    uint32_t srcID;
    uint32_t destID;
    uint8_t pckID;
    uint16_t size;
    uint8_t mailBox;
    en_pckStatus_t pckTxStatus;
    uint8_t *schBuff;
    uint8_t *schBuffFrame;
} st_pckInfTx_t;

typedef enum en_pckState
{
    //the start or the multiple frames and the FF not yet sent (state1)
    en_pckState_FFNotSent,
    //FFsent and waiting for FC (state 2)
    en_pcktState_FFSentFCNotReceived,
    //FC received and CF not sent (state 3)
    en_pckState_FCReceivedCFSent,
} en_pckState_t;

typedef enum en_frameType
{
    en_frameType_fF,
    en_frameType_cF,
    en_frameType_fC,
} en_frameType_t;

typedef enum en_nResult
{
    en_nResult_nOK,
    //when N_Bs is passed out ... (reception of FC on Tx side only)
    en_nResult_nTimeoutBs,
    /*
     * when N_Ar or N_As is passed out its timout value
     *  (any N_PDU on Tx or Rx sides)
     */
    en_nResult_timeoutAs,
    /*
     * when N_Cr is passed out its max value
     * (Reception of CF on the Rx side only)
     */
    en_nResult_timeoutCs,
    //when the sequence number is wrong (on the Rx side only)
    en_nResult_wrongSN,
    //when FS is wrong (on the Tx side only)
    en_nResult_invalidFS,
    //when unexpected PDU received (on the Rx side only)
    en_nResult_unexpPDU,
    /*
     * when FlowStatus = OVFLW when the FF_DL is more than the receiver
     *  side's buffer (on the Tx side only)
     */
    en_nResult_bufferOvFlw,
    //any general error in the network layer (both Tx or Rx)
    en_nResult_error,
    en_nResult_messageCompleteTX
} en_nResult_t;


/* --- Variables ----------------------------------------------------------- */

/* --- Global Functions Declaration ---------------------------------------- */
/**
 * \brief     To store the packet and it its information for sending afterwards
 *
 * \param    e_if            CAN interface
 * \param    *pload          Pointer to the data to be sent
 * \param    srcID           The source ID which can be used to send the packet
 * \param    destID          The destination ID
 * \param    size            The size of the packet
 *
 * \return   N_Result        it returns whether it succeeded or failed to
 *                           store the packet
 */

en_nResult_t pckSch_sendPck( en_canIf_t e_if, uint8_t *pload, uint32_t srcID,
    uint32_t destID, uint16_t size );


/**
 * \brief     To initialize the TP by calling initCLK
 *
 * \return    void
 */
void pckSch_initTp( void );
/**
 * \brief     This functions represents the entry point for handling
 *            the frames whether sending or receiving incoming frames
 *
 * \param    *txflag    to return the status of sending the frame
 * \param    *rxflag    to return the status of receiving the frame
 * \return   void
 */
void pckSch_isotpEntry( en_nResult_t *txflag, en_nResultRx_t *rxflag );

/**
 * \brief     To set the configurations related to the Rx module like BS
 *            STmin, src_ID and the CAN interface
 *
 * \param    BSConf       The block size which will be sent in
 *                        the flow control the sending node
 * \param    StminConf    The min separation time between the consecutive
 *                        frames which will be sent in the flow control the
 *                        sending
 *                        node
 * \param    c_id         The src_ID which will be used to send the
 *                        flow control
 * \param    e_if         The CAN interface
 *
 * \return    void
 */
void pckSch_tpRxConf( uint8_t BSConf, uint8_t StminConf, uint16_t s_id, uint16_t d_id,
    en_canIf_t e_if );

/* --- EOF ---------------------------------------------------------------- */

