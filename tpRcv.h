
/* Public: A handle for beginning and continuing receiving a single ISO-TP
 * message - both single and multi-frame.
 *
 * Since an ISO-TP message may contain multiple frames, we need to keep a handle
 * around while waiting for subsequent CAN messages to complete the message.
 * This struct encapsulates the local state required.
 *
 * completed - True if the received message request is completely finished.
 * success - True if the message request was successful. The value if this field
 *      isn't valid if 'completed' isn't true.
 */

/* --- License ------------------------------------------------------------- */


/* --- Module Description -------------------------------------------------- */
/**
 * \file    tpReceive.h


/* --- Includes ------------------------------------------------------------ */
#include "can.h"
#include "ringbuffer.h"
/* --- Macro Definitions --------------------------------------------------- */
#define MAXBUFFSIZE 4096


/* --- Type Definitions ---------------------------------------------------- */
typedef enum en_pckStatusRx
{
    en_pckStatusRx_pending,
    en_pckStatusRx_failed,
    en_pckStatusRx_loaded,
    en_pckStatusRx_ok,
    en_pckStatusRx_proccessing,
} en_pckStatusRx_t;



typedef struct st_pckInfRx
{
    en_canIf_t e_if;
    uint32_t arbitrationId;
    uint8_t pckId;
    uint16_t size;
    uint8_t mailBox;
    en_pckStatusRx_t pckRxStatus;
    uint8_t *p_loc;
} st_pckInfRx_t;



typedef enum
{
    CANTP_EXTENDED, CANTP_STANDARD
} en_canTpAddressingType_t;

typedef struct st_isoTpConf
{
    uint16_t timeout;
    // timeout_ms: ISO_TP_DEFAULT_RESPONSE_TIMEOUT,
    uint16_t incomingMessageSize;
    en_canTpAddressingType_t addType;
} st_isoTpConf_t;


/*
 * It states the outer states as o stands for outer
 */
typedef enum en_oState
{
    //the start and then decides either single frame or multiple frame
    en_oState_start,
    //for receiving a single frame
    en_oState_singleFrame,
    //for receiving multiple frame
    en_oState_multipleFrame,
    //For calling the callback function for sending the FC in tpsend
    en_oState_fC,
} en_oState_t;

/*
 * To state the inner states as i stands for inner
 */
typedef enum en_iState
{
    //receiving first frame
    en_iState_receivingFF,
    //sending Flow Control
    en_iState_sendingFC,
    /*
     * receiving Consecutive frame and when the
     * message is received go to start again
     */
    en_iState_receivingCF,
} en_iState_t;

typedef enum en_typeTimeout
{
    en_typeTimeout_AR,
    en_typeTimeout_BR,
    en_typeTimeout_CR,
} en_typeTimeout_t;

typedef enum en_nResultRx
{
    en_nResultRx_okRx,
    /*
     * when N_Ar or N_As is passed out its timout value
     * ( any N_PDU on Tx or Rx sides )
     *
     */
    en_nResultRx_timeoutAr,
    /*
     * when N_Bs is passed out ... ( reception of FC on Tx side only )
     *
     */
    en_nResultRx_timeoutBr,
    /*
     * when N_Cr is passed out its max value
     * ( Reception of CF on the Rx side only )
     */
    en_nResultRx_timeoutCr,
    /*
     * when the sequence number is wrong ( on the Rx side only )
     */
    en_nResultRx_wrongSNRx,
    //when FS is wrong ( on the Tx side only )
    en_nResultRx_invalidFSRx,
    //when unexpected PDU received ( on the Rx side only )
    en_nResultRx_unexpPDURx,
    /*
     * when FlowStatus = OVFLW when the FF_DL is more than
     * the receiver side's buffer ( on the Tx side only )
     */
    en_nResultRx_buffOvFlwRx,
    //any general error in the network layer ( both Tx or Rx )
    en_nResultRx_errorRx,
    en_nResultRx_messageComplete,
} en_nResultRx_t;


/* --- Variables ----------------------------------------------------------- */


/* --- Global Functions Declaration ---------------------------------------- */
/**
 * \brief     it handles the first frame received and assign the
 *            length of the packet to data_buf_length
 *
 * \param     e_if the can interface
 * \param     pc_pload the payload of the message
 * \param     c_id  CAN ID which should be used for a transmission.
 * \param     pc_ploadLen the size of the message
 * \return    N_Result_rx if it is N_BUFFER_OVFLW_Rx or N_OK_Rx
 */
en_nResultRx_t tpRcv_handleFF( en_canIf_t e_if, uint16_t c_id,
                                      uint8_t* pc_pload, uint16_t pc_ploadLen );

/**
 * \brief     it handles the first frame received and assign the length of
 *             the packet to data_buf_length
 * \param     e_if the can interface
 * \param     pc_pload the payload of the message
 * \param     c_id  CAN ID which should be used for a transmission.
 * \param     pc_ploadLen the size of the message
 * \return    N_Result_rx if it is N_WRONG_SN_Rx, N_WRONG_SN_Rx,
 *            MESSAGE_COMPLETE or N_OK_Rx
 */
en_nResultRx_t tpRcv_handleCF( en_canIf_t e_if, uint16_t c_id,
                                             uint8_t* pc_pload,
                                             uint16_t pc_ploadLen );
/**
 * \brief     it handles the single frame received and assign the
 *            length of the packet to data_buf_length
 *
 * \param     e_if the can interface
 * \param     pc_pload the payload of the message
 * \param     c_id  CAN ID which should be used for a transmission.
 * \param     pc_ploadLen the size of the message
 * \return    N_Result_rx if it is N_WRONG_SN_Rx, N_WRONG_SN_Rx,
 *            MESSAGE_COMPLETE or N_OK_Rx
 */
en_nResultRx_t tpRcv_handleSingleFrame( en_canIf_t e_if, uint16_t c_id,
                                       uint8_t* pc_pload,
                                       uint16_t pc_ploadLen );

/**
 * \brief     to initialize the buffers data_ringb, data_rx_ringb,
 *            data_len_rx_ringb
 * \return    void
 */
void tpRcv_buffinit( void );


/**
 * \brief     it get the isotp_conf
 * \return    IsoTpConf returns the isotp_conf
 */
st_isoTpConf_t tpRcv_getIsoTPconf( void );

/**
 * \brief     this is the callback function which is called whenever
 *            there is a can frame received and push its length
 *            and the data in the buffers
 * \param     e_if the can interface
 * \param     pc_pload the payload of the message
 * \param     c_id  CAN ID which should be used for a transmission.
 * \param     pc_ploadLen the size of the message
 * \return    N_Result_rx if it is N_WRONG_SN_Rx, N_WRONG_SN_Rx,
 *            MESSAGE_COMPLETE or N_OK_Rx
 */
void tpRcv_callbackBufferSave( en_canIf_t e_if, uint16_t c_id,
                           uint8_t* pc_pload, uint16_t pc_ploadLen );

/**
 * \brief     it get the buffer data_len_rx_ringb
 * \return    s_ringb_t* returns the pointer to the data_len_rx_ringb buffer
 */
s_ringb_t* tpRcv_getDataLengthRxBuff( void );

/**
 * \brief     it get the buffer data_rx_ringb
 * \return    s_ringb_t* returns the pointer to the data_rx_ringb buffer
 */
s_ringb_t* tpRcv_getDataRxBuff( void );


/**
 * \brief     to receive the first packet received
 * \param     *dptr the pointer the the data of the packet
 * \return    returns the length of the packet
 */
uint16_t tpRcv_recvPck( uint8_t *dptr );

/* --- EOF ---------------------------------------------------------------- */
