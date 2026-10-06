/**
 * \author Mr.Nobody
 * \file ModBus_Master_TL.cpp
 * \ingroup ModBus
 * \brief ModBus Master module Transport Layer (TL) functionality core
 *
 */

/* ============================= INCLUDES =================================== */
#include "ModBus_Master_TL.hpp"             /* Self include                   */
#include "ModBus_Phy.hpp"                   /* Physical layer definition      */
#include "ModBus_Core_Types.h"              /* Module C types definition      */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief ModBus master mode transport layer constructor
 *
 * \param physicalLayerObj [in]: Physical layer used for communication
 */
ModBus::Master::TransportLayer::TransportLayer( PhysicalLayer &physicalLayerObj ) :
    physicalLayer( physicalLayerObj )
{
    sm_ActualState = STATE_INIT;
    sm_NewState    = STATE_INIT;
}


/**
 * \brief Module task handler
 *
 * \param taskTime [in]: Value of cyclic execution of task handler in ms [0.001s]
 */
void ModBus::Master::TransportLayer::Task( modBus_Time_t taskTime )
{
    handlerTaskTime = taskTime;
    StateHandler();
}


/**
 * \brief Configures maximum response timeout
 *
 * Slave shall respond within specified time-frame. This time can be configured
 * through this function. Default value is \ref modBus_DefaultResponseTimeout_ms.
 *
 * \param timeout [in]: Response timeout value in ms [0.001s]
 */
void ModBus::Master::TransportLayer::Set_ResponseTimeout( modBus_Time_t timeout )
{
    responseTimeout = timeout;
}


/**
 * \brief Configures turnaround delay used after broadcast request
 *
 * Slaves do not respond to broadcast request. Master waits this time, so
 * slaves can process the request, before the request is signaled as done.
 * Default value is \ref modBus_DefaultTurnaroundDelay_ms.
 *
 * \param delay [in]: Turnaround delay in ms [0.001s]
 */
void ModBus::Master::TransportLayer::Set_TurnaroundDelay( modBus_Time_t delay )
{
    turnaroundDelay = delay;
}


/**
 * \brief Starts new request
 *
 * \param messageLen  [in]: Length of request without CRC (address and PDU)
 * \param txBufferPtr [in]: Buffer with request. Shall have space for 2 more
 *                          bytes - CRC is appended into this buffer.
 * \param rxBufferPtr [in]: Buffer for response. Size shall be at least
 *                          \ref PhysicalLayer::messageBufferMaxLen bytes.
 *
 * \return \ref MODBUS_REQUEST_OK if request has been accepted,
 *         \ref MODBUS_REQUEST_ERROR if other request is still processed or
 *         parameters are invalid.
 */
modBus_RequestState_t ModBus::Master::TransportLayer::StartTransmit( modBus_MsgLength_t messageLen,
                                                                     uint8_t * const txBufferPtr,
                                                                     uint8_t * const rxBufferPtr )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    if( ( MODBUS_MASTER_REQ_BUSY                     != requestStatus ) &&
        ( nullptr                                    != txBufferPtr   ) &&
        ( nullptr                                    != rxBufferPtr   ) &&
        ( ( MODBUS_FRAME_MIN_LEN - MODBUS_CRC_LEN )  <= messageLen    ) &&
        ( ( MODBUS_FRAME_MAX_LEN - MODBUS_CRC_LEN )  >= messageLen    )    )
    {
        txFrameData   = txBufferPtr;
        txFrameLength = messageLen;
        rxFrameData   = rxBufferPtr;
        rxFrameLength = 0u;

        requestStatus  = MODBUS_MASTER_REQ_BUSY;
        requestPending = true;

        retState = MODBUS_REQUEST_OK;
    }
    else
    {
        /* Request can not be started */
    }

    return ( retState );
}


/**
 * \brief Returns status of actual (last) request
 *
 * \return Request status, see \ref modBus_MasterRequestStatus_t
 */
modBus_MasterRequestStatus_t ModBus::Master::TransportLayer::Get_RequestStatus( void ) const
{
    return ( requestStatus );
}


/**
 * \brief API used to get status of response reception
 *
 * \param msgLen [out]: Length of response without CRC (address and PDU). Can be
 *                      null pointer. Updated only if response is ready.
 *
 * \return Reception status:
 * - \ref MODBUS_RX_NOT_RCVD : No valid response is available
 * - \ref MODBUS_RX_READY    : Valid response is stored in reception buffer
 */
modBus_RxStatus_t ModBus::Master::TransportLayer::Get_RxMessageStatus( modBus_MsgLength_t * const msgLen ) const
{
    modBus_RxStatus_t newMessageStatus = MODBUS_RX_NOT_RCVD;

    if( ( MODBUS_MASTER_REQ_DONE == requestStatus ) &&
        ( 0u                     != rxFrameLength )    )
    {
        newMessageStatus = MODBUS_RX_READY;

        if( nullptr != msgLen )
        {
            *msgLen = rxFrameLength;
        }
        else
        {
            /* Null pointer assigned */
        }
    }
    else
    {
        newMessageStatus = MODBUS_RX_NOT_RCVD;
    }

    return newMessageStatus;
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Moore Finite-State Machine - run function
 *
 * Possible states in FSM:
 * - \ref STATE_INIT          : Initialization state
 * - \ref STATE_STANDBY       : Standby state
 * - \ref STATE_TRANSMIT      : Request transmit state
 * - \ref STATE_RESPONSE_WAIT : Wait for response reception state
 * - \ref STATE_FRAME_CHECK   : Response check state
 * - \ref STATE_TURNAROUND    : Turnaround delay after broadcast request
 */
void ModBus::Master::TransportLayer::StateHandler( void )
{
    /* This array must have the same order as the state enum! */
    static const stateLinkedRoutines_t stateRoutines [] =
    {
        { STATE_INIT,
            { &ModBus::Master::TransportLayer::State_Init_Entry,
              &ModBus::Master::TransportLayer::State_Init_Execute,
              &ModBus::Master::TransportLayer::State_Init_CheckLeave,
              &ModBus::Master::TransportLayer::State_Init_Leave                 }  },

        { STATE_STANDBY,
            { &ModBus::Master::TransportLayer::State_Standby_Entry,
              &ModBus::Master::TransportLayer::State_Standby_Execute,
              &ModBus::Master::TransportLayer::State_Standby_CheckLeave,
              &ModBus::Master::TransportLayer::State_Standby_Leave              }  },

        { STATE_TRANSMIT,
            { &ModBus::Master::TransportLayer::State_Transmit_Entry,
              &ModBus::Master::TransportLayer::State_Transmit_Execute,
              &ModBus::Master::TransportLayer::State_Transmit_CheckLeave,
              &ModBus::Master::TransportLayer::State_Transmit_Leave             }  },

        { STATE_RESPONSE_WAIT,
            { &ModBus::Master::TransportLayer::State_ResponseWait_Entry,
              &ModBus::Master::TransportLayer::State_ResponseWait_Execute,
              &ModBus::Master::TransportLayer::State_ResponseWait_CheckLeave,
              &ModBus::Master::TransportLayer::State_ResponseWait_Leave         }  },

        { STATE_FRAME_CHECK,
            { &ModBus::Master::TransportLayer::State_FrameCheck_Entry,
              &ModBus::Master::TransportLayer::State_FrameCheck_Execute,
              &ModBus::Master::TransportLayer::State_FrameCheck_CheckLeave,
              &ModBus::Master::TransportLayer::State_FrameCheck_Leave           }  },

        { STATE_TURNAROUND,
            { &ModBus::Master::TransportLayer::State_Turnaround_Entry,
              &ModBus::Master::TransportLayer::State_Turnaround_Execute,
              &ModBus::Master::TransportLayer::State_Turnaround_CheckLeave,
              &ModBus::Master::TransportLayer::State_Turnaround_Leave           }  }
    };

    /* Check if the state is in valid range of available modes, if no set the default/error state. */
    if( STATE_TURNAROUND < sm_NewState )
    {
        /* In case of an invalid input status, switch to default/error state. */
        sm_NewState = STATE_INIT;
    }
    else
    {
        /* Requested state is in valid range */
    }

    /* Execute function shall be used for main execution of actual state */
    (this->*(stateRoutines[sm_ActualState].routine.execute))();

    /* CheckLeave function shall be used for check leave condition of actual state */
    (this->*(stateRoutines[sm_ActualState].routine.checkLeave))();

    /* Entry function shall be executed only in case if the state has to be changed to another state */
    if( sm_ActualState != sm_NewState )
    {
        (this->*(stateRoutines[sm_ActualState].routine.leave))();

        sm_ActualState = sm_NewState;

        (this->*(stateRoutines[sm_ActualState].routine.entry))();
    }
    else
    {
        /* No new state required */
    }
}


/**
 * \brief Increments state counter by task time (saturated)
 */
void ModBus::Master::TransportLayer::IncrementStateCounter( void )
{
    const modBus_Time_t counterMax = (modBus_Time_t)( ~( (modBus_Time_t)0u ) );

    if( ( counterMax - stateCounter ) > handlerTaskTime )
    {
        stateCounter = stateCounter + handlerTaskTime;
    }
    else
    {
        stateCounter = counterMax;
    }
}

/*============================================================================*/
/*============================= Initialization state =========================*/
/*============================================================================*/
void ModBus::Master::TransportLayer::State_Init_Entry( void )
{
    return;
}


void ModBus::Master::TransportLayer::State_Init_Execute( void )
{
    stateCounter = 0u;
    phyEventDone = false;
}


void ModBus::Master::TransportLayer::State_Init_CheckLeave( void )
{
    sm_NewState = STATE_STANDBY;
}


void ModBus::Master::TransportLayer::State_Init_Leave( void )
{
    /* Initialize physical layer */
    physicalLayer.PhyInit();
}

/*============================================================================*/
/*============================== Standby state ===============================*/
/*============================================================================*/
void ModBus::Master::TransportLayer::State_Standby_Entry( void )
{
    /* Signal inactive processing of message */
    physicalLayer.Set_ProcessingInactive();
}


void ModBus::Master::TransportLayer::State_Standby_Execute( void )
{
    return;
}


void ModBus::Master::TransportLayer::State_Standby_CheckLeave( void )
{
    if( true == requestPending )
    {
        sm_NewState = STATE_TRANSMIT;
    }
    else
    {
        /* Stay in standby state */
    }
}


void ModBus::Master::TransportLayer::State_Standby_Leave( void )
{
    /* Signal active processing of message */
    physicalLayer.Set_ProcessingActive();
}

/*============================================================================*/
/*=========================== Request transmit state =========================*/
/*============================================================================*/
void ModBus::Master::TransportLayer::State_Transmit_Entry( void )
{
    requestPending = false;
    phyEventDone   = false;
    stateCounter   = 0u;

    /* Append CRC to request */
    Add_Crc( txFrameData, &txFrameLength );

    /* Start frame sending */
    physicalLayer.Set_Transmission_Start( txFrameData, txFrameLength );
}


void ModBus::Master::TransportLayer::State_Transmit_Execute( void )
{
    if( MODBUS_TX_FINISHED == physicalLayer.Get_Transmission_Status() )
    {
        phyEventDone = true;
    }
    else
    {
        IncrementStateCounter();
    }
}


void ModBus::Master::TransportLayer::State_Transmit_CheckLeave( void )
{
    if( true == phyEventDone )
    {
        if( MODBUS_BROADCAST_ADDR == txFrameData[ 0u ] )
        {
            /* No response is sent by slaves to broadcast request */
            sm_NewState = STATE_TURNAROUND;
        }
        else
        {
            sm_NewState = STATE_RESPONSE_WAIT;
        }
    }
    else if( modBus_TransmissionTimeout_ms <= stateCounter )
    {
        requestStatus = MODBUS_MASTER_REQ_TX_ERROR;

        sm_NewState = STATE_STANDBY;
    }
    else
    {
        /* Wait for transmission complete */
    }
}


void ModBus::Master::TransportLayer::State_Transmit_Leave( void )
{
    /* Disable frame transmission */
    physicalLayer.Set_Transmission_Stop();
}

/*============================================================================*/
/*========================== Wait for response state =========================*/
/*============================================================================*/
void ModBus::Master::TransportLayer::State_ResponseWait_Entry( void )
{
    phyEventDone = false;
    stateCounter = 0u;

    /* Prepare reception of response */
    physicalLayer.Set_Reception_Start();
}


void ModBus::Master::TransportLayer::State_ResponseWait_Execute( void )
{
    if( MODBUS_RX_READY == physicalLayer.Get_Reception_Status() )
    {
        phyEventDone = true;
    }
    else
    {
        IncrementStateCounter();
    }
}


void ModBus::Master::TransportLayer::State_ResponseWait_CheckLeave( void )
{
    if( true == phyEventDone )
    {
        sm_NewState = STATE_FRAME_CHECK;
    }
    else if( responseTimeout <= stateCounter )
    {
        requestStatus = MODBUS_MASTER_REQ_TIMEOUT;

        sm_NewState = STATE_STANDBY;
    }
    else
    {
        /* Wait for reception */
    }
}


void ModBus::Master::TransportLayer::State_ResponseWait_Leave( void )
{
    physicalLayer.Set_Reception_Stop();
}

/*============================================================================*/
/*============================ Response check state ==========================*/
/*============================================================================*/
void ModBus::Master::TransportLayer::State_FrameCheck_Entry( void )
{
    return;
}


void ModBus::Master::TransportLayer::State_FrameCheck_Execute( void )
{
    const uint8_t           *rcvdFrame = physicalLayer.Get_RxMessageBuff();
    const modBus_MsgLength_t rcvdLen   = physicalLayer.Get_RxMessageLen();

    if( ( nullptr              == rcvdFrame ) ||
        ( MODBUS_FRAME_MIN_LEN >  rcvdLen   ) ||
        ( MODBUS_FRAME_MAX_LEN <  rcvdLen   )    )
    {
        /* Incorrect frame length detected */
        requestStatus = MODBUS_MASTER_REQ_CRC_ERROR;
    }
    else if( MODBUS_CRC_OK != Check_Crc( rcvdFrame, rcvdLen ) )
    {
        /* CRC does not correspond */
        requestStatus = MODBUS_MASTER_REQ_CRC_ERROR;
    }
    else if( txFrameData[ 0u ] != rcvdFrame[ 0u ] )
    {
        /* Response was sent by other slave */
        requestStatus = MODBUS_MASTER_REQ_ADDR_ERROR;
    }
    else
    {
        /* Valid response, store it for application */
        for( modBus_MsgLength_t byteIndex = 0u; rcvdLen > byteIndex; byteIndex++ )
        {
            rxFrameData[ byteIndex ] = rcvdFrame[ byteIndex ];
        }

        rxFrameLength = rcvdLen - MODBUS_CRC_LEN;

        requestStatus = MODBUS_MASTER_REQ_DONE;
    }
}


void ModBus::Master::TransportLayer::State_FrameCheck_CheckLeave( void )
{
    sm_NewState = STATE_STANDBY;
}


void ModBus::Master::TransportLayer::State_FrameCheck_Leave( void )
{
    return;
}

/*============================================================================*/
/*========================= Broadcast turnaround state =======================*/
/*============================================================================*/
void ModBus::Master::TransportLayer::State_Turnaround_Entry( void )
{
    stateCounter = 0u;
}


void ModBus::Master::TransportLayer::State_Turnaround_Execute( void )
{
    IncrementStateCounter();
}


void ModBus::Master::TransportLayer::State_Turnaround_CheckLeave( void )
{
    if( turnaroundDelay <= stateCounter )
    {
        requestStatus = MODBUS_MASTER_REQ_DONE;

        sm_NewState = STATE_STANDBY;
    }
    else
    {
        /* Wait for turnaround delay */
    }
}


void ModBus::Master::TransportLayer::State_Turnaround_Leave( void )
{
    return;
}

/* =============================== TASKS ==================================== */
