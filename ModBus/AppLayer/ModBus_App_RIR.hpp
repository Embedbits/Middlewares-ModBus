/**
 * \author Mr.Nobody
 * \file ModBus_App_RIR.hpp
 * \ingroup ModBus
 * \brief ModBus Read Input Registers (RIR, 0x04) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RIR_HPP
#define APPLAYER_MODBUS_APP_RIR_HPP

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
    class RIR : public FunctionsHandler
    {
    public:

        RIR( const modBus_RegistersConfigStruct_t *inputRegistersHandler, modBus_RegistersCount_t registersCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Read Input Registers (RIR) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RIR;

        /** Registers configuration array */
        const modBus_RegistersConfigStruct_t *registersHandler = nullptr;

        /** Count of supported registers. */
        modBus_RegistersCount_t supportedRegistersCount = 0u;

    }; /* class RIR */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RIR_HPP */
