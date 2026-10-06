/**
 * \author Mr.Nobody
 * \file ModBus_App_RFIFO.hpp
 * \ingroup ModBus
 * \brief ModBus Read FIFO Queue (RFIFO, 0x18) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RFIFO_HPP
#define APPLAYER_MODBUS_APP_RFIFO_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/**
 * \brief Callback providing content of FIFO queue
 *
 * \param fifoAddress [in]: FIFO pointer address from request
 * \param values     [out]: Queue values. At most \ref MODBUS_RFIFO_MAX_COUNT
 *                          values can be written.
 * \param fifoCount  [out]: Count of values in queue. If it is higher than
 *                          \ref MODBUS_RFIFO_MAX_COUNT, exception 0x03 is sent.
 *
 * \return \ref MODBUS_REQUEST_OK, \ref MODBUS_REQUEST_ERROR if FIFO address is
 *         not supported (exception 0x02)
 */
typedef modBus_RequestState_t ( modBus_FifoCallback_t ) ( uint16_t fifoAddress,
                                                          modBus_RegistersValue_t *values,
                                                          uint16_t *fifoCount );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Maximal count of values in FIFO queue response */
#define MODBUS_RFIFO_MAX_COUNT                                ( 31u )

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class RFIFO : public FunctionsHandler
    {
    public:

        RFIFO( modBus_FifoCallback_t *fifoCallback );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Read FIFO Queue (RFIFO) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RFIFO;

        /** Length of request without CRC */
        static constexpr modBus_MsgLength_t requestLen = 4u;

        /** FIFO pointer address position in request */
        static constexpr modBus_MsgLength_t fifoAddr_BuffPos = 2u;

        /** Byte count position in response */
        static constexpr modBus_MsgLength_t byteCount_BuffPos = 2u;

        /** FIFO count position in response */
        static constexpr modBus_MsgLength_t fifoCount_BuffPos = 4u;

        /** FIFO values start position in response */
        static constexpr modBus_MsgLength_t fifoValues_BuffPos = 6u;

        /** Callback providing FIFO content */
        modBus_FifoCallback_t *fifoCallback = nullptr;

    }; /* class RFIFO */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RFIFO_HPP */
