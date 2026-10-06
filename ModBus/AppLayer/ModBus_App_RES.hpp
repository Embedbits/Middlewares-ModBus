/**
 * \author Mr.Nobody
 * \file ModBus_App_RES.hpp
 * \ingroup ModBus
 * \brief ModBus Read Exception Status (RES, 0x07) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RES_HPP
#define APPLAYER_MODBUS_APP_RES_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/**
 * \brief Callback providing device specific exception status outputs
 *
 * \param exceptionStatus [out]: Eight exception status outputs (bit 0 is the
 *                               lowest output)
 *
 * \return \ref MODBUS_REQUEST_OK, \ref MODBUS_REQUEST_ERROR results in
 *         exception 0x04
 */
typedef modBus_RequestState_t ( modBus_ExceptionStatusCallback_t ) ( uint8_t *exceptionStatus );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class RES : public FunctionsHandler
    {
    public:

        RES( modBus_ExceptionStatusCallback_t *statusCallback );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Read Exception Status (RES) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RES;

        /** Length of request without CRC */
        static constexpr modBus_MsgLength_t requestLen = 2u;

        /** Output data position in response */
        static constexpr modBus_MsgLength_t outputData_BuffPos = 2u;

        /** Length of response without CRC */
        static constexpr modBus_MsgLength_t responseLen = 3u;

        /** Callback providing exception status */
        modBus_ExceptionStatusCallback_t *statusCallback = nullptr;

    }; /* class RES */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RES_HPP */
