/**
 * \author Mr.Nobody
 * \file ModBus_App_RWMR.hpp
 * \ingroup ModBus
 * \brief ModBus Read/Write Multiple Registers (RWMR, 0x17) application layer
 *        handler header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RWMR_HPP
#define APPLAYER_MODBUS_APP_RWMR_HPP

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
    class RWMR : public FunctionsHandler
    {
    public:

        RWMR( const modBus_RegistersConfigStruct_t *holdingRegistersHandler, modBus_RegistersCount_t registersCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Read/Write Multiple Registers (RWMR) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RWMR;

        /** Read starting address position in request */
        static constexpr modBus_MsgLength_t readStartAddr_BuffPos = 2u;

        /** Quantity to read position in request */
        static constexpr modBus_MsgLength_t readQuantity_BuffPos = 4u;

        /** Write starting address position in request */
        static constexpr modBus_MsgLength_t writeStartAddr_BuffPos = 6u;

        /** Quantity to write position in request */
        static constexpr modBus_MsgLength_t writeQuantity_BuffPos = 8u;

        /** Write byte count position in request */
        static constexpr modBus_MsgLength_t writeByteCount_BuffPos = 10u;

        /** Write register values start position in request */
        static constexpr modBus_MsgLength_t writeValues_BuffPos = 11u;

        /** Byte count position in response */
        static constexpr modBus_MsgLength_t readByteCount_BuffPos = 2u;

        /** Read register values start position in response */
        static constexpr modBus_MsgLength_t readValues_BuffPos = 3u;

        /** Maximum count of registers to read */
        static constexpr uint16_t maxReadQuantity = 0x007Du;

        /** Maximum count of registers to write */
        static constexpr uint16_t maxWriteQuantity = 0x0079u;

        /** Registers configuration array */
        const modBus_RegistersConfigStruct_t *registersHandler = nullptr;

        /** Count of supported registers. */
        modBus_RegistersCount_t supportedRegistersCount = 0u;

    }; /* class RWMR */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RWMR_HPP */
