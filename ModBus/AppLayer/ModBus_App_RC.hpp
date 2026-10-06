/**
 * \author Mr.Nobody
 * \file ModBus_App_RC.hpp
 * \ingroup ModBus
 * \brief ModBus Read Coils (RC, 0x01) application layer handler header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RC_HPP
#define APPLAYER_MODBUS_APP_RC_HPP

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
    class RC : public FunctionsHandler
    {
    public:

        RC( const modBus_CoilsConfigStruct_t *coilsHandler, modBus_CoilsCount_t coilsCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Read Coils (RC) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RC;

        /** Coils configuration array */
        const modBus_CoilsConfigStruct_t *coilsHandler = nullptr;

        /** Count of supported coils. */
        modBus_CoilsCount_t supportedCoilsCount = 0u;

    }; /* class RC */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RC_HPP */
