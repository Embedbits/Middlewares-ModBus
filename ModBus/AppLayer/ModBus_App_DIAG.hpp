/**
 * \author Mr.Nobody
 * \file ModBus_App_DIAG.hpp
 * \ingroup ModBus
 * \brief ModBus Diagnostics (DIAG, 0x08) application layer handler header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_DIAG_HPP
#define APPLAYER_MODBUS_APP_DIAG_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
#include "ModBus_App_CommDiag.hpp"      /* Communication diagnostics          */
/* ============================= TYPEDEFS =================================== */

/** \brief Diagnostics sub-function codes */
typedef enum
{
    MODBUS_DIAG_RETURN_QUERY_DATA          = 0x00u, /**< Echo of request data                         */
    MODBUS_DIAG_RESTART_COMM               = 0x01u, /**< Restart communications option                */
    MODBUS_DIAG_RETURN_DIAG_REGISTER       = 0x02u, /**< Return diagnostic register                   */
    MODBUS_DIAG_CHANGE_ASCII_DELIMITER     = 0x03u, /**< Change ASCII input delimiter (ASCII only)    */
    MODBUS_DIAG_FORCE_LISTEN_ONLY          = 0x04u, /**< Force listen only mode                       */
    MODBUS_DIAG_CLEAR_COUNTERS             = 0x0Au, /**< Clear counters and diagnostic register       */
    MODBUS_DIAG_BUS_MESSAGE_COUNT          = 0x0Bu, /**< Return bus message count                     */
    MODBUS_DIAG_BUS_COMM_ERROR_COUNT       = 0x0Cu, /**< Return bus communication error count         */
    MODBUS_DIAG_BUS_EXCEPTION_ERROR_COUNT  = 0x0Du, /**< Return bus exception error count             */
    MODBUS_DIAG_SLAVE_MESSAGE_COUNT        = 0x0Eu, /**< Return slave message count                   */
    MODBUS_DIAG_SLAVE_NO_RESPONSE_COUNT    = 0x0Fu, /**< Return slave no response count               */
    MODBUS_DIAG_SLAVE_NAK_COUNT            = 0x10u, /**< Return slave NAK count                       */
    MODBUS_DIAG_SLAVE_BUSY_COUNT           = 0x11u, /**< Return slave busy count                      */
    MODBUS_DIAG_BUS_CHAR_OVERRUN_COUNT     = 0x12u, /**< Return bus character overrun count           */
    MODBUS_DIAG_CLEAR_OVERRUN              = 0x14u  /**< Clear overrun counter and flag               */
}   modBus_DiagSubFunction_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class DIAG : public FunctionsHandler
    {
    public:

        DIAG( CommDiagnostics &commDiagnostics );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Diagnostics (DIAG) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_DIAG;

        /** Sub-function position in message buffer */
        static constexpr modBus_MsgLength_t subFunction_BuffPos = 2u;

        /** Data field position in message buffer */
        static constexpr modBus_MsgLength_t data_BuffPos = 4u;

        /** Length of request with one data word (without CRC) */
        static constexpr modBus_MsgLength_t requestLen = 6u;

        /** Data field value of Restart Communications Option clearing event log */
        static constexpr uint16_t restartClearLogData = 0xFF00u;

        /** Communication diagnostics */
        CommDiagnostics &diagnostics;

        bool Get_CounterValue( uint16_t subFunction, uint16_t &counterValue ) const;

    }; /* class DIAG */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_DIAG_HPP */
