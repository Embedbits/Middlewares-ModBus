/**
 * \author Mr.Nobody
 * \file ModBus_App_GCEL.cpp
 * \ingroup ModBus
 * \brief ModBus Get Communication Event Log (GCEL, 0x0C) application layer
 *        handler
 *
 * # (0x0C) Get Comm Event Log (Serial Line only) #
 *
 * This function code is used to get a status word, event count, message count,
 * and a field of event bytes from the remote device. The status word and event
 * count are identical to that returned by Get Communications Event Counter
 * (0x0B). The message count is identical to Diagnostics - Return Bus Message
 * Count (0x0B). The event bytes field contains 0-64 bytes, byte 0 is the most
 * recent event (see MODBUS_COMM_EVENT_* in ModBus_App_CommDiag.hpp).
 *
 * | Request               | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x0C             |
 *
 * | Response              | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x0C             |
 * | Byte Count            | 1 Byte   | 6 + N            |
 * | Status                | 2 Bytes  | 0x0000 or 0xFFFF |
 * | Event Count           | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Message Count         | 2 Bytes  | 0x0000 to 0xFFFF |
 * | Events                | N Bytes  | N = 0 to 64      |
 *
 * Exceptions: 0x03 (request length).
 *
 * Example: request `0C`, response `0C 08 00 00 01 08 01 21 20 00`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_GCEL.hpp"          /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Get Communication Event Log handler constructor
 *
 * \param commDiagnostics [in]: Communication diagnostics (shared with AppCore)
 */
ModBus::AppLayer::GCEL::GCEL( CommDiagnostics &commDiagnostics ) :
    diagnostics( commDiagnostics )
{
    return;
}


/**
 * \brief Processes Get Communication Event Log (GCEL) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::GCEL::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint8_t eventsCnt = diagnostics.Get_EventLog( &msgBuffer[ events_BuffPos ], MODBUS_COMM_EVENT_LOG_SIZE );

        msgBuffer[ byteCount_BuffPos ] = (uint8_t)( countersLen + eventsCnt );

        Set_16b( msgBuffer, status_BuffPos,       statusNotBusy );
        Set_16b( msgBuffer, eventCount_BuffPos,   diagnostics.Get_EventCounter() );
        Set_16b( msgBuffer, messageCount_BuffPos, diagnostics.Get_BusMessageCount() );

        msgLength = events_BuffPos + eventsCnt;
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
