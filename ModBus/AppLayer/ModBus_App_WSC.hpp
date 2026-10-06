/**
 * \author Mr.Nobody
 * \file ModBus_App_WSC.hpp
 * \ingroup ModBus
 * \brief ModBus Write Single Coil (WSC, 0x05) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_WSC_HPP
#define APPLAYER_MODBUS_APP_WSC_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class WSC : public FunctionsHandler
    {
    public:

        WSC( const modBus_CoilsConfigStruct_t *coilsHandler, modBus_CoilsCount_t coilsCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Write Single Coil (WSC) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_WSC;

        /** Length of request (and response) without CRC */
        static constexpr modBus_MsgLength_t requestLen = 6u;

        /** Output address position in message buffer */
        static constexpr modBus_MsgLength_t outputAddr_BuffPos = 2u;

        /** Output value position in message buffer */
        static constexpr modBus_MsgLength_t outputValue_BuffPos = 4u;

        /** Output value requesting coil ON */
        static constexpr uint16_t coilOnValue = 0xFF00u;

        /** Output value requesting coil OFF */
        static constexpr uint16_t coilOffValue = 0x0000u;

        /** Coils configuration array */
        const modBus_CoilsConfigStruct_t *coilsHandler = nullptr;

        /** Count of supported coils. */
        modBus_CoilsCount_t supportedCoilsCount = 0u;

    }; /* class WSC */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_WSC_HPP */
