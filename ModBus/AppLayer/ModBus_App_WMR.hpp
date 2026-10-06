/**
 * \author Mr.Nobody
 * \file ModBus_App_WMR.hpp
 * \ingroup ModBus
 * \brief ModBus Write Multiple Registers (WMR, 0x10) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_WMR_HPP
#define APPLAYER_MODBUS_APP_WMR_HPP

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
    class WMR : public FunctionsHandler
    {
    public:

        WMR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Write Multiple Registers (WMR) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_WMR;

        /** Register starting address position in request */
        static constexpr modBus_MsgLength_t startingAddr_BuffPos = 2u;

        /** Quantity of registers position in request */
        static constexpr modBus_MsgLength_t quantity_BuffPos = 4u;

        /** Byte count position in request */
        static constexpr modBus_MsgLength_t byteCount_BuffPos = 6u;

        /** Register values start position in request */
        static constexpr modBus_MsgLength_t registerValues_BuffPos = 7u;

        /** Length of response without CRC */
        static constexpr modBus_MsgLength_t responseLen = 6u;

        /** Maximum count of registers in one request */
        static constexpr uint16_t maxQuantity = 0x007Bu;

        /** Registers configuration array */
        const modBus_RegistersConfigStruct_t *registersHandler = nullptr;

        /** Count of supported registers. */
        modBus_RegistersCount_t supportedRegistersCount = 0u;

    }; /* class WMR */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_WMR_HPP */
