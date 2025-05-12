/* --- License ------------------------------------------------------------- */


/* --- Module Description -------------------------------------------------- */
/**
 * \file    packet_scheduler.c

/* --- Includes ------------------------------------------------------------ */
#include "pckSch.h"
#include "logger.h"


/* --- Macro Definitions --------------------------------------------------- */
#define LOGGER_ENABLE 0
#define LOGGER_LEVEL 3


#define MAXNOPCK 40
#define MAXPCKBUFSIZE 4096

/* for 100usec */
#define TXTIMEOUT 100000
#define RXTIMEOUT 100000

/* Types of FS*/
#define FCTYPE 0x30
#define CFTYPE 0x20
#define FFTYPE 0x10

#define HNIBBLEMASK 0xF0

#define MAXSN 14
/* --- Type Definitions ---------------------------------------------------- */


/* --- Variables ----------------------------------------------------------- */
static st_pckInfTx_t a_pckInfoTx[MAXNOPCK];
/* original data*/
static uint8_t*     p_schBuf[MAXNOPCK];
static uint8_t*     p_schBufFrame[MAXNOPCK];
static uint8_t      flag = 0;
static uint16_t     pckId = 0;
static uint16_t     totPckSize = 0;
static uint16_t     currentPckID = 0;
static uint16_t     blockSTXCurrent = 0;
/* this flag to indicate if the current pakcet is new or not */
static uint8_t      EndOfPck = 0;
static en_pckState_t TxState = en_pckState_FFNotSent;
static uint16_t     numberMessages;
static int8_t       SN;
static en_frameType_t  frameType;
//fF stands for first frame
static uint8_t      fFSent;
/* To indicate there is CF in the mail box */
static uint8_t      CFInMailBox = 0;
static en_mailBox_t currentMailBox;
en_nResult_t        g_nResult = en_nResult_nOK;
/*for the timers*/
static uint32_t     nBs = 1;
static uint32_t     nCs = 1;
static uint32_t     nAs = 1;
static uint32_t     sTmin = 1;
static uint32_t     timeOut = 1;
static uint8_t      CFSent = 0;
static st_FCConf_t  st_FCConf_t_Tx;
static st_FCConf_t  st_FCConf_t_Rx;
static uint32_t     endRemaining;
//fc for Flow control
uint8_t             g_fCRx;
static uint8_t      fCReceived;

/*
 * tprecieve variables
 */
static uint8_t      blockSize = 3;
static uint32_t     nBr = 1;
static uint32_t     nCr = 1;
static uint32_t     nAr = 1;
static en_typeTimeout_t  tTimeout = en_typeTimeout_AR;
static en_oState_t  oState = en_oState_start;
static en_iState_t  iState = en_iState_receivingFF;
static en_nResultRx_t  nResultRx = en_nResultRx_okRx;
static uint8_t      endMessage = 0;
static uint8_t      timeBefore = 1;
static uint16_t     blockSizeRXCurrent = 0;

static st_pckInfRx_t pckInfoRx;


/* --- Local Functions Declaration ----------------------------------------- */

en_nResult_t _isotpEntryTx( void );

uint8_t _tpRcv( void );
uint8_t _isotpEntryRx( void );

uint8_t _writePck( void );

void _resetSCHRx( void );
void _resetSCHTx( void );
void _resetPckInfoTx( void );

en_nResultRx_t _handleDataNonExist(void);
en_nResultRx_t _handleDataExist( uint8_t d_len, s_ringb_t* data_rx_buff);
en_nResultRx_t _handleStartState( uint8_t _nibble2, uint16_t _cId,
                             uint16_t _ploadLen, uint8_t* _p_pload);
en_nResultRx_t _handleMultipleFrameState( uint8_t _nibble2, uint16_t _cId,
                             uint16_t _ploadLen, uint8_t* _p_pload);
en_nResultRx_t _handleFFState( uint8_t _nibble2, uint16_t _cId,
                             uint16_t _ploadLen, uint8_t* _p_pload);

/* --- Global Functions Definition ----------------------------------------- */

en_nResult_t pckSch_sendPck( en_canIf_t e_if, uint8_t *pload, uint32_t srcID,
                         uint32_t destID, uint16_t size )
{


    if ( size+totPckSize<MAXPCKBUFSIZE )
    {
        a_pckInfoTx[pckId].e_if = e_if;
        a_pckInfoTx[pckId].srcID = srcID;
        a_pckInfoTx[pckId].destID = destID;
        a_pckInfoTx[pckId].pckID  = pckId;
        a_pckInfoTx[pckId].size = size;
        a_pckInfoTx[pckId].pckTxStatus =
            en_pckStatus_txStatusPending;
        p_schBuf[pckId] = pload;
        p_schBufFrame[pckId] = pload;
        totPckSize += size;
        pckId++;
        numberMessages++;
        g_nResult = en_nResult_nOK;
        LOG2_OK( "packet no %d added!", pckId );
    }
    else
    {
        g_nResult = en_nResult_bufferOvFlw;  // No space for the new packet
        LOG2_OK( "No space for storing the new packet!", 2 );
    }
    return g_nResult;
}



void pktSch_initTp()
{
    can_initCLK();

}

void pktSch_tpRxConf( uint8_t BSConf, uint8_t StminConf, uint16_t srcID,
    uint16_t destID, en_canIf_t e_if )
{
    st_FCConf_t_Rx.blockSize = BSConf;
    st_FCConf_t_Rx.sTmin = StminConf;
    st_FCConf_t_Rx.srcID = srcID;
    st_FCConf_t_Rx.destID = destID;
    st_FCConf_t_Rx.e_if = e_if;
}




void pktSch_isotpEntry( en_nResult_t *txflag, en_nResultRx_t *rxflag )
{

    //LOG_HEXDUMP( sch_buf, 8 );
    *txflag = _isotpEntryTx();
    *rxflag = _isotpEntryRx();

}






/* --- Local Functions Definition ----------------------------------------- */


/**
 * \brief     to handle the sending of the frames
 *
 * \return    returns the status of sending such a frame
 */
en_nResult_t _isotpEntryTx()
{
        en_nResult_t _nResult;
        en_mailBox_t _retval = en_mailBox_noMailBox;
        //the payload of the SF is 7 bytes )
        if ( a_pckInfoTx[currentPckID].size > 7 && numberMessages > 0 )
        {
            switch ( TxState ) {
                case en_pckState_FFNotSent:
                    /*
                     * reset the Ticks for TXclck to avoid over flow
                     */
                    can_resetTicksTx();
                    CFInMailBox = 0;
                    LOG2_OK( "Multiple frame has been found to send" );
                    LOG2_OK( "Trying to send the first frame!" );
                    can_setTxLED();
                    SN = 0;
                    _retval = _writePck();
                    EndOfPck = 0;
                    if ( _retval != en_mailBox_noMailBox )
                    {
                        LOG_HEXDUMP( p_schBuf[pckId - 1],
                                     a_pckInfoTx[pckId - 1].size );
                        LOG2_OK( "The first frame has been sent" );
                        TxState = en_pcktState_FFSentFCNotReceived;
                        nBs = can_getTicksTx() + TXTIMEOUT;
                        LOG2_OK( "Time stamp( N_Bs ): %d", nBs );
                        _retval = en_nResult_nOK;
                        _nResult = en_nResult_nOK;
                    }
                    break;
                case en_pcktState_FFSentFCNotReceived:
                    /*
                     * check if the FC has been received or not
                     */
                    if ( tpSnd_getFCRxFlag() == 1 )
                    {
                        fCReceived = 1;
                        st_FCConf_t_Tx = tpSnd_getFC();
                    }
                    /*
                     * check if the FC received and N_Bs not time out
                     */
                    if ( tpSnd_getFCRxFlag() == 1 && nBs >= can_getTicksTx())
                    {
                        // checks if N_Bs is not time out yet
                        LOG2_OK( "Time stamp( N_Bs ): %d", nBs );
                        LOG2_OK( "The flow control has been received" );
                        nAs = can_getTicksTx() + TXTIMEOUT;
                        LOG1_OK( "Time stamp( _N_As ): %d", nAs );
                        if ( !CFInMailBox )
                        {
                            currentMailBox = _writePck();
                        }
                        if ( isTransmitted( st_FCConf_t_Tx.e_if,
                                            currentMailBox )
                            && currentMailBox != en_mailBox_noMailBox
                            && !EndOfPck && nAs >= can_getTicksTx())
                        {
                            CFInMailBox = 0;
                            tpSnd_resetFCRxFlag();
                            LOG2_OK( "Time stamp( _N_As ): %d", can_getTicksTx());
                            LOG2_OK( "Sending the 1st consecutive frame \
                                succeeded" );
                            nCs = can_getTicksTx() + TXTIMEOUT;
                            sTmin = can_getTicksTx() + st_FCConf_t_Tx.sTmin;
                            LOG2_OK( "Time stamp( N_Cs ): %d", nCs );
                            blockSTXCurrent++;
                            TxState = en_pckState_FCReceivedCFSent;
                            if ( blockSTXCurrent >= st_FCConf_t_Tx.blockSize )
                            {
                                nBs = can_getTicksTx()+TXTIMEOUT;
                                LOG2_OK( "Time stamp( _N_Cs ): %d", nBs );
                                TxState = en_pcktState_FFSentFCNotReceived;
                                blockSTXCurrent = 0;
                            }
                            CFSent = 1;
                        }
                        //at the end of the packet sending
                        else if ( _retval != en_mailBox_noMailBox
                            && EndOfPck )
                        {
                            _nResult = en_nResult_messageCompleteTX;
                            _retval = en_nResult_messageCompleteTX;
                            LOG2_OK( "Message Complete" );
                            can_resetTxLED();
                            blockSTXCurrent = 0;
                            TxState = en_pckState_FFNotSent;
                            CFSent = 1;
                            _resetPckInfoTx();
                        }
                        else if ( nAs < can_getTicksTx())
                        {
                            LOG1_OK( "Time out _N_As, %d", can_getTicksTx());
                            LOG1_OK( " _N_As, %d", nAs );
                            _nResult = en_nResult_timeoutAs;
                            _retval = _nResult;
                            blockSTXCurrent = 0;
                            can_resetTxLED();
                            TxState = en_pckState_FFNotSent;
                            nAs = 1;
                            numberMessages--;
                            //return n_result;
                        }
                    }
                    else if ( nBs < can_getTicksTx())
                    {
                        /*
                         * if time out then abort transmission and return
                         * N_TIMEOUT_Bs
                         */
                        LOG2_OK( "Time out N_Bs, %d, N_Bs:%d", can_getTicksTx(),
                                 nBs );
                        _nResult = en_nResult_nTimeoutBs;
                        _retval = _nResult;
                        blockSTXCurrent = 0;
                        TxState = en_pckState_FFNotSent;
                        can_resetTxLED();
                        nBs = 1;
                        numberMessages--;
                    }
                    break;
                case en_pckState_FCReceivedCFSent:
                    if ( nCs > nAs )
                    {
                        timeOut = nCs;
                    }
                    else
                    {
                        timeOut = nAs;
                    }
                    if ( st_FCConf_t_Tx.blockSize > blockSTXCurrent
                        && timeOut >= can_getTicksTx()
                        && sTmin < can_getTicksTx())
                    {
                        LOG2_OK( "Time stamp( _N_Cs ): %d", nCs );
                        nAs = can_getTicksTx() + TXTIMEOUT;
                        LOG2_OK( "Time stamp( _N_As ): %d", nAs );
                        LOG2_OK( "Trying to send consecutive frame no %d",
                                 blockSTXCurrent );
                        LOG2_OK( "Time stamp( Ticks ): %d", can_getTicksTx());
                        if ( !CFInMailBox )
                        {
                            currentMailBox =  _writePck();
                        }
                        if ( isTransmitted( st_FCConf_t_Tx.e_if,
                                            currentMailBox )
                            && currentMailBox !=  en_mailBox_noMailBox
                            && nAs >= can_getTicksTx() && !EndOfPck )
                        {
                            CFInMailBox = 0;
                            LOG2_OK( "Consecutive frame %d has been sent",
                                     blockSTXCurrent );
                            nCs = can_getTicksTx()+TXTIMEOUT;
                            sTmin = can_getTicksTx()+st_FCConf_t_Tx.sTmin;
                            LOG2_OK( "Time stamp: %d", can_getTicksTx());
                            LOG2_OK( "Time stamp( _Stmin ): %d", sTmin );
                            LOG2_OK( "Time stamp( _N_Cs ): %d", nCs );
                            blockSTXCurrent++;
                            /* Checks if the number of CF has exceeded the
                             * configured BS
                             */
                            if ( blockSTXCurrent >= st_FCConf_t_Tx.blockSize )
                            {
                                TxState = en_pcktState_FFSentFCNotReceived;
                                nBs = can_getTicksTx()+TXTIMEOUT;
                                LOG2_OK( "Time stamp( N_Bs ): %d", nBs );
                                blockSTXCurrent = 0;
                            }
                        }
                        else if ( EndOfPck )
                        {
                            _nResult = en_nResult_messageCompleteTX;
                            _retval = en_nResult_messageCompleteTX;
                            can_resetTxLED();
                            LOG1_OK( "Message Complete!" );
                            blockSTXCurrent = 0;
                            TxState = en_pckState_FFNotSent;
                            /* shifting the current processing frame to the
                             *  next
                             */
                            p_schBufFrame[currentPckID] += endRemaining;
                            SN = 0;
                            numberMessages--;
                            currentPckID++;
                            frameType = en_frameType_fF;
                            if ( currentPckID == MAXNOPCK )
                            {
                                currentPckID = 0;
                            }
                            else
                            {
                                a_pckInfoTx[currentPckID].pckTxStatus
                                = en_PckStatus_txStatusLoaded;
                               // current_packet_id++;
                                a_pckInfoTx[currentPckID].pckTxStatus
                                = en_PckStatus_txStatusProccessing;
                            }
                            _resetPckInfoTx();
                        }
                        else if ( nAs < can_getTicksTx())
                        {
                            LOG2_OK( "Time out _N_As: %d", can_getTicksTx());
                            _nResult = en_nResult_timeoutAs;
                            _retval = _nResult;
                            blockSTXCurrent = 0;
                            can_resetTxLED();
                            TxState = en_pckState_FFNotSent;
                            numberMessages--;
                        }
                    }
                    else if ( nCs < can_getTicksTx())
                    {
                        LOG2_OK( "Time out _N_Cs: %d, %d", can_getTicksTx(),
                                 nCs );
                        _nResult = en_nResult_timeoutCs;
                        _retval = _nResult;
                        blockSTXCurrent = 0;
                        can_resetTxLED();
                        TxState = en_pckState_FFNotSent;
                        numberMessages--;
                    }
                    break;
                default:
                    break;
            }
            //retval = N_OK;
        }
        // if single message
        else if ( a_pckInfoTx[currentPckID].size <= 7
            && numberMessages > 0 )
        {
            LOG2_OK( "Sending single frame" );
            nAs = can_getTicksTx() + TXTIMEOUT;
            LOG2_OK( "Time stamp( _N_As ): %d", nAs );
            _retval = _writePck();
            if ( _retval == en_mailBox_noMailBox )
            {
                LOG2_OK( "Can not send the single frame" );
                _nResult = en_nResult_error;
                _retval = en_nResult_error;
            }
            else if ( _retval != en_mailBox_noMailBox
                && nAs >= can_getTicksTx())
            {
                LOG2_OK( "Single frame has been successfully sent!" );
                _retval = en_nResult_nOK;
                _nResult = en_nResult_nOK;
                _resetPckInfoTx();
            }
            else if ( nAs < can_getTicksTx())
            {
                LOG2_OK( "Time out _N_As: %d", can_getTicksTx());
                _nResult = en_nResult_timeoutAs;
                blockSTXCurrent = 0;
                can_resetTxLED();
                TxState = en_pckState_FFNotSent;

                numberMessages--;
            }
        }
    return _nResult;
}

/**
 * \brief     To handle the incoming frames
 *
 * \return    returns the status of receiving such a frame
 */
uint8_t _tpRcv()
{
    uint8_t _dLen;
    s_ringb_t* _p_dataLenRxBuff;
    s_ringb_t* _p_dataRxBuff;
    uint16_t _dataExist;
    en_nResultRx_t _retval;

    _p_dataLenRxBuff = tpRcv_getDataLengthRxBuff();
    _p_dataRxBuff = tpRcv_getDataRxBuff();
    _dataExist = ringb_pulla( _p_dataLenRxBuff, &_dLen );

    /* If there is no data in the ring buffer
     * check if the sate is en_innerState_sendingFC then try to
     * send a flow control
     * or if there is timeout then handle it
     */
    if ( _dataExist == 0 )
    {
        _retval = _handleDataNonExist();
    }
    /*
     * if there is data in the ring buffer then handle it
     */
    else
    {
        _retval = _handleDataExist( _dLen, _p_dataRxBuff );
    }
return _retval;
}

/**
 * \brief     To handle the incoming frames
 *
 * \return    returns the status of receiving such a frame
 */
uint8_t _isotpEntryRx()
{
    uint8_t _retval;
    uint8_t _localFrame[3];

    if ( iState == en_iState_sendingFC
        && oState == en_oState_multipleFrame )
    {
        _localFrame[0] = FCTYPE;     //Type|FS
        _localFrame[1] = st_FCConf_t_Rx.blockSize;     //BS
        _localFrame[2] = st_FCConf_t_Rx.sTmin;     //stmin
        //send the FC 3 bytes
        _retval = tpSnd_isoTpsend( st_FCConf_t_Rx.e_if,
                            _localFrame, st_FCConf_t_Rx.srcID, 3 );

        if ( _retval != en_mailBox_noMailBox )
        {
            LOG2_OK( "The flow control has been sent" );
            iState = en_iState_receivingCF;
        }
    }
    else
    {
        _retval = _tpRcv();
    }
return _retval;

}

/**
 * \brief     It is called to send the frame and it controls
 *            the sending of SF, CF.
 *
 * \return    The number of the mailbox that is used for transmission or
 *         en_mailBox_noMailBox if there is no empty mailbox.
 */
uint8_t _writePck()
{
    uint8_t _retval;
    uint16_t _remainingSize;
    uint8_t _frameSent[8];
    uint8_t _i = 0;
    uint8_t _lCount;
    _remainingSize = a_pckInfoTx[currentPckID].size -
        ( p_schBufFrame[currentPckID] - p_schBuf[currentPckID] );
    /* In the case of sending single message*/
    if ( a_pckInfoTx[currentPckID].size <= 7 )
    {
        _frameSent[0] = ( a_pckInfoTx[currentPckID].size );
        for ( _i = 0;  _i < a_pckInfoTx[currentPckID].size;
            _i++ )
        {
            _frameSent[_i+1] = ( *( p_schBufFrame[currentPckID]+_i ));
        }
         _retval = tpSnd_isoTpsend( a_pckInfoTx[currentPckID].e_if,
                             &_frameSent,
                             a_pckInfoTx[currentPckID].srcID,
                             a_pckInfoTx[currentPckID].size + 1 );
       if ( _retval != en_mailBox_noMailBox )
        {
            numberMessages--;
            currentPckID++;
        }
    }
    /* In the case of sending multiple frames*/
    else
    {
        if ( frameType == en_frameType_fF ) //for FF
        {
            uint16_t tmpv;
            _frameSent[0] = FFTYPE |
                ( a_pckInfoTx[currentPckID].size >> 8 );
            tmpv = a_pckInfoTx[currentPckID].size >> 8;
            tmpv = ( uint8_t )a_pckInfoTx[currentPckID].size;
            _frameSent[1] = ( uint8_t )a_pckInfoTx[currentPckID].size;
            for ( _lCount = 0;  _lCount < 6;  _lCount++ )
                _frameSent[_lCount + 2] = p_schBufFrame[currentPckID][_lCount];
            _retval = tpSnd_isoTpsend( a_pckInfoTx[currentPckID].e_if,
                                &_frameSent[0],
                                a_pckInfoTx[currentPckID].srcID, 8 );
            if ( _retval != en_mailBox_noMailBox )
            {
                a_pckInfoTx[currentPckID].mailBox = _retval;
                p_schBufFrame[currentPckID] += 6;
                a_pckInfoTx[currentPckID].pckTxStatus
                = en_PckStatus_txStatusProccessing;
                frameType = en_frameType_cF;
                fFSent = 1;
            }
        }
        /* In the case of sending consecutive frames*/
        else
        {
            /* check if the remaining size is more than 7 bytes
             *  ( more than one frame in standard addressing )
             */
            if ( _remainingSize > 7 )
            {
                if ( SN > MAXSN )
                    SN = -1;
                if ( fFSent == 1 )
                {
                    SN = 0;
                    fFSent = 0;
                }
                SN++;
                _frameSent[0] = CFTYPE | SN;
                for ( _lCount = 1; _lCount < 8; _lCount++ )
                    _frameSent[_lCount] = *( p_schBufFrame[currentPckID]
                                                         + _lCount - 1 );
                _retval = tpSnd_isoTpsend( a_pckInfoTx[currentPckID].e_if,
                                    _frameSent,
                                    a_pckInfoTx[currentPckID].srcID,
                                    8 );
                if ( _retval != en_mailBox_noMailBox )
                {
                    CFInMailBox = 1;
                    a_pckInfoTx[currentPckID].mailBox = _retval;
                    p_schBufFrame[currentPckID] += 7;
                    a_pckInfoTx[currentPckID].pckTxStatus =
                        en_PckStatus_txStatusProccessing;
                }
            }
            else
            {
                if ( SN > MAXSN )
                    SN = -1;
                SN++;
                _frameSent[0] = CFTYPE | SN;
                for ( _lCount = 0; _lCount < _remainingSize; _lCount++ )
                    _frameSent[_lCount+1] = *( p_schBufFrame[currentPckID] +
                        _lCount );
                _retval = tpSnd_isoTpsend( a_pckInfoTx[currentPckID].e_if,
                                    _frameSent,
                                    a_pckInfoTx[currentPckID].srcID,
                                    _remainingSize + 1 );
                if ( _retval != en_mailBox_noMailBox )
                {
                    EndOfPck = 1;
                    a_pckInfoTx[currentPckID].mailBox = _retval;
                    endRemaining = _remainingSize;
                }
            }
        }
    }
    return _retval;
}

/**
 * \brief     To reset all the static variables related to Rx
 *
 * \return    void
 */
void _resetSCHRx()
{
    blockSize = 3;
    nBr = 1;
    nCr = 1;
    nAr = 1;
    tTimeout = en_typeTimeout_AR;
    oState = en_oState_start;
    iState = en_iState_receivingFF;
    endMessage = 0;
    timeBefore = 1;
    blockSizeRXCurrent = 0;
}

/**
 * \brief     To reset the static variables related to the Tx process
 *
 * \return    void
 */
void _resetSCHTx()
{
    flag = 0;
    pckId = 0;
    totPckSize = 0;
    currentPckID = 0;
    blockSTXCurrent = 0;
    CFInMailBox = 0;
    EndOfPck = 0;
    TxState = en_pckState_FFNotSent;
    numberMessages = 0;
    SN = 0;
    CFSent = 0;
    fCReceived = 0;
}

/**
 * \brief     To reset the info which is related to such a packet
 *            in the packet_info_Tx struct
 *
 * \return    void
 */
void _resetPckInfoTx()
{
    uint8_t _lCount = 0;
    totPckSize = a_pckInfoTx[0].size;
    for ( _lCount = 0; _lCount < pckId; _lCount++ )
    {
        a_pckInfoTx[_lCount] = a_pckInfoTx[_lCount+1];
        p_schBuf[_lCount] = p_schBuf[_lCount+1];
        p_schBufFrame[_lCount] = p_schBufFrame[_lCount+1];
    }
    currentPckID--;
    pckId--;
}

/**
 * \brief     This function is called from tprecv() when there is no
 *            data in the ring buffer
 *            it goes into the state machine of the reception process
 * \return    en_NResultRx_t it returns the result according to this
 *            enumeration
 */
en_nResultRx_t _handleDataNonExist( void )
{

    int8_t _ret = -1;
    uint8_t _localFrame[3];
    if ( iState == en_iState_sendingFC )
     {
         //tpReceiveRoute( 0, 1, FC_callback );
         LOG2_OK( "Trying to send the Flow control" );
         _localFrame[0] = FCTYPE;     //Type|FS
         _localFrame[1] = st_FCConf_t_Rx.blockSize;     //BS
         _localFrame[2] = st_FCConf_t_Rx.sTmin;     //stmin
         _ret = tpSnd_isoTpsend( st_FCConf_t_Rx.e_if, _localFrame,
                          st_FCConf_t_Rx.srcID, 3 ); //send the FC
         if ( _ret != en_mailBox_noMailBox && nAr >= can_getTicksRx())
         {
             LOG2_OK( "The flow control has been sent" );
             LOG2_OK( "Time stamp( N_Ar ): %d", nAr );
             iState = en_iState_receivingCF;
             LOG2_OK( "Trying to receive a consecutive frame" );
             nCr = can_getTicksRx() + RXTIMEOUT;
             LOG2_OK( "Time stamp( N_Cr ): %d", nCr );
             tTimeout = en_typeTimeout_CR;
         }
         else if ( nAr < can_getTicksRx())
         {
             can_resetRxLED();
             LOG2_OK( "Time out( N_Ar ): %d", nAr );
             nResultRx = en_nResultRx_timeoutAr;
             oState = en_oState_start;
         }
     }
    if ( oState != en_oState_start )
    {
         if ( tTimeout == en_typeTimeout_AR )
         {
             timeBefore = 1;
             if ( nAr < can_getTicksRx())
             {
                 can_resetRxLED();
                 LOG2_OK( "Time out( N_Ar ): %d", can_getTicksRx());
                 nResultRx = en_nResultRx_timeoutAr;
                 oState = en_oState_start;
             }
         }
         else if ( tTimeout == en_typeTimeout_BR )
         {
             timeBefore = 1;
             if ( nBr < can_getTicksRx())
             {
                 can_resetRxLED();
                 LOG2_OK( "Time out( N_Br ): %d", can_getTicksRx());
                 nResultRx = en_nResultRx_timeoutBr;
                 oState = en_oState_start;
             }
         }
         else if ( tTimeout == en_typeTimeout_CR )
         {
             timeBefore = 1;
             if ( nCr < can_getTicksRx())
             {
                 can_resetRxLED();
                 LOG2_OK( "Time out( N_Cr ): %d", can_getTicksRx());
                 nResultRx = en_nResultRx_timeoutCr;
                 oState = en_oState_start;
             }
         }
    }
    return nResultRx;
}

/**
 * \brief     This function is called from tprecv() when there is data in
 *            the ring buffer
 *            it goes into the state machine of the reception process
 * \param     d_len         The length of the frame received
 * \param     data_rx_buff  The address of the data ring buffer
 * \return    en_NResultRx_t it returns the result according to this
 *            enumeration
 */
en_nResultRx_t _handleDataExist( uint8_t d_len, s_ringb_t* data_rx_buff)
{
    en_canIf_t _eIf;
    int8_t _ret = -1;
    uint8_t _localFrame[3];
    uint8_t _nibble2;
    uint16_t _cId;
    uint16_t _ploadLen;
    uint8_t* _p_pload;
    uint8_t _dBuff[10];
    en_nResultRx_t _retval;

    timeBefore = 0;
    ringb_pull( data_rx_buff, _dBuff, d_len );
    _p_pload = &_dBuff[0];
    _ploadLen = d_len - 2;
    _cId = _dBuff[d_len - 1];
    _cId = ( _cId << 8 );
    _cId = _cId | ( _dBuff[d_len - 2] );
    /*
     * To get the highest nibble
     */
    _nibble2 = ( *_p_pload ) & HNIBBLEMASK;

    if ( _nibble2 == FCTYPE && _cId ==
        a_pckInfoTx[currentPckID].destID )
    {
        LOG2_OK( "Calling the flow control callback function" );
        _retval = tpSnd_fC_callback( _p_pload );
    }
    else if ( st_FCConf_t_Rx.destID == _cId )
    {
        switch( oState )
        {
            case en_oState_start:
                _retval = _handleStartState( _nibble2,
                                        _cId,
                                        _ploadLen,
                                        _p_pload);
                break;
            case en_oState_singleFrame:
                LOG2_OK( "Handling a single frame" );
                _retval = tpRcv_handleSingleFrame( _eIf, _cId, _p_pload,
                                                 _ploadLen );
                if ( _retval == en_nResultRx_okRx )
                {
                    LOG2_OK( "The single frame has been successfully stored" );
                    endMessage = 1;
                    oState = en_oState_start;
                }
                break;
            case en_oState_multipleFrame:
                    _retval = _handleMultipleFrameState( _nibble2,
                                                    _cId,
                                                    _ploadLen,
                                                    _p_pload);
                    break;
            default:
                _retval = en_nResultRx_errorRx;
                can_resetRxLED();
                oState = en_oState_start;
                break;

        }
    }
    return _retval;
}

/**
 * \brief     To handle the Start state
 * \param     _nibble2         The highest nibble in PCI
 * \param     _cId             The ID of Sender
 * \param     _ploadLen        the length of the frame
 * \param     _p_pload         pointer to the data
 * \return    en_NResultRx_t it returns the result according to this
 *            enumeration
 */
en_nResultRx_t _handleStartState( uint8_t _nibble2, uint16_t _cId,
                             uint16_t _ploadLen, uint8_t* _p_pload)
{
    en_canIf_t _eIf;
    int8_t _ret = -1;
    uint8_t _localFrame[3];
    uint8_t _dBuff[10];
    en_nResultRx_t _retval;

    can_setRxLED();
    can_resetTicksRx();
    endMessage = 0;
    blockSizeRXCurrent = 0;
    if ( _nibble2 == 0 )
    {
        oState = en_oState_singleFrame;
        LOG2_OK( "Receiving a single frame" );
        _retval = tpRcv_handleSingleFrame( _eIf, _cId,
                                         _p_pload, _ploadLen );
        if ( _retval == en_nResultRx_okRx )
        {
            //LOG_HEXDUMP( pc_pload, pc_ploadLen );
            LOG2_OK( "The single frame has been received" );
            oState = en_oState_start;
        }

    }
    else if ( _nibble2 == FFTYPE )  //FF
    {
        _retval = _handleFFState( _nibble2,
                                 _cId,
                                 _ploadLen,
                                 _p_pload);
    }
    else if ( _nibble2 == CFTYPE )  //Consecutive frame
    {
        if ( iState != en_iState_receivingCF ||
            iState != en_iState_sendingFC )
        {
            can_resetRxLED();
            LOG2_OK( "Unexpected PDU!" );
            _retval = en_nResultRx_unexpPDURx;
            oState = en_oState_start;
        }
        else if ( nCr >= can_getTicksRx())
        {
            can_resetRxLED();
            LOG2_OK( "Receiving the consecutive frame!" );
            oState = en_oState_multipleFrame;
            iState = en_iState_receivingCF;
            _retval = tpRcv_handleCF( _eIf, _cId,
                                                  _p_pload,
                                                  _ploadLen );
        }
        else if ( nCr < can_getTicksRx())
        {
            can_resetRxLED();
            LOG2_OK( "Time out( N_Cr ): %d", nCr );
            nResultRx = en_nResultRx_timeoutCr;
            oState = en_oState_start;
        }
    }
    else if ( _nibble2 == FCTYPE && _cId  == st_FCConf_t_Tx.srcID )
    {
        LOG2_OK( "Calling the flow control callback function" );
        _retval = tpSnd_fC_callback( _p_pload );
    }
    else
    {
        can_resetRxLED();
        _retval = en_nResultRx_unexpPDURx;
        LOG2_OK( "Unexpected PDU" );
    }
    return _retval;
}


/**
 * \brief     To handle the multiple frame state
 * \param     _nibble2         The highest nibble in PCI
 * \param     _cId             The ID of Sender
 * \param     _ploadLen        the length of the frame
 * \param     _p_pload         pointer to the data
 * \return    en_NResultRx_t it returns the result according to this
 *            enumeration
 */
en_nResultRx_t _handleMultipleFrameState( uint8_t _nibble2, uint16_t _cId,
                             uint16_t _ploadLen, uint8_t* _p_pload)
{
    en_canIf_t _eIf;
    int8_t _ret = -1;
    uint8_t _localFrame[3];
    uint8_t _dBuff[10];
    en_nResultRx_t _retval;
    switch( iState )
    {
        case en_iState_receivingFF:
            LOG2_OK( "Handling the first frame!" );
            _retval = tpRcv_handleFF( _eIf, _cId,
                                            _p_pload,
                                            _ploadLen );
            if ( _retval == en_nResultRx_buffOvFlwRx )
            {
                can_resetRxLED();
                LOG2_OK( "Over flow!" );
                _localFrame[0] = 0x32;     //Type|FS overflow
                //send the FC
                _ret = tpSnd_isoTpsend( st_FCConf_t_Rx.e_if,
                                 _localFrame,
                                 st_FCConf_t_Rx.srcID, 3 );

                oState = en_oState_start;
            }
            else if ( _retval == en_nResultRx_okRx )
                iState = en_iState_sendingFC;
            break;
        case en_iState_sendingFC:
            LOG2_OK( "Sending the flow control" );
            //Type|FS
            _localFrame[0] = FCTYPE;
            //BS
            _localFrame[1] = st_FCConf_t_Rx.blockSize;
            //stmin
            _localFrame[2] = st_FCConf_t_Rx.sTmin;
            //send the FC
            _retval = tpSnd_isoTpsend( st_FCConf_t_Rx.e_if,
                                _localFrame,
                                st_FCConf_t_Rx.srcID, 3 );
            if ( _retval != en_mailBox_noMailBox
                && nAr >= can_getTicksRx())
            {
                LOG2_OK( "Time stamp( N_Ar ): %d",
                         can_getTicksRx());
                LOG2_OK( "The flow control has been \
                          successfully sent" );
                iState = en_iState_receivingCF;
                nCr = can_getTicksRx()+RXTIMEOUT;
                LOG2_OK( "Time stamp( N_Cr ): %d", nCr );
                LOG2_OK( "Receiving the Consecutive frame" );
                tTimeout = en_typeTimeout_CR;
            }
            else if ( nAr < can_getTicksRx())
            {
                can_resetRxLED();
                LOG2_OK( "Time out( N_Ar ): %d", nAr );
                nResultRx = en_nResultRx_timeoutAr;
                oState = en_oState_start;
            }
            break;
        case en_iState_receivingCF:
            if ( nCr >= can_getTicksRx())
            {
                LOG2_OK( "Time stamp( N_Cr ): %d",
                         can_getTicksRx());
                LOG2_OK( "Handling the consecutive frame" );
                _retval = tpRcv_handleCF( _eIf,
                                                 _cId,
                                                 _p_pload,
                                                 _ploadLen );
                //counter++;

             //  if ( ( counter*7+6 )>data_buf_length )
                  // pc_ploadLen = pc_ploadLen-( ( counter*7+6 )-data_buf_length )-1;
               // LOG_HEXDUMP( pc_pload, pc_ploadLen );
                if ( _retval == en_nResultRx_wrongSNRx )
                {
                    _resetSCHRx();
                    LOG2_OK( "Wrong seqNo" );
                }
                if ( _retval == en_nResultRx_okRx )
                {

                    nCr = can_getTicksRx() + RXTIMEOUT;
                    LOG2_OK( "Time stamp( N_Cr ): %d", nCr );
                    LOG2_OK( "The consecutive frame has been \
                        stored no %d", blockSizeRXCurrent );
                    tTimeout = en_typeTimeout_CR;
                    oState = en_oState_multipleFrame;
                    iState = en_iState_receivingCF;
                    blockSizeRXCurrent++;
                    if ( blockSizeRXCurrent >= st_FCConf_t_Rx.blockSize )
                    {
                        iState = en_iState_sendingFC;
                        blockSizeRXCurrent = 0;
                    }
                }
                else if ( _retval == en_nResultRx_messageComplete )
                {
                    can_resetRxLED();
                    //GPIO_ResetBits( GPIOD, GPIO_Pin_15 );
                    LOG2_OK( "Message complete!" );
                    endMessage = 1;
                    oState = en_oState_start;
                    iState = en_iState_receivingFF;
                }
                else
                    return _retval;
            }
            else
            {
                LOG2_OK( "Time stamp( N_Cr ): %d", nCr );
                nResultRx = en_nResultRx_timeoutCr;
                oState = en_oState_start;
            }
          break;
        default:
            _retval = en_nResultRx_errorRx;
            can_resetRxLED();
            oState = en_oState_start;
            break;
    }
    return _retval;
}


/**
 * \brief     To handle the FF state
 * \param     _nibble2         The highest nibble in PCI
 * \param     _cId             The ID of Sender
 * \param     _ploadLen        the length of the frame
 * \param     _p_pload         pointer to the data
 * \return    en_NResultRx_t it returns the result according to this
 *            enumeration
 */
en_nResultRx_t _handleFFState( uint8_t _nibble2, uint16_t _cId,
                             uint16_t _ploadLen, uint8_t* _p_pload)
{
    en_canIf_t _eIf;
    int8_t _ret = -1;
    uint8_t _localFrame[3];
    uint8_t _dBuff[10];
    en_nResultRx_t _retval;
    LOG2_OK( "Receiving the First frame" );
    nBr = can_getTicksRx() + RXTIMEOUT;
    LOG2_OK( "Time stamp( N_Br+timeout ): %d", nBr );
    tTimeout = en_typeTimeout_BR;
    oState = en_oState_multipleFrame;
    iState = en_iState_receivingFF;
    pckInfoRx.arbitrationId = _cId;
    pckInfoRx.e_if = _eIf;
    pckInfoRx.size = _ploadLen;
    _retval = tpRcv_handleFF( pckInfoRx.e_if,
                                    pckInfoRx.arbitrationId
                                    , _p_pload, _ploadLen );

    if ( _retval == en_nResultRx_buffOvFlwRx )
    {
        LOG2_OK( "Can not be stored ( Over flow )" );
        can_resetRxLED();
        oState = en_oState_start;
        iState = en_iState_receivingFF;
        return _retval;
    }
    else if ( _retval == en_nResultRx_okRx
        && nBr >= can_getTicksRx())
    {

        //LOG_HEXDUMP( pc_pload, pc_ploadLen );
        //printf( "\n\r" );
        LOG2_OK( "Time stamp( N_Br ): %d", can_getTicksRx());
        LOG2_OK( "The first frame has been successfully stored"
                );
        iState = en_iState_sendingFC;
        _localFrame[0] = FCTYPE;     //Type|FS
        _localFrame[1] = st_FCConf_t_Rx.blockSize;     //BS
        _localFrame[2] = st_FCConf_t_Rx.sTmin;     //stmin
        nAr = can_getTicksRx() + RXTIMEOUT;
        LOG2_OK( "Time stamp( N_Ar+timeout ): %d", nAr );
        LOG2_OK( "Sending the flow control" );
        tTimeout = en_typeTimeout_AR;
        _ret = tpSnd_isoTpsend( st_FCConf_t_Rx.e_if, _localFrame,
                         st_FCConf_t_Rx.srcID, 3 );
        if ( _ret != en_mailBox_noMailBox &&
            nAr >= can_getTicksRx())
        {
            LOG2_OK( "Time stamp( N_Ar ): %d", can_getTicksRx());
            LOG2_OK( "The flow control has been sent" );
            iState = en_iState_receivingCF;

            nCr = can_getTicksRx() + RXTIMEOUT;
            LOG2_OK( "Time stamp( N_Cr+timeout ): %d", nCr );
            LOG2_OK( "Receiving the consecutive frames" );
            tTimeout = en_typeTimeout_CR;
        }
        else if ( nAr < can_getTicksRx())
        {
            can_resetRxLED();
            LOG2_OK( "Time out( N_Ar ): %d", nAr );
            nResultRx = en_nResultRx_timeoutAr;
            oState = en_oState_start;
        }
    }
    else if ( nBr < can_getTicksRx())
    {
        can_resetRxLED();
        LOG2_OK( "Time out( N_Br ): %d", nBr );
        nResultRx = en_nResultRx_timeoutBr;
        oState = en_oState_start;
    }
    return _retval;
}
/* --- EOF ----------------------------------------------------------------- */









