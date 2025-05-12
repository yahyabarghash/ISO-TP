/* --- License ------------------------------------------------------------- */

/* --- Module Description -------------------------------------------------- */
/**
 * \file    tpReceive.c

/* --- Includes ------------------------------------------------------------ */

#include "tpRcv.h"
#include "can.h"
#include "logger.h"

/* --- Macro Definitions --------------------------------------------------- */
#define LOGGER_ENABLE 0
#define LOGGER_LEVEL 1

#define MAXNOPCK 40
#define MAXPCKBUFSIZE 4096
#define MAXFRMBUFSIZE 1024

/* --- Type Definitions ---------------------------------------------------- */

/* --- Variables ----------------------------------------------------------- */

static st_isoTpConf_t isotp_conf;

//static callback_rxmain __can_callback_main;

static uint16_t currentFrameNumber;
static uint16_t startIndexFF;
static uint16_t dataBufLength;
static uint8_t  fFRecv = 0;
static int8_t   seqNumber = 0;
static uint16_t currentPckId;
static uint8_t a_dataBuff[MAXPCKBUFSIZE];
static s_ringb_t dataRingb;
static ringb_atom_t a_dataRxBuff[MAXFRMBUFSIZE];
static s_ringb_t dataRxRingb;
static ringb_atom_t a_dataLenRxBuff[MAXFRMBUFSIZE];
static s_ringb_t dataLenRxRingb;
static st_pckInfRx_t a_pckInfoRx[MAXNOPCK];

/* --- Local Functions Declaration ----------------------------------------- */


void _addNewPck( uint16_t length );


void _resetTPRecv( void );


void _resetCurrentFrameNumber( void );

/* --- Global Functions Definition ----------------------------------------- */
en_nResultRx_t tpRcv_handleFF( en_canIf_t e_if,
                                      uint16_t c_id,
                                      uint8_t* pc_pload,uint16_t pc_ploadLen )
{
    en_nResultRx_t _retval;
    uint16_t _fF_DL;
    _fF_DL = ( pc_pload[0] & 0x0F );
    _fF_DL = ( _fF_DL << 8 ) | pc_pload[1];
    dataBufLength = _fF_DL;
    seqNumber = -1;
    startIndexFF = currentFrameNumber;
    if (_fF_DL > MAXBUFFSIZE - currentFrameNumber)
    {
        _retval = en_nResultRx_buffOvFlwRx;
        return _retval;
    }
    //to store the data into the buffer
//  for ( uint8_t count = 0;count<6;count++ )
//              data_buff[current_frame_number++] = pc_pload[count+2];
    ringb_push( &dataRingb, &pc_pload[2], 6 );
    currentFrameNumber += 6;
    fFRecv = 1;
    _retval = en_nResultRx_okRx;
    return _retval;
}

en_nResultRx_t tpRcv_handleCF( en_canIf_t e_if, uint16_t c_id,
                                            uint8_t* pc_pload,
                                            uint16_t pc_ploadLen )
{
    en_nResultRx_t _retval = en_nResultRx_okRx;
    uint8_t _nibble1;
    uint8_t _step = 7;
    _nibble1 = ( pc_pload[0] & 0x0F );
    if ( seqNumber == 15 )
        seqNumber = -1;
    if ( fFRecv == 1 )
    {
        seqNumber = 0;
        fFRecv = 0;
    }
    if ( _nibble1 != seqNumber + 1 )
    {
        _retval = en_nResultRx_wrongSNRx;
    }
    if ( _retval != en_nResultRx_wrongSNRx )
    {
        seqNumber++;
        if (currentFrameNumber + 7 >= dataBufLength + startIndexFF
            && _retval != en_nResultRx_wrongSNRx )
        {
            _step = dataBufLength - ( currentFrameNumber - startIndexFF );
            _retval = en_nResultRx_messageComplete;
        }
        //  for ( uint8_t count = 0;count<step;count++ )
        //              data_buff[current_frame_number+count] = pc_pload[count+1];
        ringb_push( &dataRingb, &pc_pload[1], _step );
        currentFrameNumber += _step;
        if ( _retval != en_nResultRx_messageComplete )
            _retval = en_nResultRx_okRx;
        if (currentFrameNumber > dataBufLength + startIndexFF)
        {
            seqNumber = 0;
            //current_frame_number = 0;
            //LOG_HEXDUMP( data_buf, data_buf_length );
            _retval = en_nResultRx_messageComplete;
        }
        if ( _retval == en_nResultRx_messageComplete )
            _addNewPck( dataBufLength );
    }
    return _retval;
}

en_nResultRx_t tpRcv_handleSingleFrame( en_canIf_t e_if, uint16_t c_id,
                                       uint8_t* pc_pload,
                                       uint16_t pc_ploadLen )
{
    en_nResultRx_t _retval;
    uint8_t _nibble1;
    //the length
    _nibble1 = *pc_pload & 0x0F;
    if (_nibble1 == 0)
    {
        //ignore the message
        _retval = en_nResultRx_okRx;
    }
    if (isotp_conf.addType == CANTP_STANDARD && _nibble1 > 7)
    {
        //Error in SF_DL
        return en_nResultRx_errorRx;
    }
    else if (isotp_conf.addType == CANTP_STANDARD && _nibble1 < 7)
    {
        if (currentFrameNumber + _nibble1
            <= MAXPCKBUFSIZE - currentFrameNumber)
        {
            ringb_push( &dataRingb, &pc_pload[1], _nibble1 );
            currentFrameNumber =+ _nibble1;
            _addNewPck( _nibble1 );
            _retval = en_nResultRx_okRx;
        }
        else
            _retval = en_nResultRx_buffOvFlwRx;
    }
    else
        _retval = en_nResultRx_errorRx;
    return _retval;
}

void tpRcv_buffinit()
{
    ringb_init( &dataRingb, a_dataBuff, MAXPCKBUFSIZE );
    ringb_init( &dataRxRingb, a_dataRxBuff, MAXFRMBUFSIZE );
    ringb_init( &dataLenRxRingb, a_dataLenRxBuff, MAXFRMBUFSIZE );
}

st_isoTpConf_t tpRcv_getIsoTPconf( void )
{
    return isotp_conf;
}

/*this callback function to store the frames in the ring buffer */
void tpRcv_callbackBufferSave( en_canIf_t e_if, uint16_t c_id, uint8_t* pc_pload,
                      uint16_t pc_ploadLen )
{
    uint16_t _counter = 0;
    uint8_t _a_dataArr[10];
    for ( _counter = 0; _counter < pc_ploadLen; _counter++ )
    {
        _a_dataArr[_counter] = *( pc_pload + _counter );
    }
    //store the low byte first of c_id
    _a_dataArr[_counter] = (uint8_t) ( c_id & 0x00FF );
    _a_dataArr[_counter + 1] = (uint8_t) ( c_id >> 8 );
    /*
     * one buffer to store the data with c_id and the other
     * to store the length of this data
     */
    ringb_push( &dataRxRingb, &_a_dataArr[0], pc_ploadLen + 2 );
    ringb_pusha( &dataLenRxRingb, pc_ploadLen + 2 );
    LOG_HEXDUMP(_a_dataArr,10);
    LOG1_OK(
        "Data has been fetched into the buffer in time stampTx :\
         %d and Rx: %d ",
        can_getTicksTx(), can_getTicksRx() );
}

s_ringb_t *tpRcv_getDataLengthRxBuff()
{
    return &dataLenRxRingb;
}

s_ringb_t* tpRcv_getDataRxBuff()
{
    return &dataRxRingb;
}

uint16_t tpRcv_recvPck( uint8_t *dptr )
{
    uint16_t _retval = 0;
    uint8_t _a_arr[100];
    uint8_t _lCount;
    if ( currentFrameNumber > 0 )
    {
        _retval = ringb_pull( &dataRingb, dptr, a_pckInfoRx[0].size );
        for ( _lCount = 0; _lCount < 100; _lCount++ )
            _a_arr[_lCount] = dptr[_lCount];
        currentFrameNumber -= a_pckInfoRx[0].size;
        //shifting the packet_info_rx by
        for ( _lCount = 0; _lCount < currentPckId; _lCount++ )
        {
            a_pckInfoRx[_lCount] = a_pckInfoRx[_lCount + 1];
        }

        currentPckId--;
    }
    return _retval;
}

/* --- Local Functions Definition ----------------------------------------- */
/**
 * \brief     This function is called when successfully storing the message
 *            to add the packet
 *            into the packet_info_rx to save the information of the received
 *            packet
 * \param     length the length of the packet
 * \return    void
 */
void _addNewPck( uint16_t length )
{
    a_pckInfoRx[currentPckId].pckId = currentPckId;
    a_pckInfoRx[currentPckId].size = length;
    a_pckInfoRx[currentPckId].p_loc = &a_dataBuff[currentFrameNumber
                                                        - length];
    currentPckId++;
    if ( currentPckId > MAXNOPCK )
        currentPckId = 0;
}

/**
 * \brief     This function is to reset the static variables
 * \param     void
 * \return    void
 */
void _resetTPRecv()
{
    currentFrameNumber = 0;
    startIndexFF = 0;
    dataBufLength = 0;
    seqNumber = 0;
    currentPckId = 0;
}

/**
 * \brief     it reset the current frame number to 0
 * \return    void
 */
void _resetCurrentFrameNumber()
{
    currentFrameNumber = 0;
}
/* --- EOF ----------------------------------------------------------------- */
