/**
 * \author Mr.Nobody
 * \file ModBus_App_CommDiag.cpp
 * \ingroup ModBus
 * \brief ModBus application layer communication diagnostics (serial line).
 *
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_CommDiag.hpp"      /* Self include                       */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Communication diagnostics constructor
 *
 * \param transportLayerObj [in]: Slave transport layer providing bus counters
 */
ModBus::AppLayer::CommDiagnostics::CommDiagnostics( ModBus::Slave::TransportLayer &transportLayerObj ) :
    transportLayer( transportLayerObj )
{
    const modBus_StatsCounter_t tlCounters = transportLayer.Get_StatCounter();

    busMessageBase     = tlCounters.framesRcvdCnt;
    busCommErrorBase   = tlCounters.crcErrorCount;
    lastCommErrorCount = tlCounters.crcErrorCount;
}


/**
 * \brief Cyclic update - logs receive events of frames with communication error
 *
 * Called by application core in every task.
 */
void ModBus::AppLayer::CommDiagnostics::Update( void )
{
    const modBus_StatsCounter_t tlCounters = transportLayer.Get_StatCounter();

    /* Transport layer counters can be reset by application */
    if( tlCounters.crcErrorCount < lastCommErrorCount )
    {
        lastCommErrorCount = tlCounters.crcErrorCount;
    }
    else
    {
        /* Counter not reset */
    }

    while( tlCounters.crcErrorCount > lastCommErrorCount )
    {
        uint8_t eventByte = MODBUS_COMM_EVENT_RCV | MODBUS_COMM_EVENT_RCV_COMM_ERROR;

        if( true == listenOnlyMode )
        {
            eventByte |= MODBUS_COMM_EVENT_RCV_LISTEN_ONLY;
        }
        else
        {
            /* Normal mode */
        }

        AddEvent( eventByte );

        lastCommErrorCount++;
    }
}


/**
 * \brief Signals reception of request addressed to the slave (or broadcast)
 *
 * \param slaveAddr [in]: Address in received request
 */
void ModBus::AppLayer::CommDiagnostics::Set_RequestReceived( modBus_DeviceAddr_t slaveAddr )
{
    uint8_t eventByte = MODBUS_COMM_EVENT_RCV;

    if( MODBUS_BROADCAST_ADDR == slaveAddr )
    {
        eventByte |= MODBUS_COMM_EVENT_RCV_BROADCAST;
    }
    else
    {
        /* Request addressed directly to slave */
    }

    if( true == listenOnlyMode )
    {
        eventByte |= MODBUS_COMM_EVENT_RCV_LISTEN_ONLY;
    }
    else
    {
        /* Normal mode */
    }

    slaveMessageCount++;

    AddEvent( eventByte );
}


/**
 * \brief Signals end of request processing
 *
 * \param functionCode [in]: Function code of request
 * \param errorStatus  [in]: Result of processing (exception code)
 * \param responseSent [in]: True if (normal or exception) response is sent
 */
void ModBus::AppLayer::CommDiagnostics::Set_RequestProcessed( modBus_FunctionCode_t functionCode,
                                                              modBus_ErrorStatus_t errorStatus,
                                                              bool responseSent )
{
    if( MODBUS_ERROR_NO_ACTIVE_ERROR == errorStatus )
    {
        /* Event counter is not incremented by fetch event counter / log commands */
        if( ( MODBUS_FUNCTION_GCEC != functionCode ) &&
            ( MODBUS_FUNCTION_GCEL != functionCode )    )
        {
            eventCounter++;
        }
        else
        {
            /* Fetch command */
        }
    }
    else if( true == responseSent )
    {
        busExceptionErrorCount++;

        if( MODBUS_ERROR_NEGATIVE_ACKNOWLEDGE == errorStatus )
        {
            slaveNakCount++;
        }
        else if( MODBUS_ERROR_SLAVE_DEVICE_BUSY == errorStatus )
        {
            slaveBusyCount++;
        }
        else
        {
            /* Other exception */
        }
    }
    else
    {
        /* Exception is not sent */
    }

    if( true == responseSent )
    {
        uint8_t eventByte = MODBUS_COMM_EVENT_SEND;

        switch( errorStatus )
        {
            case MODBUS_ERROR_ILLEGAL_FUNCTION:
            case MODBUS_ERROR_ILLEGAL_DATA_ADDRESS:
            case MODBUS_ERROR_ILLEGAL_DATA_VALUE:
                eventByte |= MODBUS_COMM_EVENT_SEND_READ_EXCEPTION;
                break;

            case MODBUS_ERROR_SLAVE_DEVICE_FAILURE:
                eventByte |= MODBUS_COMM_EVENT_SEND_SLAVE_ABORT;
                break;

            case MODBUS_ERROR_ACKNOWLEDGE:
            case MODBUS_ERROR_SLAVE_DEVICE_BUSY:
                eventByte |= MODBUS_COMM_EVENT_SEND_SLAVE_BUSY;
                break;

            case MODBUS_ERROR_NEGATIVE_ACKNOWLEDGE:
                eventByte |= MODBUS_COMM_EVENT_SEND_SLAVE_NAK;
                break;

            default:
                /* Normal response or other exception */
                break;
        }

        AddEvent( eventByte );
    }
    else
    {
        slaveNoResponseCount++;
    }
}


/**
 * \brief Signals request ignored in Listen Only Mode (not processed, no response)
 */
void ModBus::AppLayer::CommDiagnostics::Set_RequestIgnored( void )
{
    slaveNoResponseCount++;
}


/**
 * \brief Returns state of Listen Only Mode
 *
 * \return True if slave is in Listen Only Mode (no responses are sent)
 */
bool ModBus::AppLayer::CommDiagnostics::Get_ListenOnlyMode( void ) const
{
    return ( listenOnlyMode );
}


/**
 * \brief Activates Listen Only Mode (Diagnostics sub-function 0x04)
 *
 * Slave stops to respond. Only Restart Communications Option request is
 * processed.
 */
void ModBus::AppLayer::CommDiagnostics::Set_ListenOnlyMode( void )
{
    listenOnlyMode = true;

    AddEvent( MODBUS_COMM_EVENT_LISTEN_ONLY_ENTERED );
}


/**
 * \brief Restart of communication (Diagnostics sub-function 0x01)
 *
 * Communication counters are cleared and Listen Only Mode is left.
 *
 * \param clearEventLog [in]: True if communication event log shall be cleared
 */
void ModBus::AppLayer::CommDiagnostics::Restart( bool clearEventLog )
{
    const uint16_t actDiagRegister = diagnosticRegister;

    Clear_Counters();

    /* Diagnostic register is not part of communication counters */
    diagnosticRegister = actDiagRegister;

    if( true == clearEventLog )
    {
        eventLogHead  = 0u;
        eventLogCount = 0u;
    }
    else
    {
        /* Event log is kept */
    }

    listenOnlyMode = false;

    AddEvent( MODBUS_COMM_EVENT_COMM_RESTART );
}


/**
 * \brief Clears all counters and diagnostic register (Diagnostics sub-function 0x0A)
 */
void ModBus::AppLayer::CommDiagnostics::Clear_Counters( void )
{
    const modBus_StatsCounter_t tlCounters = transportLayer.Get_StatCounter();

    busMessageBase   = tlCounters.framesRcvdCnt;
    busCommErrorBase = tlCounters.crcErrorCount;

    busExceptionErrorCount = 0u;
    slaveMessageCount      = 0u;
    slaveNoResponseCount   = 0u;
    slaveNakCount          = 0u;
    slaveBusyCount         = 0u;
    eventCounter           = 0u;
    diagnosticRegister     = 0u;
}


/**
 * \brief Clears overrun counter (Diagnostics sub-function 0x14)
 *
 * Character overrun is not detected, thus nothing is cleared.
 */
void ModBus::AppLayer::CommDiagnostics::Clear_OverrunCounter( void )
{
    return;
}


/** \brief Returns count of frames detected on bus (incl. frames with error) */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_BusMessageCount( void ) const
{
    return ( (modBus_EventCount_t)( transportLayer.Get_StatCounter().framesRcvdCnt - busMessageBase ) );
}


/** \brief Returns count of frames with CRC error */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_BusCommErrorCount( void ) const
{
    return ( (modBus_EventCount_t)( transportLayer.Get_StatCounter().crcErrorCount - busCommErrorBase ) );
}


/** \brief Returns count of exception responses sent */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_BusExceptionErrorCount( void ) const
{
    return ( busExceptionErrorCount );
}


/** \brief Returns count of requests addressed to slave (incl. broadcast) */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_SlaveMessageCount( void ) const
{
    return ( slaveMessageCount );
}


/** \brief Returns count of requests without response (broadcast, Listen Only Mode) */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_SlaveNoResponseCount( void ) const
{
    return ( slaveNoResponseCount );
}


/** \brief Returns count of Negative Acknowledge exception responses */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_SlaveNakCount( void ) const
{
    return ( slaveNakCount );
}


/** \brief Returns count of Slave Device Busy exception responses */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_SlaveBusyCount( void ) const
{
    return ( slaveBusyCount );
}


/** \brief Returns count of character overruns (not detected, always 0) */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_BusCharOverrunCount( void ) const
{
    return ( 0u );
}


/** \brief Returns communication event counter (successfully completed requests) */
modBus_EventCount_t ModBus::AppLayer::CommDiagnostics::Get_EventCounter( void ) const
{
    return ( eventCounter );
}


/**
 * \brief Copies communication event log, the most recent event first
 *
 * \param eventBuffer [out]: Buffer for events
 * \param bufferSize   [in]: Size of buffer
 *
 * \return Count of events stored in buffer
 */
uint8_t ModBus::AppLayer::CommDiagnostics::Get_EventLog( uint8_t * const eventBuffer, uint8_t bufferSize ) const
{
    uint8_t eventsCnt = 0u;

    if( nullptr != eventBuffer )
    {
        eventsCnt = ( bufferSize < eventLogCount ) ? bufferSize : eventLogCount;

        for( uint8_t eventIndex = 0u; eventsCnt > eventIndex; eventIndex++ )
        {
            const uint8_t logPos = (uint8_t)( ( eventLogHead + MODBUS_COMM_EVENT_LOG_SIZE - 1u - eventIndex ) %
                                              MODBUS_COMM_EVENT_LOG_SIZE );

            eventBuffer[ eventIndex ] = eventLog[ logPos ];
        }
    }
    else
    {
        /* Null pointer assigned */
    }

    return ( eventsCnt );
}


/**
 * \brief Sets content of diagnostic register (Diagnostics sub-function 0x02)
 *
 * \param regValue [in]: Application specific diagnostic register value
 */
void ModBus::AppLayer::CommDiagnostics::Set_DiagnosticRegister( uint16_t regValue )
{
    diagnosticRegister = regValue;
}


/** \brief Returns content of diagnostic register */
uint16_t ModBus::AppLayer::CommDiagnostics::Get_DiagnosticRegister( void ) const
{
    return ( diagnosticRegister );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Stores event into event log. The oldest event is overwritten.
 *
 * \param eventByte [in]: Event to be stored
 */
void ModBus::AppLayer::CommDiagnostics::AddEvent( uint8_t eventByte )
{
    eventLog[ eventLogHead ] = eventByte;

    eventLogHead = (uint8_t)( ( eventLogHead + 1u ) % MODBUS_COMM_EVENT_LOG_SIZE );

    if( MODBUS_COMM_EVENT_LOG_SIZE > eventLogCount )
    {
        eventLogCount++;
    }
    else
    {
        /* Log is full, the oldest event was overwritten */
    }
}

/* =============================== TASKS ==================================== */
