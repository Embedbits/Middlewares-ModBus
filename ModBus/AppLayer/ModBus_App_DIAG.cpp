/**
 * \author Mr.Nobody
 * \file ModBus_App_DIAG.cpp
 * \ingroup ModBus
 * \brief ModBus Diagnostics (DIAG, 0x08) application layer handler
 *
 * # (0x08) Diagnostics (Serial Line only) #
 *
 * Function code 08 provides a series of tests for checking the communication
 * system between a client device and a server. The sub-function code field
 * in the query defines the type of test to be performed. Normal response to
 * all sub-functions (except Force Listen Only Mode, which has no response)
 * contains the sub-function code and data field.
 *
 * | Request / Response    | Size     | Value            |
 * |-----------------------|----------|------------------|
 * | Function code         | 1 Byte   | 0x08             |
 * | Sub-function          | 2 Bytes  |                  |
 * | Data                  | N x 2 B  |                  |
 *
 * | Sub-function | Name                              | Request data | Response data      |
 * |--------------|-----------------------------------|--------------|--------------------|
 * | 0x00         | Return Query Data                 | any          | echo               |
 * | 0x01         | Restart Communications Option     | 0000 / FF00  | echo               |
 * | 0x02         | Return Diagnostic Register        | 0000         | register           |
 * | 0x04         | Force Listen Only Mode            | 0000         | no response        |
 * | 0x0A         | Clear Counters and Diag. Register | 0000         | echo               |
 * | 0x0B         | Return Bus Message Count          | 0000         | counter            |
 * | 0x0C         | Return Bus Comm. Error Count      | 0000         | counter            |
 * | 0x0D         | Return Bus Exception Error Count  | 0000         | counter            |
 * | 0x0E         | Return Server Message Count       | 0000         | counter            |
 * | 0x0F         | Return Server No Response Count   | 0000         | counter            |
 * | 0x10         | Return Server NAK Count           | 0000         | counter            |
 * | 0x11         | Return Server Busy Count          | 0000         | counter            |
 * | 0x12         | Return Bus Character Overrun Count| 0000         | counter            |
 * | 0x14         | Clear Overrun Counter and Flag    | 0000         | echo               |
 *
 * Restart Communications Option leaves Listen Only Mode and clears counters,
 * data FF00 clears also communication event log. It is the only request
 * processed in Listen Only Mode (no response is sent in that case).
 *
 * Exceptions: 0x01 (unsupported sub-function, incl. 0x03 - ASCII only),
 * 0x03 (data field, request length).
 *
 * Example - Return Query Data: request `08 00 00 A5 37`, response `08 00 00 A5 37`.
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App_DIAG.hpp"          /* Self include                       */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Diagnostics handler constructor
 *
 * \param commDiagnostics [in]: Communication diagnostics (shared with AppCore)
 */
ModBus::AppLayer::DIAG::DIAG( CommDiagnostics &commDiagnostics ) :
    diagnostics( commDiagnostics )
{
    return;
}


/**
 * \brief Processes Diagnostics (DIAG) request
 *
 * \param msgBuffer [in,out]: Request, overwritten by response
 * \param msgLength [in,out]: Length of request / response without CRC
 *
 * \return Exception code or \ref MODBUS_ERROR_NO_ACTIVE_ERROR
 */
modBus_ErrorStatus_t ModBus::AppLayer::DIAG::SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength )
{
    modBus_ErrorStatus_t errorStatus = MODBUS_ERROR_NO_ACTIVE_ERROR;

    if( ( nullptr == msgBuffer ) || ( data_BuffPos > msgLength ) )
    {
        errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
    }
    else
    {
        const uint16_t subFunction  = Get_16b( msgBuffer, subFunction_BuffPos );
        uint16_t       counterValue = 0u;

        if( MODBUS_DIAG_RETURN_QUERY_DATA == subFunction )
        {
            /* Response is echo of request (any data length) */
        }
        else if( requestLen != msgLength )
        {
            /* All other sub-functions have exactly one data word */
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else if( MODBUS_DIAG_RESTART_COMM == subFunction )
        {
            const uint16_t requestData = Get_16b( msgBuffer, data_BuffPos );

            if( ( 0x0000u != requestData ) && ( restartClearLogData != requestData ) )
            {
                errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
            }
            else
            {
                diagnostics.Restart( restartClearLogData == requestData );
            }
        }
        else if( ( MODBUS_DIAG_RETURN_DIAG_REGISTER != subFunction                   ) &&
                 ( MODBUS_DIAG_FORCE_LISTEN_ONLY    != subFunction                   ) &&
                 ( MODBUS_DIAG_CLEAR_COUNTERS       != subFunction                   ) &&
                 ( MODBUS_DIAG_CLEAR_OVERRUN        != subFunction                   ) &&
                 ( false == Get_CounterValue( subFunction, counterValue )            )    )
        {
            /* Sub-function is not supported (incl. Change ASCII Input Delimiter) */
            errorStatus = MODBUS_ERROR_ILLEGAL_FUNCTION;
        }
        else if( 0x0000u != Get_16b( msgBuffer, data_BuffPos ) )
        {
            /* Data field of all following sub-functions shall be 0x0000 */
            errorStatus = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        else if( MODBUS_DIAG_RETURN_DIAG_REGISTER == subFunction )
        {
            Set_16b( msgBuffer, data_BuffPos, diagnostics.Get_DiagnosticRegister() );
        }
        else if( MODBUS_DIAG_FORCE_LISTEN_ONLY == subFunction )
        {
            /* No response is sent, application core checks Listen Only Mode */
            diagnostics.Set_ListenOnlyMode();
        }
        else if( MODBUS_DIAG_CLEAR_COUNTERS == subFunction )
        {
            diagnostics.Clear_Counters();
        }
        else if( MODBUS_DIAG_CLEAR_OVERRUN == subFunction )
        {
            diagnostics.Clear_OverrunCounter();
        }
        else
        {
            /* Counter value read by supported sub-function check */
            Set_16b( msgBuffer, data_BuffPos, counterValue );
        }

        /* Response has the same length as request, msgLength is not changed */
    }

    return ( errorStatus );
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Returns value of counter requested by sub-function
 *
 * \param subFunction   [in]: Diagnostics sub-function
 * \param counterValue [out]: Value of counter
 *
 * \return True if sub-function returns counter
 */
bool ModBus::AppLayer::DIAG::Get_CounterValue( uint16_t subFunction, uint16_t &counterValue ) const
{
    bool isCounter = true;

    switch( subFunction )
    {
        case MODBUS_DIAG_BUS_MESSAGE_COUNT:
            counterValue = diagnostics.Get_BusMessageCount();
            break;

        case MODBUS_DIAG_BUS_COMM_ERROR_COUNT:
            counterValue = diagnostics.Get_BusCommErrorCount();
            break;

        case MODBUS_DIAG_BUS_EXCEPTION_ERROR_COUNT:
            counterValue = diagnostics.Get_BusExceptionErrorCount();
            break;

        case MODBUS_DIAG_SLAVE_MESSAGE_COUNT:
            counterValue = diagnostics.Get_SlaveMessageCount();
            break;

        case MODBUS_DIAG_SLAVE_NO_RESPONSE_COUNT:
            counterValue = diagnostics.Get_SlaveNoResponseCount();
            break;

        case MODBUS_DIAG_SLAVE_NAK_COUNT:
            counterValue = diagnostics.Get_SlaveNakCount();
            break;

        case MODBUS_DIAG_SLAVE_BUSY_COUNT:
            counterValue = diagnostics.Get_SlaveBusyCount();
            break;

        case MODBUS_DIAG_BUS_CHAR_OVERRUN_COUNT:
            counterValue = diagnostics.Get_BusCharOverrunCount();
            break;

        default:
            isCounter = false;
            break;
    }

    return ( isCounter );
}

/* =============================== TASKS ==================================== */
