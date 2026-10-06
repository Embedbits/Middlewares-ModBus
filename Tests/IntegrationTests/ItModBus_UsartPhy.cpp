/**
 * \author Mr.Nobody
 * \file ItModBus_UsartPhy.cpp
 * \ingroup ModBus
 * \brief ModBus physical layer over USART MCAL used by integration tests.
 *
 * Frame format: 8 data bits, parity (9-bit USART word with parity bit) and
 * stop bits from the bus configuration. Reception runs in one shot buffer
 * mode, end of frame is detected by USART receiver timeout of 3.5 characters
 * (\ref USART_RX_END_TIMEOUT).
 */

/* ============================= INCLUDES =================================== */
#include "ItModBus_UsartPhy.hpp"            /* Self include                   */
/* ============================= TYPEDEFS =================================== */

/** \brief USART callbacks of one port slot */
typedef struct
{
    usart_XferCallback_t    *TxComplete; /**< Transmission complete   */
    usart_RxEndCallback_t   *RxEnd;      /**< End of received frame   */
    usart_XferCallback_t    *RxComplete; /**< Reception buffer full   */
    usart_XferErrCallback_t *Error;      /**< Transfer error          */
}   itModBus_PhyCallbacks_t;

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** USART interrupt priority */
static constexpr usart_IrqPrio_t itModBus_PhyIrqPriority = 5u;

/** Count of bits of start bit and 8 data bits */
static constexpr uint32_t itModBus_StartAndDataBits = 9u;

/* ============================== MACROS ==================================== */

/* ========================== LOCAL VARIABLES =============================== */

/** Physical layer object of every port slot (USART callbacks have no context) */
static ItModBus::UsartPhy *itModBus_PhyInstance[ IT_MODBUS_PHY_PORT_CNT ] = { nullptr, nullptr };

/* ========================== LOCAL FUNCTIONS =============================== */

/** \brief Transmission complete callback of port slot */
template< itModBus_PhyPort_t PORT >
static void ItModBus_TxCompleteCallback( void )
{
    itModBus_PhyInstance[ PORT ]->On_TxComplete();
}


/** \brief End of received frame callback of port slot */
template< itModBus_PhyPort_t PORT >
static void ItModBus_RxEndCallback( usart_RxDataCnt_t rxCnt )
{
    itModBus_PhyInstance[ PORT ]->On_RxEnd( rxCnt );
}


/** \brief Reception buffer full callback of port slot - frame of maximal length */
template< itModBus_PhyPort_t PORT >
static void ItModBus_RxCompleteCallback( void )
{
    itModBus_PhyInstance[ PORT ]->On_RxEnd( ModBus::PhysicalLayer::messageBufferMaxLen );
}


/** \brief Transfer error callback of port slot */
template< itModBus_PhyPort_t PORT >
static void ItModBus_ErrorCallback( usart_XferErrorId_t errorId )
{
    itModBus_PhyInstance[ PORT ]->On_Error( errorId );
}


/** USART callbacks of port slots */
static const itModBus_PhyCallbacks_t itModBus_PhyCallbacks[ IT_MODBUS_PHY_PORT_CNT ] =
{
    {
        ItModBus_TxCompleteCallback< IT_MODBUS_PHY_PORT_MASTER >,
        ItModBus_RxEndCallback     < IT_MODBUS_PHY_PORT_MASTER >,
        ItModBus_RxCompleteCallback< IT_MODBUS_PHY_PORT_MASTER >,
        ItModBus_ErrorCallback     < IT_MODBUS_PHY_PORT_MASTER >
    },
    {
        ItModBus_TxCompleteCallback< IT_MODBUS_PHY_PORT_SLAVE >,
        ItModBus_RxEndCallback     < IT_MODBUS_PHY_PORT_SLAVE >,
        ItModBus_RxCompleteCallback< IT_MODBUS_PHY_PORT_SLAVE >,
        ItModBus_ErrorCallback     < IT_MODBUS_PHY_PORT_SLAVE >
    }
};

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Creates physical layer bound to port slot
 *
 * \param port       [in]: Port slot (USART callbacks)
 * \param portConfig [in]: Hardware resources, must stay valid
 */
ItModBus::UsartPhy::UsartPhy( itModBus_PhyPort_t port, const itModBus_PortConfig_t &portConfig )
    : phyPort( port ), hwConfig( portConfig )
{
    itModBus_PhyInstance[ port ] = this;
}


void ItModBus::UsartPhy::Set_Transmission_Start( const uint8_t *msgBuffer, modBus_MsgLength_t msgLen )
{
    const modBus_MsgLength_t recordLen = ( messageBufferMaxLen < msgLen ) ? messageBufferMaxLen : msgLen;

    for( modBus_MsgLength_t byteIdx = 0u; recordLen > byteIdx; byteIdx++ )
    {
        lastTxFrame[ byteIdx ] = msgBuffer[ byteIdx ];
    }

    lastTxFrameLen = recordLen;
    txFrameCnt++;

    txFinished = false;

    if( USART_REQUEST_OK != Usart_Set_TxStart( hwConfig.Bus, msgBuffer, msgLen ) )
    {
        /* Transport layer waits for the end of transmission - reported as finished */
        txStartErrCnt++;
        txFinished = true;
    }
    else
    {
        /* Transmission is running */
    }
}


void ItModBus::UsartPhy::Set_Transmission_Stop( void )
{
    if( false == txFinished )
    {
        (void)Usart_Set_TxStop( hwConfig.Bus );

        txFinished = true;
    }
    else
    {
        /* Transmission already finished */
    }
}


modBus_TxStatus_t ItModBus::UsartPhy::Get_Transmission_Status( void )
{
    return ( ( true == txFinished ) ? MODBUS_TX_FINISHED : MODBUS_TX_ACTIVE );
}


void ItModBus::UsartPhy::Set_Reception_Start( void )
{
    (void)Usart_Set_RxStop( hwConfig.Bus );

    messageRxLen    = 0u;
    rxFrameEndCnt   = 0u;
    rxFrameReported = false;
    rxFrameEnd      = false;

    (void)Usart_Set_RxStart( hwConfig.Bus );
}


void ItModBus::UsartPhy::Set_Reception_Stop( void )
{
    (void)Usart_Set_RxStop( hwConfig.Bus );
}


modBus_RxStatus_t ItModBus::UsartPhy::Get_Reception_Status( void )
{
    modBus_RxStatus_t rxStatus = MODBUS_RX_NOT_RCVD;

    if( true == rxFrameEnd )
    {
        if( false == rxFrameReported )
        {
            /* New frame - recorded before the slave builds the response in the buffer */
            messageRxLen    = rxFrameEndCnt;
            rxFrameReported = true;

            for( modBus_MsgLength_t byteIdx = 0u; messageRxLen > byteIdx; byteIdx++ )
            {
                lastRxFrame[ byteIdx ] = messageRxBuffer[ byteIdx ];
            }

            lastRxFrameLen = messageRxLen;
            rxFrameCnt++;
        }
        else
        {
            /* Frame already reported */
        }

        rxStatus = MODBUS_RX_READY;
    }
    else
    {
        /* Frame is not complete */
    }

    return ( rxStatus );
}


void ItModBus::UsartPhy::Set_ProcessingActive( void )
{
    /* No activity indication in tests */
}


void ItModBus::UsartPhy::Set_ProcessingInactive( void )
{
    /* No activity indication in tests */
}


void ItModBus::UsartPhy::Set_Baudrate( modBus_Baudrate_t baudrate )
{
    busConfiguration.Baudrate = baudrate;
}


void ItModBus::UsartPhy::Set_Parity( modBus_Parity_t parity )
{
    busConfiguration.Parity = parity;
}


void ItModBus::UsartPhy::Set_StopBits( modBus_StopBits_t stopbits )
{
    busConfiguration.StopBits = stopbits;
}


/**
 * \brief Initializes USART with actual bus configuration
 *
 * 8 data bits (+ parity bit), receiver timeout 3.5 characters, both directions
 * in selected transfer mode. RX pin gets pull-up - the line is defined while
 * the TX pin of the other side is not initialized yet.
 */
void ItModBus::UsartPhy::PhyInit( void )
{
    usart_BusConfig_t     busConfig;
    usart_RequestState_t  initState = USART_REQUEST_ERROR;

    (void)Usart_Set_TxStop( hwConfig.Bus );
    (void)Usart_Set_RxStop( hwConfig.Bus );

    dataConfig.TxMode             = xferMode;
    dataConfig.RxMode             = xferMode;
    dataConfig.RxBuffer           = messageRxBuffer;
    dataConfig.RxBufferSize       = messageBufferMaxLen;
    dataConfig.RxBufferMode       = USART_BUFFER_MODE_ONE_SHOT;
    dataConfig.RxEndMode          = USART_RX_END_TIMEOUT;
    dataConfig.TxDmaPeriphId      = USART_DMA_PERIPH_1;
    dataConfig.TxDmaChannelId     = hwConfig.TxDmaChannel;
    dataConfig.TxDmaPriority      = USART_DMA_PRIORITY_LOW;
    dataConfig.RxDmaPeriphId      = USART_DMA_PERIPH_1;
    dataConfig.RxDmaChannelId     = hwConfig.RxDmaChannel;
    dataConfig.RxDmaPriority      = USART_DMA_PRIORITY_HIGH;
    dataConfig.IrqPriority        = itModBus_PhyIrqPriority;
    dataConfig.TxCompleteCallback = itModBus_PhyCallbacks[ phyPort ].TxComplete;
    dataConfig.RxHalfCallback     = nullptr;
    dataConfig.RxCompleteCallback = itModBus_PhyCallbacks[ phyPort ].RxComplete;
    dataConfig.RxEndCallback      = itModBus_PhyCallbacks[ phyPort ].RxEnd;
    dataConfig.ErrorCallback      = itModBus_PhyCallbacks[ phyPort ].Error;

    if( USART_REQUEST_OK == Usart_Get_DefaultConfig( &busConfig ) )
    {
        busConfig.PeriphId       = hwConfig.Bus;
        busConfig.BaudRate       = busConfiguration.Baudrate;
        busConfig.TransferMode   = USART_TRANSFER_MODE_TX_RX;
        busConfig.HalfDuplex     = USART_HALF_DUPLEX_INACTIVE;
        busConfig.BusTxPin       = hwConfig.TxPin;
        busConfig.BusRxPin       = hwConfig.RxPin;
        busConfig.DataConfig     = &dataConfig;

        /* 3.5 characters of silence - end of ModBus RTU frame */
        busConfig.RxTimeoutValue = ( ( 7u * Get_CharacterBits() ) + 1u ) / 2u;

        switch( busConfiguration.Parity )
        {
            case MODBUS_PARITY_EVEN:
                busConfig.Parity    = USART_PARITY_EVEN;
                busConfig.DataWidth = USART_DATA_WIDTH_9;       /* 8 data bits + parity bit */
                break;

            case MODBUS_PARITY_ODD:
                busConfig.Parity    = USART_PARITY_ODD;
                busConfig.DataWidth = USART_DATA_WIDTH_9;       /* 8 data bits + parity bit */
                break;

            default:
                busConfig.Parity    = USART_PARITY_NONE;
                busConfig.DataWidth = USART_DATA_WIDTH_8;
                break;
        }

        switch( busConfiguration.StopBits )
        {
            case MODBUS_STOPBIT_0_5: busConfig.StopBits = USART_STOP_BITS_0_5; break;
            case MODBUS_STOPBIT_1_5: busConfig.StopBits = USART_STOP_BITS_1_5; break;
            case MODBUS_STOPBIT_2:   busConfig.StopBits = USART_STOP_BITS_2;   break;
            default:                 busConfig.StopBits = USART_STOP_BITS_1;   break;
        }

        initState = Usart_Init( &busConfig );
    }
    else
    {
        /* Default configuration not available */
    }

    if( USART_REQUEST_OK == initState )
    {
        initOk = ( GPIO_REQUEST_OK == Gpio_Set_PinPull( hwConfig.RxGpioPort, hwConfig.RxGpioPin, GPIO_PIN_PULL_UP ) );
    }
    else
    {
        initOk = false;
    }

    txFinished = true;
}


uint8_t *ItModBus::UsartPhy::Get_RxMessageBuff( void )
{
    return ( messageRxBuffer );
}


modBus_MsgLength_t ItModBus::UsartPhy::Get_RxMessageLen( void )
{
    return ( messageRxLen );
}


/**
 * \brief Selects data transfer mode of both directions, applied by next PhyInit
 *
 * \param mode [in]: USART_XFER_MODE_ISR / USART_XFER_MODE_DMA / USART_XFER_MODE_POLL
 */
void ItModBus::UsartPhy::Set_XferMode( usart_XferMode_t mode )
{
    xferMode = mode;
}


/** \brief Returns true if the last PhyInit succeeded (USART initialized, RX pull-up set) */
bool ItModBus::UsartPhy::Get_InitOk( void ) const
{
    return ( initOk );
}


/** \brief Returns count of frames received (reported to transport layer) */
uint32_t ItModBus::UsartPhy::Get_RxFrameCnt( void ) const
{
    return ( rxFrameCnt );
}


/** \brief Returns copy of the last received frame (incl. CRC) */
const uint8_t *ItModBus::UsartPhy::Get_LastRxFrame( void ) const
{
    return ( lastRxFrame );
}


/** \brief Returns length of the last received frame (incl. CRC) */
modBus_MsgLength_t ItModBus::UsartPhy::Get_LastRxFrameLen( void ) const
{
    return ( lastRxFrameLen );
}


/** \brief Returns count of started transmissions */
uint32_t ItModBus::UsartPhy::Get_TxFrameCnt( void ) const
{
    return ( txFrameCnt );
}


/** \brief Returns copy of the last transmitted frame (incl. CRC) */
const uint8_t *ItModBus::UsartPhy::Get_LastTxFrame( void ) const
{
    return ( lastTxFrame );
}


/** \brief Returns length of the last transmitted frame (incl. CRC) */
modBus_MsgLength_t ItModBus::UsartPhy::Get_LastTxFrameLen( void ) const
{
    return ( lastTxFrameLen );
}


/** \brief Returns count of USART transfer errors */
uint32_t ItModBus::UsartPhy::Get_ErrorCnt( void ) const
{
    return ( errorCnt );
}


/** \brief Returns count of rejected transmission starts */
uint32_t ItModBus::UsartPhy::Get_TxStartErrorCnt( void ) const
{
    return ( txStartErrCnt );
}


/** \brief Returns identification of the last USART transfer error (USART_XFER_ERROR_CNT - none) */
usart_XferErrorId_t ItModBus::UsartPhy::Get_LastErrorId( void ) const
{
    return ( lastErrorId );
}


/** \brief Clears records of communication (frames, errors) */
void ItModBus::UsartPhy::Reset_Records( void )
{
    lastRxFrameLen = 0u;
    rxFrameCnt     = 0u;
    lastTxFrameLen = 0u;
    txFrameCnt     = 0u;
    errorCnt       = 0u;
    lastErrorId    = USART_XFER_ERROR_CNT;
    txStartErrCnt  = 0u;
}


/** \brief Clears records of USART errors (exchange repeated after debug halt) */
void ItModBus::UsartPhy::Clear_Errors( void )
{
    errorCnt    = 0u;
    lastErrorId = USART_XFER_ERROR_CNT;
}


/** \brief Transmission complete event (interrupt context) */
void ItModBus::UsartPhy::On_TxComplete( void )
{
    txFinished = true;
}


/**
 * \brief End of received frame event (interrupt context)
 *
 * \param rxCnt [in]: Count of received bytes
 */
void ItModBus::UsartPhy::On_RxEnd( usart_RxDataCnt_t rxCnt )
{
    if( false == rxFrameEnd )
    {
        rxFrameEndCnt = ( messageBufferMaxLen < rxCnt ) ? messageBufferMaxLen : rxCnt;
        rxFrameEnd    = true;
    }
    else
    {
        /* Frame already ended (buffer full and timeout) */
    }
}


/**
 * \brief Transfer error event (interrupt context)
 *
 * \param errorId [in]: Error identification
 */
void ItModBus::UsartPhy::On_Error( usart_XferErrorId_t errorId )
{
    lastErrorId = errorId;

    errorCnt++;
}

/* ========================== LOCAL FUNCTIONS =============================== */

/** \brief Returns count of bits of one character (start, 8 data, parity, stop bits) */
uint32_t ItModBus::UsartPhy::Get_CharacterBits( void ) const
{
    const uint32_t parityBits = ( MODBUS_PARITY_NONE == busConfiguration.Parity ) ? 0u : 1u;
    const uint32_t stopBits   = ( ( MODBUS_STOPBIT_1_5 == busConfiguration.StopBits ) ||
                                  ( MODBUS_STOPBIT_2   == busConfiguration.StopBits )    ) ? 2u : 1u;

    return ( itModBus_StartAndDataBits + parityBits + stopBits );
}
