/**
 * \author Mr.Nobody
 * \file ModBus_App_GCEC.hpp
 * \ingroup ModBus
 * \brief ModBus Get Communication Event Counter (GCEC, 0x0B) application layer
 *        handler header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_GCEC_HPP
#define APPLAYER_MODBUS_APP_GCEC_HPP

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
    class GCEC : public FunctionsHandler
    {
    public:

        GCEC( CommDiagnostics &commDiagnostics );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Get Communication Event Counter (GCEC) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_GCEC;

        /** Length of request without CRC */
        static constexpr modBus_MsgLength_t requestLen = 2u;

        /** Status word position in response */
        static constexpr modBus_MsgLength_t status_BuffPos = 2u;

        /** Event count position in response */
        static constexpr modBus_MsgLength_t eventCount_BuffPos = 4u;

        /** Length of response without CRC */
        static constexpr modBus_MsgLength_t responseLen = 6u;

        /** Status word - no program command is processed (device is never busy) */
        static constexpr uint16_t statusNotBusy = 0x0000u;

        /** Communication diagnostics */
        CommDiagnostics &diagnostics;

    }; /* class GCEC */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_GCEC_HPP */
