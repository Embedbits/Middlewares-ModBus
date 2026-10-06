/**
 * \author Mr.Nobody
 * \file ModBus_App_GCEL.hpp
 * \ingroup ModBus
 * \brief ModBus Get Communication Event Log (GCEL, 0x0C) application layer
 *        handler header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_GCEL_HPP
#define APPLAYER_MODBUS_APP_GCEL_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
#include "ModBus_App_CommDiag.hpp"      /* Communication diagnostics          */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class GCEL : public FunctionsHandler
    {
    public:

        GCEL( CommDiagnostics &commDiagnostics );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Get Communication Event Log (GCEL) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_GCEL;

        /** Length of request without CRC */
        static constexpr modBus_MsgLength_t requestLen = 2u;

        /** Byte count position in response */
        static constexpr modBus_MsgLength_t byteCount_BuffPos = 2u;

        /** Status word position in response */
        static constexpr modBus_MsgLength_t status_BuffPos = 3u;

        /** Event count position in response */
        static constexpr modBus_MsgLength_t eventCount_BuffPos = 5u;

        /** Message count position in response */
        static constexpr modBus_MsgLength_t messageCount_BuffPos = 7u;

        /** Events position in response */
        static constexpr modBus_MsgLength_t events_BuffPos = 9u;

        /** Count of bytes of status, event count and message count */
        static constexpr uint8_t countersLen = 6u;

        /** Status word - no program command is processed (device is never busy) */
        static constexpr uint16_t statusNotBusy = 0x0000u;

        /** Communication diagnostics */
        CommDiagnostics &diagnostics;

    }; /* class GCEL */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_GCEL_HPP */
