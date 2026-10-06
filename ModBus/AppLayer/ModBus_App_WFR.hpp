/**
 * \author Mr.Nobody
 * \file ModBus_App_WFR.hpp
 * \ingroup ModBus
 * \brief ModBus Write File Record (WFR, 0x15) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_WFR_HPP
#define APPLAYER_MODBUS_APP_WFR_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
#include "ModBus_App_RFR.hpp"           /* File record callback type          */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class WFR : public FunctionsHandler
    {
    public:

        WFR( modBus_FileRecordCallback_t *writeCallback );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Write File Record (WFR) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_WFR;

        /** Request data length position in request */
        static constexpr modBus_MsgLength_t dataLength_BuffPos = 2u;

        /** First sub-request position */
        static constexpr modBus_MsgLength_t subRequests_BuffPos = 3u;

        /** Length of sub-request header (reference type, file, record, length) */
        static constexpr uint8_t subRequestHeaderLen = 7u;

        /** Minimal request data length */
        static constexpr uint8_t dataLengthMin = 0x09u;

        /** Maximal request data length */
        static constexpr uint8_t dataLengthMax = 0xFBu;

        /** Maximal count of records in one sub-request */
        static constexpr uint16_t recordLengthMax = ( dataLengthMax - subRequestHeaderLen ) / 2u;

        /** Callback writing file records */
        modBus_FileRecordCallback_t *writeCallback = nullptr;

        modBus_ErrorStatus_t CheckRequest( const modBus_Message_t * const msgBuffer, modBus_MsgLength_t msgLength ) const;

    }; /* class WFR */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_WFR_HPP */
