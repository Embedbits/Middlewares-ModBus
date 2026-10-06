/**
 * \author Mr.Nobody
 * \file ModBus_App_MWR.hpp
 * \ingroup ModBus
 * \brief ModBus Mask Write Register (MWR, 0x16) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_MWR_HPP
#define APPLAYER_MODBUS_APP_MWR_HPP

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
    class MWR : public FunctionsHandler
    {
    public:

        MWR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Mask Write Register (MWR) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_MWR;

        /** Length of request (and response) without CRC */
        static constexpr modBus_MsgLength_t requestLen = 8u;

        /** Reference address position in message buffer */
        static constexpr modBus_MsgLength_t referenceAddr_BuffPos = 2u;

        /** AND mask position in message buffer */
        static constexpr modBus_MsgLength_t andMask_BuffPos = 4u;

        /** OR mask position in message buffer */
        static constexpr modBus_MsgLength_t orMask_BuffPos = 6u;

        /** Registers configuration array */
        const modBus_RegistersConfigStruct_t *registersHandler = nullptr;

        /** Count of supported registers. */
        modBus_RegistersCount_t supportedRegistersCount = 0u;

    }; /* class MWR */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_MWR_HPP */
