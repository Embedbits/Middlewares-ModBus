/**
 * \author Mr.Nobody
 * \file ModBus_App.hpp
 * \ingroup ModBus
 * \brief ModBus application layer core (slave) header file
 *
 */

#ifndef APPLAYER_MODBUS_APP_HPP
#define APPLAYER_MODBUS_APP_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
#include "ModBus_App_CommDiag.hpp"      /* Communication diagnostics          */
#include "ModBus_Slave_TL.hpp"          /* Transport Layer handler include    */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
namespace Slave
{
    /**
     * \brief Application layer core of slave device
     *
     * Dispatches received requests to registered function handlers and passes
     * response to transport layer:
     * - Request with unsupported function code is answered by exception
     *   \ref MODBUS_ERROR_ILLEGAL_FUNCTION.
     * - Error returned by handler is answered by exception response.
     * - Broadcast request (address 0) is processed, but no response is sent.
     * - In Listen Only Mode (see \ref ModBus::AppLayer::CommDiagnostics) only
     *   Diagnostics - Restart Communications Option is processed and no
     *   response is sent.
     */
    class AppCore
    {
    public:
        AppCore( ModBus::Slave::TransportLayer &transportLayerObj,
                 ModBus::FunctionsHandler * const functionHandlers[],
                 uint8_t functionsCount,
                 ModBus::AppLayer::CommDiagnostics *commDiagnostics = nullptr );

        void Task( modBus_Time_t taskTime );

    private:

        /** Transport layer object reference */
        ModBus::Slave::TransportLayer &transportLayer;

        /** Pointer to array of function handlers */
        ModBus::FunctionsHandler * const *functionHandlers;

        /** Communication diagnostics (optional) */
        ModBus::AppLayer::CommDiagnostics *diagnostics;

        /** Bit mask for signaling error during processing */
        static constexpr uint8_t errorMask = 0x80u;

        /** Buffer position in message buffer of slave address */
        static constexpr uint8_t slaveAddr_BuffPos = 0u;

        /** Buffer position in message buffer of function code */
        static constexpr uint8_t functionCode_BuffPos = 1u;

        /** Buffer position in message buffer of exception code */
        static constexpr uint8_t exceptionCode_BuffPos = 2u;

        /** Buffer position of Diagnostics sub-function */
        static constexpr uint8_t diagSubFunction_BuffPos = 2u;

        /** Diagnostics sub-function Restart Communications Option */
        static constexpr uint16_t diagRestartSubFunction = 0x0001u;

        /** Message length (without CRC) in case of an error during processing */
        static constexpr uint8_t ExceptionMessageLen = 3u;

        /** Count of available functions in functions handlers array */
        uint8_t functionCount = 0u;

        void NewMessageHandler( void );

        ModBus::FunctionsHandler *FindHandler( modBus_FunctionCode_t functionCode ) const;

        bool IsAllowedInListenOnlyMode( const modBus_Message_t * const msgBuffer, modBus_MsgLength_t msgLength ) const;

        void ProcessException( modBus_Message_t * const msgBuffer, modBus_ErrorStatus_t errorId, modBus_MsgLength_t &msgLength );
    };
} /* namespace Slave */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_HPP */
