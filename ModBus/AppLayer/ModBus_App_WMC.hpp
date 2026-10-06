/**
 * \author Mr.Nobody
 * \file ModBus_App_WMC.hpp
 * \ingroup ModBus
 * \brief ModBus Write Multiple Coils (WMC, 0x0F) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_WMC_HPP
#define APPLAYER_MODBUS_APP_WMC_HPP

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
    class WMC : public FunctionsHandler
    {
    public:

        WMC( const modBus_CoilsConfigStruct_t *coilsHandler, modBus_CoilsCount_t coilsCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Write Multiple Coils (WMC) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_WMC;

        /** Starting address position in request */
        static constexpr modBus_MsgLength_t startingAddr_BuffPos = 2u;

        /** Quantity of outputs position in request */
        static constexpr modBus_MsgLength_t quantity_BuffPos = 4u;

        /** Byte count position in request */
        static constexpr modBus_MsgLength_t byteCount_BuffPos = 6u;

        /** Outputs value start position in request */
        static constexpr modBus_MsgLength_t outputsValue_BuffPos = 7u;

        /** Length of response without CRC */
        static constexpr modBus_MsgLength_t responseLen = 6u;

        /** Maximum count of coils in one request */
        static constexpr uint16_t maxQuantity = 0x07B0u;

        /** Coils configuration array */
        const modBus_CoilsConfigStruct_t *coilsHandler = nullptr;

        /** Count of supported coils. */
        modBus_CoilsCount_t supportedCoilsCount = 0u;

    }; /* class WMC */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_WMC_HPP */
