/**
 * \author Mr.Nobody
 * \file ModBus_App_GCEC.cpp
 * \ingroup ModBus
 * \brief ModBus Get Communication Event Counter (GCEC, 0x0B) application layer
 *        handler
 *
 * # (0x0B) Get Comm Event Counter (Serial Line only) #
 *
 * This function code is used to get a status word and an event count from the
 * remote device's communication event counter. The event counter is
 * incremented once for each successful message completion. It is not
 * incremented for exception responses or fetch event counter / log commands.
 * The counter is reset by Diagnostics sub-functions Restart Communications
 * Option (0x01) and Clear Counters and Diagnostic Register (0x0A). The status
 * word is 0x0000, because the device never processes long program commands.
 *
 * | Request               | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x0B             |
 *
 * | Response              | Size    | Value            |
 * |-----------------------|---------|------------------|
 * | Function code         | 1 Byte  | 0x0B             |
 * | Status                | 2 Bytes | 0x0000 or 0xFFFF |
 * | Event Count           | 2 Bytes | 0x0000 to 0xFFFF |
 *
 * Exceptions: 0x03 (request length).
 *
 * Example - 264 events: request `0B`, response `0B 00 00 01 08`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_GCEC.hpp"          /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Get Communication Event Counter handler constructor
 *
 * \param commDiagnostics [in]: Communication diagnostics (shared with AppCore)
 */
ModBus::AppLayer::GCEC::GCEC( CommDiagnostics &commDiagnostics ) :
    diagnostics( commDiagnostics )
{
    return;
}


/**
 * \brief Processes Get Communication Event Counter (GCEC) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::GCEC::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( requestLen != msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        Set_16b( msgBuffer, status_BuffPos, statusNotBusy );
        Set_16b( msgBuffer, eventCount_BuffPos, diagnostics.Get_EventCounter() );

        msgLength = responseLen;
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/* =============================== TASKS ==================================== */
