#include "CAN_Config.h"
#include "can.h"

static can_fp_callback_t pf_can_callback;
static uint32_t ticksTx;
static uint32_t ticksRx;
static uint32_t ticksGn;


/*Before initializing the CAN you should
1- Configure the i/o port first
	1- Enable the clock of the port by calling RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);
	2- Changes the mapping of the CANTx and CANRx pin by calling GPIO_PinAFConfig(GPIOx,GPIO_PinSourcex,GPIO_AF_CANx);
	3- Specify GPIO_Mode, GPIO_Speed, GPIO_OType and GPIO_PuPd and then call GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct)
2- Call can_ioctl(e_can_if_t e_if, e_can_ioctl_t e_ctlType, void* p_ctlValue) to enable the filter
*/
int8_t can_init( en_canIf_t e_if, en_canBrate_t e_brate,
                can_fp_callback_t pf_rxCallback )
{

	int8_t _retval = 1;
	uint8_t _rate = 0;
	CAN_TypeDef* p_canx;
	CAN_InitTypeDef _canInitStruct;
	NVIC_InitTypeDef _nvicInitStructure;
	// Check the parameters
	assert_param( IS_CAN_ALL_PERIPH( e_if ));
	switch( e_brate )
	{
	    case en_canBrate_62p5KBPS:
            _rate = 1;
	    break;
	    case en_canBrate_125KBPS:
	        _rate = 2;
	        break;
	    case en_canBrate_250KBPS:
	        _rate = 4;
	        break;
	    case en_canBrate_500KBPS:
	        _rate = 8;
	        break;
	    case en_canBrate_1000KBPS:
	        _rate = 16;
	        break;
	}
	switch ( e_if )
	{
        case 0:
            p_canx = CAN1;
            RCC_APB1PeriphClockCmd( RCC_APB1Periph_CAN1, ENABLE );
            break;
        case 1:
            p_canx = CAN2;
            RCC_APB1PeriphClockCmd( RCC_APB1Periph_CAN2, ENABLE );
            break;
        default:
            break;
	}

	CAN_DeInit( p_canx );

	CAN_StructInit( &_canInitStruct );
	_canInitStruct.CAN_Mode = CAN_Mode_CONF;
	_canInitStruct.CAN_SJW = CAN_SJW_CONF;
	_canInitStruct.CAN_BS1 = CAN_BS1_CONF;
	_canInitStruct.CAN_BS2 = CAN_BS2_CONF;
	_canInitStruct.CAN_Prescaler = CAN_Prescaler_CONF / _rate;
	// if Success
	if ( CAN_Init( p_canx, &_canInitStruct) == 1 )
	{
		_retval = 0;
	}
	// if FAILED
	else
	{
		_retval = 1;
	}
	if( pf_rxCallback == 0 )
	{
		pf_can_callback = pf_rxCallback;
	}
	else
	{
	    pf_can_callback = pf_rxCallback;
	    CAN_ITConfig( p_canx, CAN_IT_FMP0, ENABLE );
	    CAN_ITConfig( p_canx, CAN_IT_FMP1, ENABLE );
	    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );

	    // Highest priority for CAN messages
	    _nvicInitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	    _nvicInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	    _nvicInitStructure.NVIC_IRQChannelSubPriority = 0;
	    _nvicInitStructure.NVIC_IRQChannelCmd = ENABLE;
	    NVIC_Init( &_nvicInitStructure );

	    _nvicInitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	    _nvicInitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	    NVIC_Init( &_nvicInitStructure );
	}
return _retval;
}

int8_t can_ioctl( en_canIf_t e_if, en_canIoctl_t e_ctlType, void* p_ctlValue )
{
	int8_t _retval=0;
	if( e_ctlType == en_canIoctl_filter )
	{
		//Apply the default filter to the specified FIFO (Accept all the messages)
		CAN_FilterInitTypeDef   CAN_FilterInitStructure;
		CAN_FilterInit( &CAN_FilterInitStructure );
		CAN_FilterInitStructure.CAN_FilterNumber =
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) -> CAN_FilterNumber;
		CAN_FilterInitStructure.CAN_FilterMode=
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) -> CAN_FilterMode;
		CAN_FilterInitStructure.CAN_FilterScale =
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) -> CAN_FilterScale;
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment=
		    (( CAN_FilterInitTypeDef* ) p_ctlValue ) -> CAN_FilterFIFOAssignment;
		CAN_FilterInitStructure.CAN_FilterActivation =
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) -> CAN_FilterActivation;
		CAN_FilterInitStructure.CAN_FilterIdLow =
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) -> CAN_FilterIdLow;
		CAN_FilterInitStructure.CAN_FilterIdHigh=
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) -> CAN_FilterIdHigh;
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh =
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) ->
		        CAN_FilterMaskIdHigh << 5; //1 care and 0 do not care
		CAN_FilterInitStructure.CAN_FilterMaskIdLow =
		    (( CAN_FilterInitTypeDef * ) p_ctlValue ) -> CAN_FilterIdLow << 5;
		CAN_FilterInit(&CAN_FilterInitStructure);
	}
	return _retval;
}

en_mailBox_t can_send( en_canIf_t e_if, int16_t i_id,
                      uint8_t* pc_pload, uint8_t c_ploadLen )
{
    en_mailBox_t _retval = en_mailBox_noMailBox;
	/* Create a local variable of structure type "CanTxMsg" */
	CanTxMsg _txMessage;
	uint8_t _transmitMailbox;

	CAN_TypeDef* _canx;
	switch ( e_if )
	{
        case 0:
            _canx = CAN1;
            break;
        case 1:
            _canx = CAN2;
            break;
        default:
            break;
	}

	/* Fill our local structure variable with configuration contents */
	_txMessage.IDE = IDE_CONF;
	_txMessage.StdId = i_id;
	_txMessage.ExtId = ExtId_CONF;
	_txMessage.RTR = RTR_CONF;
	_txMessage.DLC = c_ploadLen;
	for(int i = 1; i <= c_ploadLen; i++)
	{
		_txMessage.Data[i - 1] = pc_pload[i - 1];
	}
	_transmitMailbox = CAN_Transmit( _canx, &_txMessage );
	if ( _transmitMailbox == 0 )
	{
	    _retval = en_mailBox_mailBox1;
	}
	else if ( _transmitMailbox == 1 )
	{
	    _retval = en_mailBox_mailBox2;
	}
	else if ( _transmitMailbox == 2 )
	{
	    _retval = en_mailBox_mailBox3;
	}
	else
	{
	    _retval = en_mailBox_noMailBox;
	}
	return _retval;
}


int8_t can_receive( en_canIf_t e_if, uint16_t* pi_id,
                   uint8_t* pc_pload, uint8_t* pc_ploadLen )
{

	int8_t _retval = 1;
	CanRxMsg _rxMessage;
	CAN_TypeDef* _canx;

	switch ( e_if )
	{
        case 0:
            _canx = CAN1;
            break;
        case 1:
            _canx = CAN2;
            break;
        default:
            break;
	}
	CAN_Receive( _canx, CANRx_FIFO_CONF, &_rxMessage );
	*pi_id = _rxMessage.StdId;
	*pc_ploadLen = _rxMessage.DLC;
	for(int8_t i = 0; i < _rxMessage.DLC; i++)
	{
		pc_pload[i] = _rxMessage.Data[i];
	}

	return _retval;
}
uint8_t isTransmitted( en_canIf_t e_if, uint8_t mailBox )
{
    uint8_t _retval;
    CAN_TypeDef* _canx;
    switch ( e_if )
    {
        case 0:
            _canx = CAN1;
            break;
        case 1:
            _canx = CAN2;
            break;
        default:
            break;
    }
    if( CAN_TransmitStatus( _canx, mailBox ) != CAN_TxStatus_Ok )
    {
        _retval = 0;
    }
    else
    {
        _retval = 1;
    }
    return _retval;

}
int8_t can_poll( en_canIf_t e_if )
{
	int8_t _retval=1;
	CAN_TypeDef* _canx;
	CanRxMsg _rxMessage;
	switch ( e_if )
	{
        case 0:
            _canx=CAN1;
            break;
        case 1:
            _canx=CAN2;
            break;
        default:
            break;
	}
	if( CAN_MessagePending ( _canx, CAN_FIFO0 ) != 0 )
	{

		CAN_Receive(_canx, CAN_FIFO0, &_rxMessage );
		_retval = pf_can_callback(e_if,_rxMessage.StdId,&_rxMessage.Data[0],
		                          _rxMessage.DLC );
	}
	else if ( CAN_MessagePending ( _canx,CAN_FIFO1 ) != 0 )
	{

	        CAN_Receive ( _canx, CAN_FIFO1, &_rxMessage );
	        _retval = pf_can_callback( e_if,_rxMessage.StdId,
	                                   &_rxMessage.Data[0],_rxMessage.DLC );
	}
	else
		_retval = -1;

	return _retval;
}

int8_t can_deinit( en_canIf_t e_if )
{

	CAN_TypeDef* _canx;

	switch ( e_if )
	{
        case 0:
            _canx=CAN1;
            break;
        case 1:
            _canx=CAN2;
            break;
        default:
            break;
	}
	CAN_DeInit( _canx );

	return 0;
}
void can_initCLK()
{

    SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK_Div8 );
    /* every tick=100usec */
    SysTick_Config(SystemCoreClock / 200000);
    NVIC_SetPriority( SysTick_IRQn, 0 );


}

void can_setTxLED()
{
    GPIO_SetBits( GPIOD, GPIO_Pin_13 );
}
void can_resetTxLED()
{
    GPIO_ResetBits( GPIOD, GPIO_Pin_13 );
}
void can_setRxLED()
{
    GPIO_SetBits( GPIOD, GPIO_Pin_15 );
}
void can_resetRxLED()
{
    GPIO_ResetBits( GPIOD, GPIO_Pin_15 );
}

void SysTick_Handler(void)
{
    ticksTx++;
    ticksRx++;
    ticksGn++;
}
uint32_t can_getTicksGeneral()
{
    return ticksGn;
}
void can_resetTicksGeneral()
{
    ticksGn=0;
}
uint32_t can_getTicksTx()
{
    return ticksTx;
}
uint32_t can_getTicksRx()
{
    return ticksRx;
}
void can_resetTicksTx(){
    ticksTx=0;
}
void can_resetTicksRx()
{
    ticksRx=0;
}

// the interrupt handler fot CAN1
void CAN1_RX0_IRQHandler( void )
{
	/* Create a local variable of structure type "CanRxMsg" */
	CanRxMsg _rxMessage;

	memset( &_rxMessage, 0, sizeof( _rxMessage ));
	CAN_Receive ( CAN1, CAN_FIFO0, &_rxMessage );
	pf_can_callback( en_canIf_iF0,_rxMessage.StdId, &_rxMessage.Data[0],
	                 _rxMessage.DLC);
}
// the interrupt handler fot CAN1
void CAN2_RX0_IRQHandler(void)
{
	/* Create a local variable of structure type "CanRxMsg" */
	CanRxMsg _rxMessage;

	memset( &_rxMessage, 0, sizeof( _rxMessage ));
	CAN_Receive ( CAN2, CAN_FIFO0, &_rxMessage );
	pf_can_callback ( en_canIf_iF1, _rxMessage.StdId, &_rxMessage.Data[0],
	                  _rxMessage.DLC);

}

void CAN1_RX1_IRQHandler(void)
{
	/* Create a local variable of structure type "CanRxMsg" */
	CanRxMsg _rxMessage;

	memset ( &_rxMessage, 0, sizeof( _rxMessage ));
	CAN_Receive ( CAN1, CAN_FIFO1, &_rxMessage );
	pf_can_callback ( en_canIf_iF0, _rxMessage.StdId, &_rxMessage.Data[0],
	                  _rxMessage.DLC);

}
void CAN2_RX1_IRQHandler(void)
{
	/* Create a local variable of structure type "CanRxMsg" */
	CanRxMsg _rxMessage;

	memset ( &_rxMessage, 0, sizeof( _rxMessage ));
	CAN_Receive ( CAN2, CAN_FIFO1, &_rxMessage );
	pf_can_callback ( en_canIf_iF1, _rxMessage.StdId, &_rxMessage.Data[0],
	                  _rxMessage.DLC);

}
