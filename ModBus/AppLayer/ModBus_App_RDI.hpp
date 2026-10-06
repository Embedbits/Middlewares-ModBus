/**
 * \author Mr.Nobody
 * \file ModBus_App_RDI.hpp
 * \ingroup ModBus
 * \brief ModBus Read Discrete Inputs (RDI, 0x02) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RDI_HPP
#define APPLAYER_MODBUS_APP_RDI_HPP

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
    class RDI : public FunctionsHandler
    {
    public:

        RDI( const modBus_DiscreteIoConfigStruct_t *discreteIoHandler, modBus_DiscreteIoCount_t discreteIoCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Read Discrete Inputs (RDI) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RDI;

        /** Discrete inputs configuration array */
        const modBus_DiscreteIoConfigStruct_t *discreteIoHandler = nullptr;

        /** Count of supported discrete inputs. */
        modBus_DiscreteIoCount_t supportedDiscreteIoCount = 0u;

    }; /* class RDI */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RDI_HPP */
