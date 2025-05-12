/* --- License ------------------------------------------------------------- */


/* --- Module Description -------------------------------------------------- */

/* --- Includes ------------------------------------------------------------ */

#include "main.h"
#include "logger.h"

/* --- Macro Definitions --------------------------------------------------- */
#define LOGGER_ENABLE   1
#define LOGGER_LEVEL    3
#define BUFFERSIZE     114

/* --- Type Definitions ---------------------------------------------------- */


/* --- Variables ----------------------------------------------------------- */

const uint16_t  LEDS = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
const uint16_t  LED[4] = {GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15};
const uint16_t  USER_BUTTON = GPIO_Pin_0;

static uint32_t _timeFinish;
static uint32_t _timeFinish2;

/* --- Local Functions Declaration ----------------------------------------- */

void _setLed( uint8_t state );

void _initMain(void);

void _initAPItest();

void _genRand( uint8_t* arr, uint16_t length );

void _prepareDataWithCRC( uint8_t a_arr[], uint16_t len );

uint8_t _checkData( uint16_t length, uint8_t *ptr );

/* --- Global Functions Definition ----------------------------------------- */
int main()
{
    uint8_t         _currentButtonStatus = Bit_RESET;
    uint8_t         _lastButtonStatus = Bit_RESET;
    uint8_t         _a_paLoad[1000];
    uint8_t         _a_temppa_load[1000];
    uint32_t        _NumberOfMessages1 = 0;
    uint32_t        _NumberOfMessages2 = 0;
    uint32_t        _NumberErr = 0;
    uint8_t         _a_darr[1000];
    uint16_t        _length;
    uint8_t         _temp;
    uint8_t         _flag1 = 0;
    uint8_t         _flag2 = 1;
    uint8_t         _flag3 = 0;
    uint8_t         _flag4 = 1;
    uint16_t        _len;
    uint32_t        _delayValue;
    uint8_t         _s_id = 1;
    uint8_t         _d_id = 0;
    en_nResultRx_t  _rxStatus = -1;
    en_nResult_t    _txStatus = -1;
    uint16_t        _lCount=0;

    SystemInit();
    uart_init();
    _initAPItest();
    _initMain();
    tpRcv_buffinit();
    pktSch_initTp();
    pktSch_tpRxConf( 6, 5, _s_id, _d_id, en_canIf_iF0 );

    do
    {
        _currentButtonStatus = GPIO_ReadInputDataBit( GPIOA, USER_BUTTON );
        if ( _lastButtonStatus !=  _currentButtonStatus  )
        {
            _delayValue = can_getTicksTx() + 20000; //for 20msec delay
            while( _delayValue < can_getTicksTx());
        }

        /* each success or pressing the user button */
        if (((( _lastButtonStatus !=  _currentButtonStatus &&
                _currentButtonStatus !=  RESET )
                || _txStatus == en_nResult_messageCompleteTX )
                && _NumberOfMessages1 < 1000 ))
        {
            _len = 250;
        // genRand( pa_load );
            if ( _flag2 == 1 )
                can_resetTicksGeneral();
            _flag2 = 0;
            if ( _NumberOfMessages1 == 999 )
                _flag1 = 1;
            _prepareDataWithCRC( _a_paLoad, _len );
            for ( _lCount = 0; _lCount < _len; _lCount++ )
            {
                _a_temppa_load[_lCount] = ( uint8_t )( _a_paLoad[_lCount] );
            }

            pckSch_sendPck( en_canIf_iF0, _a_temppa_load, _s_id, _d_id,
                               _len );
            _NumberOfMessages1++;
            LOG_OK( "Sending message no. %d ", _NumberOfMessages1 );

        }
        else if ( _flag1 == 1 )
        {
            _timeFinish = can_getTicksGeneral();
            _flag1 = 0;
        }

        if ( _rxStatus == en_nResultRx_messageComplete )
        {
            _length =  tpRcv_recvPck( _a_darr );
            _temp = _checkData( _length, _a_darr );
            _NumberOfMessages2++;
            if ( _flag4 == 1 )
                can_resetTicksGeneral();
            _flag4 = 0;
            if ( _NumberOfMessages2 == 999 )
                _flag3 = 1;
            if ( _temp == 1 )
            {
                LOG_OK( "CRC OK for message number %d", _NumberOfMessages2 );
                _temp = 1;
            }
            else
            {
                LOG_OK( "CRC Not OK for message number %d", _NumberOfMessages2 );
                _NumberErr++;
            }
            LOG_OK( "Number of errors %d", _NumberErr );
        }
        else if ( _flag3 == 1 )
        {
            _timeFinish2 = can_getTicksGeneral();
            _flag3 = 0;
        }

        pktSch_isotpEntry( &_txStatus, &_rxStatus);
        _lastButtonStatus = _currentButtonStatus;
    }while( 1 );
}

/* --- Local Functions Definition ----------------------------------------- */
/**
 * \brief     to turn on specific led
 *
 * \param     state    the number led
 * \return    void
 */
void _setLed( uint8_t state )
{
    GPIO_ResetBits( GPIOD, LEDS );
    GPIO_SetBits( GPIOD, LED[state] );
}

/**
 * \brief     to initialize the can and includes the configuration of the filter and which pin used for can Tx and Rx
 * \param     void
 * \return    void
 */
void _initMain()
{
    //===Configuration for CAN====

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );

    GPIO_PinAFConfig( GPIOD, GPIO_PinSource0, GPIO_AF_CAN1 );
    GPIO_PinAFConfig( GPIOD, GPIO_PinSource1, GPIO_AF_CAN1 );

    GPIO_InitTypeDef GPIO_InitStructureCAN;

    GPIO_InitStructureCAN.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructureCAN.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructureCAN.GPIO_Speed = GPIO_Speed_50MHz;
    //GPIO_InitStructureCAN.GPIO_OType = GPIO_OType_PP; // Push - pull
    GPIO_InitStructureCAN.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init( GPIOD, &GPIO_InitStructureCAN );

    GPIO_InitStructureCAN.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructureCAN.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructureCAN.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructureCAN.GPIO_OType = GPIO_OType_PP; // Push - pull
    GPIO_InitStructureCAN.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init( GPIOD, &GPIO_InitStructureCAN );

    can_init( en_canIf_iF0, en_canBrate_1000KBPS, tpRcv_callbackBufferSave );

    CAN_FilterInitTypeDef   CAN_FilterInitStructure1;
    CAN_FilterInitStructure1.CAN_FilterNumber = 0;
    CAN_FilterInitStructure1.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure1.CAN_FilterScale = CAN_FilterScale_16bit;
    CAN_FilterInitStructure1.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
    CAN_FilterInitStructure1.CAN_FilterActivation = ENABLE;
    CAN_FilterInitStructure1.CAN_FilterIdLow = 0x00;
    CAN_FilterInitStructure1.CAN_FilterIdHigh = 0x00;
    CAN_FilterInitStructure1.CAN_FilterMaskIdHigh = 0x0000 << 5; //1 care and 0 do not care
    CAN_FilterInitStructure1.CAN_FilterMaskIdLow = 0x0000 << 5;
    can_ioctl( en_canIf_iF0, en_canIoctl_filter, &CAN_FilterInitStructure1 );
}

/**
 * \brief     to initialize the testing configurations including the configurations
 *              of the LEDs and user buttons and enabling the CRC
 * \param     void
 * \return    void
 */
void _initAPItest()
{
    //=======================CRC=======
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_CRC, ENABLE );

    // Configuration for the LEDs
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD, ENABLE );
    GPIO_InitTypeDef gpio_leds;
    GPIO_StructInit( &gpio_leds );
    gpio_leds.GPIO_Mode = GPIO_Mode_OUT;
    gpio_leds.GPIO_Pin = LEDS;
    GPIO_Init( GPIOD, &gpio_leds );
    GPIO_SetBits( GPIOD, LEDS );
    GPIO_ResetBits( GPIOD, LEDS );

    //====== Configuration for the buttons
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE );
    GPIO_InitTypeDef gpio_button;
    GPIO_StructInit( &gpio_button );
    gpio_button.GPIO_Mode = GPIO_Mode_IN;
    gpio_button.GPIO_Pin = USER_BUTTON;
    GPIO_Init( GPIOA, &gpio_button );
}


int8_t callback_main( en_canIf_t e_if, uint16_t c_id, uint8_t* pc_pload,
                      uint8_t pc_ploadLen )
{
    int8_t retval = 0;
    _setLed( pc_pload[0] % 4 );
    return retval;
}

/**
 * \brief    To fill in the array passed with random numbers
 * \param    arr     the array that will be filled with random numbers
 * \param    length  the length of random numbers you want to fill
 * \return   void
 */
void _genRand( uint8_t* arr, uint16_t length )
{
    uint16_t _i;
    srand( can_getTicksTx());
    for ( _i = 0; _i < length; _i++ )
    {
        arr[_i] = rand() % 255;
    }
}

/**
 * \brief    To get the CRC of the data inside arr with the length len and put
 *           the result into arr
 * \param    arr     the array you want to get the CRC for
 * \param    length  the length of data you want to get the CRC for
 * \return   void
 */
void _prepareDataWithCRC( uint8_t a_arr[], uint16_t len )
{
    uint32_t tmp;
    uint32_t ret2 = 0;
    //generate random numbers with length len
    _genRand( a_arr, len );
//   for ( uint16_t i = 0;i<len;i++ ){
//       arr[i] = i;
//    }
    /*
     * calculate the CRC of the data array -4
     */
    ret2 = CRC_CalcBlockCRC(( uint32_t * )a_arr, ( len-4 )/4 );
    CRC_ResetDR();
    /*
     * put the CRC value in the array to be sent
     */
    tmp = ( ret2 & 0xFF000000 );//HSB
    tmp = tmp >> 24;//HSB
    a_arr[len-4] = tmp;
    tmp = ( ret2 & 0x00FF0000 );
    tmp = tmp >> 16;
    a_arr[len-3] = tmp;
    tmp = ( ret2 & 0x0000FF00 );
    tmp = tmp >> 8;
    a_arr[len - 2] = tmp;
    a_arr[len - 1] = ( ret2 & 0x000000FF );
}

/**
 * \brief    To check the CRC for data with specific length
 * \param    length    the length of data you want check the CRC for
 * \param    *ptr      pointer to set of data that you want to check the CRC for
 * \return   returns 1 if CRC OK , 0 if there is error
 */
uint8_t _checkData( uint16_t length, uint8_t *ptr )
{
    uint32_t CRC_value;
    uint32_t temp;
    uint8_t err = 1;
    /*
     * get the CRC value sent
     */
    CRC_value = ( ptr[length - 4] << 8 );
    CRC_value = ( CRC_value | ptr[length - 3] ) << 8;
    CRC_value = ( CRC_value | ptr[length - 2] ) << 8;
    CRC_value = ( CRC_value | ptr[length - 1] );

    CRC_ResetDR();
    temp = CRC_CalcBlockCRC(( uint32_t *)ptr, ( length - 4 ) / 4 );
    CRC_ResetDR();
    /*
     * compare the sent value and the calculated CRC
     */
    if ( temp != CRC_value )
    {
        err = 0;
    }
    return err;

}


/* --- EOF ----------------------------------------------------------------- */
