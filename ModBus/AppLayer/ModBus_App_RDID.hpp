/**
 * \author Mr.Nobody
 * \file ModBus_App_RDID.hpp
 * \ingroup ModBus
 * \brief ModBus Read Device Identification (RDID, 0x2B / MEI 0x0E) application
 *        layer handler header file.
 *
 */

#ifndef APPLAYER_MODBUS_APP_RDID_HPP
#define APPLAYER_MODBUS_APP_RDID_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/** \brief Read Device ID code (type of access) */
typedef enum
{
    MODBUS_APP_RDID_BASIC_DEVICE_ID    = 1u, /**< Basic device identification (stream access)        */
    MODBUS_APP_RDID_REGULAR_DEVICE_ID  = 2u, /**< Regular device identification (stream access)      */
    MODBUS_APP_RDID_EXTENDED_DEVICE_ID = 3u, /**< Extended device identification (stream access)     */
    MODBUS_APP_RDID_SPECIFIC_DEVICE_ID = 4u  /**< One specific identification object (individual)    */
}   modBus_App_RDID_ObjType_t;


/** \brief Conformity level of device identification */
typedef enum
{
    MODBUS_APP_RDID_CONFORMITY_BASIC_STREAM    = 0x01, /**< Basic identification (stream access only)                    */
    MODBUS_APP_RDID_CONFORMITY_REGULAR_STREAM  = 0x02, /**< Regular identification (stream access only)                  */
    MODBUS_APP_RDID_CONFORMITY_EXTENDED_STREAM = 0x03, /**< Extended identification (stream access only)                 */
    MODBUS_APP_RDID_CONFORMITY_BASIC_BOTH      = 0x81, /**< Basic identification (stream access and individual access)   */
    MODBUS_APP_RDID_CONFORMITY_REGULAR_BOTH    = 0x82, /**< Regular identification (stream access and individual access) */
    MODBUS_APP_RDID_CONFORMITY_EXTENDED_BOTH   = 0x83  /**< Extended identification(stream access and individual access) */
}   modBus_App_RDID_ConformityLevel_t;


/** \brief Standard object identifications */
typedef enum
{
    MODBUS_RDID_ID_BASIC_VENDOR_NAME      = 0x00, /**< VendorName          : Category Basic   */
    MODBUS_RDID_ID_BASIC_PRODUCT_CODE     = 0x01, /**< ProductCode         : Category Basic   */
    MODBUS_RDID_ID_BASIC_SW_VERSION       = 0x02, /**< MajorMinorRevision  : Category Basic   */
    MODBUS_RDID_ID_REGULAR_VENDOR_URL     = 0x03, /**< VendorUrl           : Category Regular */
    MODBUS_RDID_ID_REGULAR_PRODUCT_NAME   = 0x04, /**< ProductName         : Category Regular */
    MODBUS_RDID_ID_REGULAR_MODEL_NAME     = 0x05, /**< ModelName           : Category Regular */
    MODBUS_RDID_ID_REGULAR_USER_APPL_NAME = 0x06  /**< UserApplicationName : Category Regular */
}   modBus_RDID_ObjList_t;


/**
 * \brief ModBus Read Device ID (RDID) object identification in range 0x00 - 0xFF
 *
 * | Object Id   | Object Name / Description | Type             | M/O       | Category |
 * |-------------|---------------------------|------------------|-----------|----------|
 * | 0x00        | VendorName                | ASCII String     | Mandatory | Basic    |
 * | 0x01        | ProductCode               | ASCII String     | Mandatory | Basic    |
 * | 0x02        | MajorMinorRevision        | ASCII String     | Mandatory | Basic    |
 * | 0x03        | VendorUrl                 | ASCII String     | Optional  | Regular  |
 * | 0x04        | ProductName               | ASCII String     | Optional  | Regular  |
 * | 0x05        | ModelName                 | ASCII String     | Optional  | Regular  |
 * | 0x06        | UserApplicationName       | ASCII String     | Optional  | Regular  |
 * | 0x07...0x7F | Reserved                  |                  | Optional  | Regular  |
 * | 0x80...0xFF | Private objects           | Device dependent | Optional  | Extended |
 */
typedef uint8_t modBus_RDID_ObjId_t;


/** \brief Representing count of used object in relevant array */
typedef uint8_t modBus_RDID_ObjCnt_t;


/**
 * \brief Callback providing object value
 *
 * \param objData   [out]: Pointer to object value (shall stay valid until the
 *                         response is transmitted, eg. constant string)
 * \param objLength [out]: Length of object value in bytes
 *
 * \return \ref MODBUS_REQUEST_OK, \ref MODBUS_REQUEST_ERROR results in
 *         exception 0x04
 */
typedef modBus_RequestState_t ( modBus_RDID_ObjCallback_t ) ( const uint8_t **objData, uint8_t *objLength );


/** \brief Configuration of one identification object */
typedef struct
{
    modBus_RDID_ObjId_t        ObjId;       /**< Object Identification          */
    modBus_RDID_ObjCallback_t *ObjCallback; /**< Callback providing object value */
}   modBus_App_RDID_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class RDID : public FunctionsHandler
    {
    public:

        RDID( const modBus_App_RDID_t *functionConfigStruct, modBus_RDID_ObjCnt_t objCount );

        modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) override;

        modBus_FunctionCode_t GetFunctionCode( void ) const override { return functionCode; }

        modBus_App_RDID_ConformityLevel_t Get_ConformityLevel( void ) const;

    private:

        /** Read Device Identification (RDID) function identification */
        static constexpr modBus_FunctionCode_t functionCode = MODBUS_FUNCTION_RDID;

        /** MEI type of Read Device Identification */
        static constexpr uint8_t meiTypeReadDeviceId = 0x0Eu;

        /** Length of request without CRC */
        static constexpr modBus_MsgLength_t requestLen = 5u;

        /** ModBus Encapsulated Interface (MEI) type position */
        static constexpr modBus_MsgLength_t meiType_BuffPos = 2u;

        /** Read Device ID code position */
        static constexpr modBus_MsgLength_t readDeviceIdCode_BuffPos = 3u;

        /** Object ID position in request */
        static constexpr modBus_MsgLength_t objectId_BuffPos = 4u;

        /** Conformity level position in response */
        static constexpr modBus_MsgLength_t conformityLevel_BuffPos = 4u;

        /** More follows position in response */
        static constexpr modBus_MsgLength_t moreFollows_BuffPos = 5u;

        /** Next object ID position in response */
        static constexpr modBus_MsgLength_t nextObjectId_BuffPos = 6u;

        /** Number of objects position in response */
        static constexpr modBus_MsgLength_t objectsCount_BuffPos = 7u;

        /** First object position in response */
        static constexpr modBus_MsgLength_t firstObject_BuffPos = 8u;

        /** More follows value - further transaction is needed */
        static constexpr uint8_t moreFollowsActive = 0xFFu;

        /** Last object ID of basic category */
        static constexpr modBus_RDID_ObjId_t basicObjIdMax = 0x02u;

        /** Last object ID of regular category */
        static constexpr modBus_RDID_ObjId_t regularObjIdMax = 0x7Fu;

        /** Last object ID of extended category */
        static constexpr modBus_RDID_ObjId_t extendedObjIdMax = 0xFFu;

        /** Objects configuration array */
        const modBus_App_RDID_t *functionConfig = nullptr;

        /** Count of available objects */
        modBus_RDID_ObjCnt_t objCount = 0u;

        const modBus_App_RDID_t *FindObject( uint16_t objId ) const;

        modBus_ErrorStatus_t AppendObject( modBus_Message_t * const msgBuffer,
                                           modBus_MsgLength_t &buffPos,
                                           const modBus_App_RDID_t &object,
                                           bool &objectFits ) const;

    }; /* class RDID */
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_RDID_HPP */
