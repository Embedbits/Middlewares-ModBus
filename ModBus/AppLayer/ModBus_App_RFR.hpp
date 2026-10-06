/**
 * \author Mr.Nobody
 * \file ModBus_App_RFR.hpp
 * \ingroup ModBus
 * \brief ModBus Read File Record (RFR, 0x14) application layer handler
 *        header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RFR_HPP
#define APPLAYER_MODBUS_APP_RFR_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/**
 * \brief Callback used to read / write file record
 *
 * \param fileNumber   [in]: File number [1 - 0xFFFF]
 * \param recordNumber [in]: Starting record number [0 - 9999]
 * \param values   [in/out]: Record values (one register per record)
 * \param recordLength [in]: Count of records (registers)
 *
 * \return \ref MODBUS_REQUEST_OK, \ref MODBUS_REQUEST_ERROR if file / record
 *         range does not exist (exception 0x02 is sent)
 */
typedef modBus_RequestState_t ( modBus_FileRecordCallback_t ) ( uint16_t fileNumber,
                                                                uint16_t recordNumber,
                                                                modBus_RegistersValue_t *values,
                                                                uint16_t recordLength );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Reference type of file record sub-request (shall be 6) */
#define MODBUS_FILE_RECORD_REFERENCE_TYPE                     ( 6u )

/** Highest record number in file */
#define MODBUS_FILE_RECORD_NUMBER_MAX                         ( 0x270Fu )

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class RFR : public FunctionsHandler
    {
    public:

        RFR( modBus_FileRecordCallback_t *readCallback );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Read File Record (RFR) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RFR;

        /** Byte count position in request / response */
        static constexpr modBus_MsgLength_t byteCount_BuffPos = 2u;

        /** First sub-request / sub-response position */
        static constexpr modBus_MsgLength_t subRequests_BuffPos = 3u;

        /** Length of one sub-request */
        static constexpr uint8_t subRequestLen = 7u;

        /** Minimal byte count of request */
        static constexpr uint8_t byteCountMin = 0x07u;

        /** Maximal byte count of request */
        static constexpr uint8_t byteCountMax = 0xF5u;

        /** Maximal count of sub-requests (byteCountMax / subRequestLen) */
        static constexpr uint8_t subRequestsMax = byteCountMax / subRequestLen;

        /** Maximal length of PDU */
        static constexpr modBus_MsgLength_t pduMaxLen = 253u;

        /** Parsed sub-request */
        typedef struct
        {
            uint16_t FileNumber;   /**< File number             */
            uint16_t RecordNumber; /**< Starting record number  */
            uint16_t RecordLength; /**< Count of records        */
        }   subRequest_t;

        /** Callback reading file records */
        modBus_FileRecordCallback_t *readCallback = nullptr;

    }; /* class RFR */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RFR_HPP */
