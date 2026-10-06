/**
 * \author Mr.Nobody
 * \file ModBus_App_RSID.hpp
 * \ingroup ModBus
 * \brief ModBus Report Slave ID (RSID, 0x11) application layer handler header
 *        file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RSID_HPP
#define APPLAYER_MODBUS_APP_RSID_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/** \brief Run indicator status */
typedef enum
{
    MODBUS_APP_RUN_INACTIVE = 0x00, /**< Device is in inactive state */
    MODBUS_APP_RUN_ACTIVE   = 0xFF  /**< Device is in active state   */
}   modBus_App_RunStatus_t;


/** \brief Run indicator status getter type */
typedef modBus_App_RunStatus_t ( modBus_App_RunStatusGetter_t )( void );


/** \brief Function configuration structure type */
typedef struct
{
    uint8_t                       SlaveId         = 0u;      /**< Slave ID (device specific)           */
    modBus_App_RunStatusGetter_t *RunStatusGetter = nullptr; /**< Run indicator status getter          */
    uint8_t                       AddData_Len     = 0u;      /**< Length of additional data            */
    const uint8_t                *AddData         = nullptr; /**< Additional data (device specific)    */
}   modBus_App_RSID_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class RSID : public FunctionsHandler
    {
    public:

        RSID( const modBus_App_RSID_t &slaveIdConfigStruct );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

    private:

        /** Report Slave ID (RSID) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RSID;

        /** Length of request without CRC */
        static constexpr modBus_MsgLength_t requestLen = 2u;

        /** Byte count position in response */
        static constexpr modBus_MsgLength_t byteCount_BuffPos = 2u;

        /** Slave identification position in response */
        static constexpr modBus_MsgLength_t slaveId_BuffPos = 3u;

        /** Run indicator status position in response */
        static constexpr modBus_MsgLength_t runIndicatorStatus_BuffPos = 4u;

        /** Additional data start position in response */
        static constexpr modBus_MsgLength_t addData_BuffPos = 5u;

        /** Function configuration */
        const modBus_App_RSID_t &functionConfig;

    }; /* class RSID */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RSID_HPP */
