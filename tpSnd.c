/* --- License ------------------------------------------------------------- */

/* --- Module Description -------------------------------------------------- */
/**
 * \file    tpSend.c

/* --- Includes ------------------------------------------------------------ */
#include "tpSnd.h"

/* --- Macro Definitions --------------------------------------------------- */

/* --- Type Definitions ---------------------------------------------------- */

/* --- Variables ----------------------------------------------------------- */
/*
 * this flag to indicate if I have received a FC frame or not
 */
static uint8_t fCReceived;
st_FCConf_t g_fcConf;
static fCRxFlag = 0;

/* --- Local Functions Declaration ----------------------------------------- */

en_mailBox_t _sendSinglemessage( en_canIf_t e_if,uint8_t *pload,
                           uint32_t arbitration_id, uint8_t size );
/* --- Global Functions Definition ----------------------------------------- */


en_mailBox_t tpSnd_isoTpsend( en_canIf_t e_if, uint8_t *pload, uint32_t arbitration_id,
           uint8_t size )
{
    en_mailBox_t _retval;
    _retval = _sendSinglemessage( e_if, pload, arbitration_id, size );
    return _retval;
}
en_nResultTx_t tpSnd_fC_callback( uint8_t* pc_pload )
{
    int8_t _retval = 0;
    en_nResultTx_t _nResult;
    //make sure it is a FC
    if ((( pc_pload[0] ) >> 4 ) == 3)
    {
        fCReceived = 1;
        //check the three states of FS
        if ((( pc_pload[0] ) & 0xF ) == 0)
        {
            //en_fSInf_cTS then store the configurations
            g_fcConf.blockSize = pc_pload[1];
            //fc_conf.BS = 1;
            if (pc_pload[2] >= 0 && pc_pload[2] <= 0x7F)
                /*
                 * x10 if sysclk is interrupted every 100usec,
                 * x100 if sysclk is interrupted every 10usec
                 */
                g_fcConf.sTmin = pc_pload[2] * 100;
            else if (pc_pload[2] >= 0xF1 && pc_pload[2] <= 0xF9)
                /* *10 if ticks interrupted every 10usec*/
                g_fcConf.sTmin = ( pc_pload[2] - 0xF0 ) * 10;
            else
                _nResult = en_nResultTx_errorTx;
            g_fcConf.fS = en_fSInf_cTS;    // means FS = en_fSInf_cTS
            _nResult = en_nResultTx_okTx;
        }
        else if ((( pc_pload[0] ) & 0xF ) == 0x1)    // FS = wait
        {
            g_fcConf.fS = en_fSInf_wait;
        }
        else if ((( pc_pload[0] ) & 0xF ) == 0x2)    // FS = overflow
        {
            _nResult = en_nResultTx_buffOvFlwTx;
            g_fcConf.fS = en_fSInf_ovFlw;
        }
        else
        {
            _nResult = en_nResultTx_invalidFSTx;
        }
    }
    else
        _retval = en_nResultTx_okTx;
    g_fCRx = fCReceived;
    fCRxFlag = 1;
    _retval = _nResult;
    return _nResult;
}

st_FCConf_t tpSnd_getFC()
{
    return g_fcConf;
}

uint8_t tpSnd_getFCRxFlag()
{
    return fCRxFlag;
}
void tpSnd_resetFCRxFlag()
{
    fCRxFlag = 0;
}
/* --- Local Functions Definition ----------------------------------------- */
/**
 * \brief     sending one isotp frame
 *
 * \param     e_if the can interface
 * \param     *pload the payload of the message
 * \param     arbitration_id  CAN ID which should be used for a transmission.
 * \param     size the size of the message
 * \return   The number of the mailbox that is used for transmission or
 *           CAN_TxStatus_NoMailBox if there is no empty mailbox
 */
en_mailBox_t _sendSinglemessage( en_canIf_t e_if, uint8_t *pload,
                           uint32_t arbitration_id, uint8_t size )
{
    en_mailBox_t _retval;
    uint8_t _a_sentFrame[8];
    int _lCount = 0;
    /*
     * the 1st nibble is for the length = 1 and the other nibble
     * for type = 0 for SF
     * Filling in the frame by the PCI and the data
     */
    for ( _lCount = 0; _lCount < size; _lCount++ )
    {
        _a_sentFrame[_lCount] = pload[_lCount];
    }
    _retval = can_send ( e_if, arbitration_id, _a_sentFrame, size );


    return _retval;
}


/* --- EOF ----------------------------------------------------------------- */

