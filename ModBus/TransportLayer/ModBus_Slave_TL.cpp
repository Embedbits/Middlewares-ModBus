/**
 * \author Mr.Nobody
 * \file ModBus_Slave_TL.cpp
 * \ingroup ModBus
 * \brief ModBus Slave module Transport Layer (TL) functionality core
 *
 */

/* ============================= INCLUDES =================================== */
#include "ModBus_Slave_TL.hpp"              /* Self include                   */
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
 * \brief ModBus transport layer slave mode module constructor
 *
 */
ModBus::Slave::TransportLayer::TransportLayer( PhysicalLayer &physicalLayerObj ) :
    physicalLayer( physicalLayerObj )
{
    sm_ActualState  = STATE_INIT;
    sm_NewState     = STATE_INIT;
}


/**
 * \brief ModBus transport layer slave mode module destructor
 *
 */
ModBus::Slave::TransportLayer::~TransportLayer( void )
{
    sm_ActualState  = STATE_INIT;
    sm_NewState     = STATE_INIT;
}


/**
 * \brief Module activation request
 *
 */
void ModBus::Slave::TransportLayer::Activate(void)
{
    runStatus = RUN_ACTIVE;
}


/**
 * \brief Module deactivation request
 *
 */
void ModBus::Slave::TransportLayer::Deactivate(void)
{
    runStatus = RUN_INACTIVE;
}


/**
 * \brief Returns status of new message
 *
 * \return Status of new message:
 *  \ref MODBUS_APP_MSG_NO_NEW      : No new message received
 *  \ref MODBUS_APP_MSG_NEW_PENDING : New message is pending for processing
 */
modBus_App_MsgStatus_t ModBus::Slave::TransportLayer::Get_NewMessageStatus( void ) const
{
    modBus_App_MsgStatus_t messageStatus = MODBUS_APP_MSG_NO_NEW;

    if( FRAME_READY_FOR_PROCESS == frameDataStruct.FrameStatus)
    {
        messageStatus = MODBUS_APP_MSG_NEW_PENDING;
    }
    else
    {
        messageStatus = MODBUS_APP_MSG_NO_NEW;
    }

    return ( messageStatus );
}


/**
 * \brief Returns length of new message
 *
 * \return Length of new message in byte count without CRC (address and PDU).
 *         Valid only if \ref Get_NewMessageStatus signals pending message.
 */
modBus_MsgLength_t ModBus::Slave::TransportLayer::Get_NewMessageLen( void ) const
{
    modBus_MsgLength_t msgLen = 0u;

    if( FRAME_READY_FOR_PROCESS == frameDataStruct.FrameStatus )
    {
        /* Frame length is checked in frame check state (at least MODBUS_FRAME_MIN_LEN) */
        msgLen = frameDataStruct.FrameLength - MODBUS_CRC_LEN;
    }
    else
    {
        /* No message pending */
    }

    return ( msgLen );
}


/**
 * \brief Returns structure of statistic counters.
 *
 * \return Structure with values of statistics counters.
 */
modBus_StatsCounter_t ModBus::Slave::TransportLayer::Get_StatCounter( void ) const
{
    return (statsCounter);
}


/**
 * \brief Restarts communication statistics counters.
 *
 */
void ModBus::Slave::TransportLayer::Reset_StatCounter( void )
{
    statsCounter.addrMatchedCnt    = 0u;
    statsCounter.addrNotMatchedCnt = 0u;
    statsCounter.crcErrorCount     = 0u;
    statsCounter.framesRcvdCnt     = 0u;
}


/**
 * \brief Returns state of module
 *
 * \return State of module
 *  - \ref MODBUS_SLAVE_TL_INIT       : Initialization state of transport layer
 *  - \ref MODBUS_SLAVE_TL_STANDBY    : StandBy state of transport layer
 *  - \ref MODBUS_SLAVE_TL_PROCESSING : Processing message of transport layer
 *  - \ref MODBUS_SLAVE_TL_EXECUTING  : Executing message of transport layer
 *  - \ref MODBUS_SLAVE_TL_SENDING    : Sending response of transport layer
 *  - \ref MODBUS_SLAVE_TL_STATE_CNT  : Count of states in transport layer
 */
modBus_App_TransportLayerState_t ModBus::Slave::TransportLayer::Get_ModuleState( void ) const
{
    return ( moduleState );
}


/**
 * \brief Module task handler
 *
 * \param taskTime [in]: Value of cyclic execution of task handler in ms [0.001s]
 *
 */
void ModBus::Slave::TransportLayer::Task( modBus_Time_t taskTime )
{
    handlerTaskTime = taskTime;
    StateHandler();
}


/**
 * \brief Set device address of slave
 *
 * \param slaveAddr [in]: Value of device address [1-247]
 *
 * \return \ref MODBUS_REQUEST_OK if address is valid and it was applied
 */
modBus_RequestState_t ModBus::Slave::TransportLayer::Set_SlaveAddr( modBus_DeviceAddr_t slaveAddr )
{
    modBus_RequestState_t retState = MODBUS_REQUEST_ERROR;

    if( ( slaveAddrMin <= slaveAddr ) &&
        ( slaveAddrMax >= slaveAddr )    )
    {
        deviceAddress = slaveAddr;

        retState = MODBUS_REQUEST_OK;
    }
    else
    {
        /* Broadcast or reserved address can not be used as slave address */
    }

    return ( retState );
}


/**
 * \brief Returns actual device address of slave
 *
 * \return Device address [1-247]
 */
modBus_DeviceAddr_t ModBus::Slave::TransportLayer::Get_SlaveAddr( void ) const
{
    return ( deviceAddress );
}


/**
 * \brief Signals that pending request has been processed
 *
 * If user process the request and create response message, then transmission
 * is triggered by this method. CRC is appended by transport layer, thus the
 * buffer shall have space for two more bytes.
 *
 * \param msgBuffer  [in]: Pointer to response message buffer
 * \param messageLen [in]: Count of bytes in response message without CRC.
 *                         Value 0 signals that no response shall be sent
 *                         (broadcast request, listen only mode).
 */
void ModBus::Slave::TransportLayer::Set_MessageReady( uint8_t * const msgBuffer, modBus_MsgLength_t messageLen )
{
    if( ( FRAME_READY_FOR_PROCESS == frameDataStruct.FrameStatus ) &&
        ( nullptr                 != msgBuffer                   )    )
    {
        frameDataStruct.FrameStatus = FRAME_READY_FOR_TRANSMIT;
        frameDataStruct.FrameData   = msgBuffer;

        if( responseMaxLen >= messageLen )
        {
            frameDataStruct.FrameLength = messageLen;
        }
        else
        {
            /* Response does not fit into the frame. Nothing is sent. */
            frameDataStruct.FrameLength = 0u;
        }
    }
    else
    {
        /* No request is pending */
    }
}


/**
 * \brief Get pointer to received message buffer
 *
 * \return Pointer to received message buffer
 */
uint8_t *ModBus::Slave::TransportLayer::Get_RxMessagePtr( void )
{
    return ( physicalLayer.Get_RxMessageBuff() );
}


/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Moore Finite-State Machine - run function
 *
 * Possible states in FSM:
 * - \ref STATE_INIT              : Initialization state
 * - \ref STATE_STANDBY           : Standby state
 * - \ref STATE_FRAME_CHECK       : Received frame check state
 * - \ref STATE_PROCESS_REQUEST   : Processing of received message state
 * - \ref STATE_TRANSMIT_MESSAGE  : Transmit processed message state
 *
 * INFO: The state machine must contain only state machine code! Inside of the
 *       state machine no check/specific function has to be implemeted. This
 *       has to be done in Execute or CheckLeave function.
 *       Entry function has to be used only for preparing the states for actually
 *       set new function.
 *
 * \par Used global variables
 * - \ref sm_NewState    (in,out): State machine new state request variable.
 * - \ref sm_ActualState (in,out): State machine actual state variable.
 *
 * \return void
 */
void ModBus::Slave::TransportLayer::StateHandler( void )
{
    /* This array must have the same order as the state enum! */
    static const sm_StateLinkedRoutines_t stateRoutines [] =
    {
        { STATE_INIT,
            { &ModBus::Slave::TransportLayer::State_Init_Entry,
              &ModBus::Slave::TransportLayer::State_Init_Execute,
              &ModBus::Slave::TransportLayer::State_Init_CheckLeave,
              &ModBus::Slave::TransportLayer::State_Init_Leave                  }  },

        { STATE_STANDBY,
            { &ModBus::Slave::TransportLayer::State_Standby_Entry,
              &ModBus::Slave::TransportLayer::State_Standby_Execute,
              &ModBus::Slave::TransportLayer::State_Standby_CheckLeave,
              &ModBus::Slave::TransportLayer::State_Standby_Leave               }  },

        { STATE_FRAME_CHECK,
            { &ModBus::Slave::TransportLayer::State_FrameCheck_Entry,
              &ModBus::Slave::TransportLayer::State_FrameCheck_Execute,
              &ModBus::Slave::TransportLayer::State_FrameCheck_CheckLeave,
              &ModBus::Slave::TransportLayer::State_FrameCheck_Leave            }  },

        { STATE_PROCESS_REQUEST,
            { &ModBus::Slave::TransportLayer::State_ProcessRequest_Entry,
              &ModBus::Slave::TransportLayer::State_ProcessRequest_Execute,
              &ModBus::Slave::TransportLayer::State_ProcessRequest_CheckLeave,
              &ModBus::Slave::TransportLayer::State_ProcessRequest_Leave        }  },

        { STATE_TRANSMIT_MESSAGE,
            { &ModBus::Slave::TransportLayer::State_TransmitMessage_Entry,
              &ModBus::Slave::TransportLayer::State_TransmitMessage_Execute,
              &ModBus::Slave::TransportLayer::State_TransmitMessage_CheckLeave,
              &ModBus::Slave::TransportLayer::State_TransmitMessage_Leave       }  }
    };

    /* Check if the state is in valid range of available modes, if no set the default/error state. */
    if( STATE_TRANSMIT_MESSAGE < sm_NewState )
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

/*----------------------------------------------------------------------------*/
/*---------------------- Start of state machine functions --------------------*/
/*----------------------------------------------------------------------------*/

/*****************************************************************************************/
/********************************** Initialization state *********************************/
/*****************************************************************************************/
/**
 * \brief Entry part of initialization state
 */
void ModBus::Slave::TransportLayer::State_Init_Entry( void )
{
    return;
}


/**
 * \brief Execute part of initialization state
 */
void ModBus::Slave::TransportLayer::State_Init_Execute( void )
{
    /* Clear Rx buffer address */
    frameDataStruct.FrameData = nullptr;

    /* Reset frame length */
    frameDataStruct.FrameLength = 0u;

    /* Reset frame status */
    frameDataStruct.FrameStatus = FRAME_INACTIVE;

    /* Set state of module */
    moduleState = MODBUS_SLAVE_TL_INIT;
}


/**
 * \brief Check-leave part of initialization state
 */
void ModBus::Slave::TransportLayer::State_Init_CheckLeave( void )
{
    if( RUN_ACTIVE == runStatus )
    {
        sm_NewState = STATE_STANDBY;
    }
    else
    {
        /* Stay in initialization state */
    }
}


/**
 * \brief Leave part of initialization state
 *
 * Physical layer is (re)initialized once, when module becomes active.
 */
void ModBus::Slave::TransportLayer::State_Init_Leave( void )
{
    physicalLayer.PhyInit();
}


/*****************************************************************************************/
/************************************* Standby state *************************************/
/*****************************************************************************************/
void ModBus::Slave::TransportLayer::State_Standby_Entry( void )
{
    /* Reset frame status */
    frameDataStruct.FrameStatus = FRAME_INACTIVE;

    /* Signal processing message */
    physicalLayer.Set_ProcessingInactive();

    /* Prepare DMA for next receive */
    physicalLayer.Set_Reception_Start();

    /* Clear counter value */
    stateCounter = 0u;

    /* Set module state */
    moduleState = MODBUS_SLAVE_TL_STANDBY;
}


void ModBus::Slave::TransportLayer::State_Standby_Execute( void )
{
    modBus_RxStatus_t msgStatus = physicalLayer.Get_Reception_Status();

    /* Clear value of Rx buffer */
    if( MODBUS_RX_READY == msgStatus )
    {
        /* Set flag value */
        frameDataStruct.FrameStatus = FRAME_RECEIVED;
    }
    else
    {
        /* No new message received */
    }

    if( ( modBus_MaxStandbyTime - stateCounter ) > handlerTaskTime )
    {
        stateCounter = stateCounter + handlerTaskTime;
    }
    else
    {
        /* Saturate counter, avoid overflow */
        stateCounter = modBus_MaxStandbyTime;
    }
}


void ModBus::Slave::TransportLayer::State_Standby_CheckLeave( void )
{
    if( RUN_INACTIVE == runStatus )
    {
        sm_NewState = STATE_INIT;
    }
    else if( FRAME_RECEIVED == frameDataStruct.FrameStatus )
    {
        sm_NewState = STATE_FRAME_CHECK;
    }
    else if( modBus_MaxStandbyTime <= stateCounter )
    {
        /* No frame received for long time. Re-initialize physical layer. */
        sm_NewState = STATE_INIT;
    }
    else
    {
        /* Stay in standby state */
    }
}


void ModBus::Slave::TransportLayer::State_Standby_Leave( void )
{
    /* Signal processing message */
    physicalLayer.Set_ProcessingActive();

    /* Stop reception */
    physicalLayer.Set_Reception_Stop();
}


/*****************************************************************************************/
/************************************ CRC check state ************************************/
/*****************************************************************************************/
void ModBus::Slave::TransportLayer::State_FrameCheck_Entry( void )
{
    frameDataStruct.FrameData = physicalLayer.Get_RxMessageBuff();
    frameDataStruct.FrameLength = physicalLayer.Get_RxMessageLen();

    /* Increment count of received frames */
    statsCounter.framesRcvdCnt ++;

    /* Set module state */
    moduleState = MODBUS_SLAVE_TL_PROCESSING;
}


void ModBus::Slave::TransportLayer::State_FrameCheck_Execute( void )
{
    modBus_CrcState_t crcState = MODBUS_CRC_INCORRECT;

    if( ( nullptr              != frameDataStruct.FrameData   ) &&
        ( MODBUS_FRAME_MIN_LEN <= frameDataStruct.FrameLength ) &&
        ( MODBUS_FRAME_MAX_LEN >= frameDataStruct.FrameLength )    )
    {
        /* Calculate CRC value from message */
        crcState = Check_Crc( frameDataStruct.FrameData, frameDataStruct.FrameLength );

        if( MODBUS_CRC_OK != crcState )
        {
            /* CRC does not correspond */
            frameDataStruct.FrameStatus = FRAME_CRC_ERROR;
            /* Increment count of CRC errors */
            statsCounter.crcErrorCount ++;
        }
        else if( ( deviceAddress         != frameDataStruct.FrameData[ 0u ] ) &&
                 ( MODBUS_BROADCAST_ADDR != frameDataStruct.FrameData[ 0u ] )    )
        {
            /* Address does not correspond */
            frameDataStruct.FrameStatus = FRAME_ADDR_ERROR;
            /* Increment count of address errors */
            statsCounter.addrNotMatchedCnt ++;
        }
        else
        {
            /* Calculated CRC correspond with received CRC */
            frameDataStruct.FrameStatus = FRAME_READY_FOR_PROCESS;
            /* Increment count of correct address frames count */
            statsCounter.addrMatchedCnt ++;
        }
    }
    else
    {
        /* Incorrect frame length detected */
        frameDataStruct.FrameStatus = FRAME_CRC_ERROR;

        /* Increment count of CRC errors */
        statsCounter.crcErrorCount ++;
    }
}


void ModBus::Slave::TransportLayer::State_FrameCheck_CheckLeave( void )
{
    if (RUN_INACTIVE == runStatus)
    {
        sm_NewState = STATE_INIT;
    }
    else if( FRAME_READY_FOR_PROCESS == frameDataStruct.FrameStatus )
    {
        /* CRC is corresponding with calculated value, continue with sequence */
        sm_NewState = STATE_PROCESS_REQUEST;
    }
    else
    {
        /* CRC is not corresponding with calculated value */
        sm_NewState = STATE_STANDBY;
    }
}


void ModBus::Slave::TransportLayer::State_FrameCheck_Leave( void )
{
    return;
}


/*****************************************************************************************/
/******************************** Request execution state ********************************/
/*****************************************************************************************/
void ModBus::Slave::TransportLayer::State_ProcessRequest_Entry( void )
{
    /* Signal that frame is ready for processing */
    frameDataStruct.FrameStatus = FRAME_READY_FOR_PROCESS;

    /* Set module state */
    moduleState = MODBUS_SLAVE_TL_EXECUTING;
}


void ModBus::Slave::TransportLayer::State_ProcessRequest_Execute( void )
{
    /* Wait for frame processing finish */
    return;
}


void ModBus::Slave::TransportLayer::State_ProcessRequest_CheckLeave( void )
{
    if( RUN_INACTIVE == runStatus )
    {
        sm_NewState = STATE_INIT;
    }
    else if( FRAME_READY_FOR_TRANSMIT == frameDataStruct.FrameStatus )
    {
        if( 0u != frameDataStruct.FrameLength )
        {
            sm_NewState = STATE_TRANSMIT_MESSAGE;
        }
        else
        {
            /* No response required (broadcast, listen only mode) */
            sm_NewState = STATE_STANDBY;
        }
    }
    else
    {
        /* Wait for frame processing finish */
    }
}


void ModBus::Slave::TransportLayer::State_ProcessRequest_Leave( void )
{
    return;
}


/******************************************************************************/
/********************** Transmit processed message state **********************/
/******************************************************************************/
void ModBus::Slave::TransportLayer::State_TransmitMessage_Entry( void )
{
    /* Calculate CRC value from message */
    Add_Crc( frameDataStruct.FrameData, &frameDataStruct.FrameLength );

    /* Trigger message transmission */
    physicalLayer.Set_Transmission_Start( frameDataStruct.FrameData, frameDataStruct.FrameLength );

    /* Set module state */
    moduleState = MODBUS_SLAVE_TL_SENDING;
}


void ModBus::Slave::TransportLayer::State_TransmitMessage_Execute( void )
{
    modBus_TxStatus_t txStatus = physicalLayer.Get_Transmission_Status();

    if( MODBUS_TX_FINISHED == txStatus )
    {
        frameDataStruct.FrameStatus = FRAME_TRANSMITTED;
    }
    else
    {
        /* Transmission was not yet completed */
    }
}


void ModBus::Slave::TransportLayer::State_TransmitMessage_CheckLeave( void )
{
    if( RUN_INACTIVE == runStatus )
    {
        sm_NewState = STATE_INIT;
    }
    else if( FRAME_TRANSMITTED == frameDataStruct.FrameStatus )
    {
        sm_NewState = STATE_STANDBY;
    }
    else
    {
        /* Wait for transmission complete */
    }
}


void ModBus::Slave::TransportLayer::State_TransmitMessage_Leave( void )
{
    /* Disable frame transmission */
    physicalLayer.Set_Transmission_Stop();

    /* Clear value of Rx buffer */
    frameDataStruct.FrameData = nullptr;

    /* Reset frame length */
    frameDataStruct.FrameLength = 0u;

    /* Reset frame status */
    frameDataStruct.FrameStatus = FRAME_INACTIVE;
}


/* =============================== TASKS ==================================== */

