/**
 * \author Mr.Nobody
 * \file ModBus_Core_Types.h
 * \ingroup ModBus
 * \brief ModBus module common (C compatible) types definition.
 *
 * Types shared by all layers of the module (physical, transport and
 * application layer). The file is C compatible, so it can be included also by
 * C modules (eg. HAL layer used by physical layer implementation).
 */

#ifndef THIRDPARTY_MODBUS_MODBUS_TYPES_H
#define THIRDPARTY_MODBUS_MODBUS_TYPES_H

/* ============================= INCLUDES =================================== */
#include "stdint.h"                             /* Standard types definition  */
/* ============================= TYPEDEFS =================================== */

/** Enumeration used to signal request processing state */
typedef enum
{
    MODBUS_REQUEST_ERROR = 0u, /**< Processing request failed  */
    MODBUS_REQUEST_OK          /**< Processing request succeed */
}   modBus_RequestState_t;


/** \brief Types used to signal device address [0-255] */
typedef uint8_t modBus_DeviceAddr_t;


/** \brief Type used to signal message length in bytes */
typedef uint16_t modBus_MsgLength_t;


/**
 * \brief Type used to signal count of events
 *
 * Use this datatype for event counts, where the following characteristics
 * are sufficient:
 *  - Range of values: 0 to 65535
 *  - Resolution     : 1
 */
typedef uint16_t modBus_EventCount_t;


/** \brief Enumeration used to signal actual status of message reception */
typedef enum
{
    MODBUS_RX_NOT_RCVD = 0u, /**< No new received/processed messages */
    MODBUS_RX_READY          /**< New message has been received      */
}   modBus_RxStatus_t;


/** \brief Enumeration used to signal actual status of message transmission */
typedef enum
{
    MODBUS_TX_ACTIVE = 0u, /**< TX message is being transmitted */
    MODBUS_TX_FINISHED     /**< TX message has been transmitted */
}   modBus_TxStatus_t;


/** \brief Enumeration type used to signal parity configuration
 * used for USART communication */
typedef enum
{
    MODBUS_PARITY_NONE = 0u, /**< Parity check is not used */
    MODBUS_PARITY_EVEN,      /**< Even parity is used      */
    MODBUS_PARITY_ODD        /**< Odd parity is used       */
}   modBus_Parity_t;


/**
 * \brief Enumeration type used to signal count of stop-bits used
 * in configuration.
 */
typedef enum
{
    MODBUS_STOPBIT_0_5 = 0u, /**< 0.5 stop bit used */
    MODBUS_STOPBIT_1,        /**< 1 stop bit used   */
    MODBUS_STOPBIT_1_5,      /**< 1.5 stop bit used */
    MODBUS_STOPBIT_2         /**< 2 stop bits used  */
}   modBus_StopBits_t;


/** \brief Type used to signal baudrate configuration
 * used for USART communication */
typedef uint32_t modBus_Baudrate_t;


/** \brief Type used to signal actual value in counter */
typedef uint32_t modBus_Counter_t;


/** \brief Type used to signal time in ms [0.001s] */
typedef uint32_t modBus_Time_t;


/** \brief Type used by message buffer */
typedef uint8_t modBus_Message_t;


/** \brief Serial bus configuration */
typedef struct
{
    modBus_Baudrate_t Baudrate; /**< Baudrate in bit/s      */
    modBus_Parity_t   Parity;   /**< Parity configuration   */
    modBus_StopBits_t StopBits; /**< Count of stop bits     */
}   modBus_BusConfig_t;


/** \brief Communication statistics counters of the transport layer */
typedef struct
{
    modBus_Counter_t crcErrorCount;     /**< Count of CRC (or too short frame) errors detected   */
    modBus_Counter_t framesRcvdCnt;     /**< Count of overall received frames                    */
    modBus_Counter_t addrMatchedCnt;    /**< Count of frames addressed to device (incl. broadcast) */
    modBus_Counter_t addrNotMatchedCnt; /**< Count of frames addressed to other devices          */
}   modBus_StatsCounter_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Null pointer definition */
#define MODBUS_NULL_PTR                                      ((void*)0u)

/** Broadcast device address. Request is executed by all slaves, no response is sent */
#define MODBUS_BROADCAST_ADDR                                ( 0u )

/** Maximal length of ModBus RTU frame (ADU) in bytes - address, PDU and CRC */
#define MODBUS_FRAME_MAX_LEN                                 ( 256u )

/** Count of bytes of CRC appended to every ModBus RTU frame */
#define MODBUS_CRC_LEN                                       ( 2u )

/** Minimal length of valid ModBus RTU frame - address, function code and CRC */
#define MODBUS_FRAME_MIN_LEN                                 ( 4u )

/* ========================= EXPORTED MACROS ================================ */

/** Inline function used to extract upper 8bits from one 16bit variable */
static inline uint8_t ModBus_Get_Upper8bFrom16b( uint16_t inputData )
{
    return ( (uint8_t)( ( inputData >> 8u ) & 0xFFu ) );
}


/** Inline function used to extract lower 8bits from one 16bit variable */
static inline uint8_t ModBus_Get_Lower8bFrom16b( uint16_t inputData )
{
    return ( (uint8_t)( inputData & 0xFFu ) );
}


/** Inline function used to merge two 8bits variables into one 16bits variable */
static inline uint16_t ModBus_Get_16bFromTwo8b( uint8_t upperByte, uint8_t lowerByte )
{
    return ( (uint16_t)( ( (uint16_t)upperByte << 8u ) | lowerByte ) );
}

/* ========================= EXPORTED VARIABLES ============================= */

#endif /* THIRDPARTY_MODBUS_MODBUS_TYPES_H */
