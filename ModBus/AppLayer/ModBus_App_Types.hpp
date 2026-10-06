/**
 * \author Mr.Nobody
 * \file ModBus_App_Types.hpp
 * \ingroup ModBus
 * \brief ModBus module application layer types definition file
 *
 * Exception codes (\ref modBus_ErrorStatus_t) returned by function handlers:
 *
 * - \ref MODBUS_ERROR_NO_ACTIVE_ERROR :
 *   No error reported
 *
 * - \ref MODBUS_ERROR_ILLEGAL_FUNCTION :
 *   The function code received in the query is not an allowable action for the
 *   server (or slave). This may be because the function code is only applicable
 *   to newer devices, and was not implemented in the unit selected. It could
 *   also indicate that the server (or slave) is in the wrong state to process
 *   a request of this type, for example because it is unconfigured and is being
 *   asked to return register values.
 *
 * - \ref MODBUS_ERROR_ILLEGAL_DATA_ADDRESS :
 *   The data address received in the query is not an allowable address for the
 *   server (or slave). More specifically, the combination of reference number
 *   and transfer length is invalid. For a controller with 100 registers, the
 *   PDU addresses the first register as 0, and the last one as 99. If a request
 *   is submitted with a starting register address of 96 and a quantity of
 *   registers of 4, then this request will successfully operate (address-wise
 *   at least) on registers 96, 97, 98, 99. If a request is submitted with a
 *   starting register address of 96 and a quantity of registers of 5, then this
 *   request will fail with Exception Code 0x02 "Illegal Data Address" since it
 *   attempts to operate on registers 96, 97, 98, 99 and 100, and there is no
 *   register with address 100.
 *
 * - \ref MODBUS_ERROR_ILLEGAL_DATA_VALUE :
 *   A value contained in the query data field is not an allowable value for
 *   server (or slave). This indicates a fault in the structure of the remainder
 *   of a complex request, such as that the implied length is incorrect. It
 *   specifically does NOT mean that a data item submitted for storage in a
 *   register has a value outside the expectation of the application program,
 *   since the MODBUS protocol is unaware of the significance of any particular
 *   value of any particular register.
 *
 * - \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE :
 *   An unrecoverable error occurred while the server (or slave) was attempting
 *   to perform the requested action.
 *
 * - \ref MODBUS_ERROR_ACKNOWLEDGE :
 *   Specialized use in conjunction with programming commands.
 *   The server (or slave) has accepted the request and is processing it, but a
 *   long duration of time will be required to do so.
 *
 * - \ref MODBUS_ERROR_SLAVE_DEVICE_BUSY :
 *   Specialized use in conjunction with programming commands.
 *   The server (or slave) is engaged in processing a long-duration program
 *   command. The client (or master) should retransmit the message later.
 *
 * - \ref MODBUS_ERROR_NEGATIVE_ACKNOWLEDGE :
 *   The server can not perform the program function received in the query.
 *
 * - \ref MODBUS_ERROR_MEMORY_PARITY_ERROR :
 *   Specialized use in conjunction with function codes 20 and 21 and reference
 *   type 6, to indicate that the extended file area failed to pass a
 *   consistency check.
 *
 * - \ref MODBUS_ERROR_GW_PATH_UNAVAILABLE :
 *   Specialized use in conjunction with gateways, indicates that the gateway
 *   was unable to allocate an internal communication path.
 *
 * - \ref MODBUS_ERROR_GW_TARGET_FAILED_TO_RESPOND :
 *   Specialized use in conjunction with gateways, indicates that no response
 *   was obtained from the target device.
 */

#ifndef APPLAYER_MODBUS_APP_TYPES_HPP
#define APPLAYER_MODBUS_APP_TYPES_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_Core_Types.h"          /* Module public types definitions    */
#include "stdint.h"                     /* Standard types definition          */
/* ============================= TYPEDEFS =================================== */

/** \brief Data-type signaling function code for ModBus messages */
typedef uint8_t modBus_FunctionCode_t;


/** \brief Enumeration used to signal if service is supported or not */
typedef enum
{
    MODBUS_SERVICE_NOT_SUPPORTED = 0u, /**< Requested service is not supported */
    MODBUS_SERVICE_SUPPORTED           /**< Requested service is supported     */
}   modBus_ServiceStatus_t;


/** \brief Type used to signal registers getter's / setter's value */
typedef uint16_t modBus_RegistersValue_t;


/** \brief Type used to signal coils (and discrete inputs) getter's / setter's value */
typedef enum
{
    MODBUS_COIL_INACTIVE = 0u, /**< Coil is OFF */
    MODBUS_COIL_ACTIVE         /**< Coil is ON  */
}   modBus_CoilsRetValue_t;


/** \brief Enumeration used to signal if handler corresponds to function code */
typedef enum
{
    MODBUS_FUNCTION_INCORRECT = 0u, /**< Function code do not correspond        */
    MODBUS_FUNCTION_EQUAL           /**< Function code corresponds with handler */
}   modBus_FunctionStatus_t;


/** \brief Enumeration used to signal error (exception) type */
typedef enum
{
    MODBUS_ERROR_NO_ACTIVE_ERROR             = 0x00, /**< No error reported                            */
    MODBUS_ERROR_ILLEGAL_FUNCTION            = 0x01, /**< Query is not an allowable action             */
    MODBUS_ERROR_ILLEGAL_DATA_ADDRESS        = 0x02, /**< Received data address is invalid             */
    MODBUS_ERROR_ILLEGAL_DATA_VALUE          = 0x03, /**< Received data are invalid                    */
    MODBUS_ERROR_SLAVE_DEVICE_FAILURE        = 0x04, /**< An unrecoverable error                       */
    MODBUS_ERROR_ACKNOWLEDGE                 = 0x05, /**< Accepted request,long processing             */
    MODBUS_ERROR_SLAVE_DEVICE_BUSY           = 0x06, /**< Rejected request,processing previous request */
    MODBUS_ERROR_NEGATIVE_ACKNOWLEDGE        = 0x07, /**< Program function can not be performed        */
    MODBUS_ERROR_MEMORY_PARITY_ERROR         = 0x08, /**< Parity error                                 */
    MODBUS_ERROR_GW_PATH_UNAVAILABLE         = 0x0A, /**< Gateway error, target not reachable          */
    MODBUS_ERROR_GW_TARGET_FAILED_TO_RESPOND = 0x0B  /**< Gateway error, target did not respond        */
}   modBus_ErrorStatus_t;


/** \brief Type used for registers callback's */
typedef modBus_RequestState_t ( modBus_RegistersCallback_t ) ( modBus_RegistersValue_t *value );


/** \brief Type used for coils function callback's */
typedef modBus_RequestState_t ( modBus_CoilsCallback_t ) ( modBus_CoilsRetValue_t *value );

/* ------------------------ Registers operations types ---------------------- */

/**
 * \brief Configuration of one register (holding or input register)
 *
 * Configuration array is indexed by register address (array index 0 is
 * register address 0). Getter is used by read functions, setter by write
 * functions. Register with null callback (or not supported) responds with
 * exception \ref MODBUS_ERROR_ILLEGAL_DATA_ADDRESS. Callback returning
 * \ref MODBUS_REQUEST_ERROR responds with \ref MODBUS_ERROR_SLAVE_DEVICE_FAILURE.
 */
typedef struct
{
    uint8_t                     RegisterId;    /**< Register identification (informative)   */
    modBus_ServiceStatus_t      ServiceStatus; /**< Supporting status of requested function */
    modBus_RegistersCallback_t *ApplGetter;    /**< Routine getter of requested function    */
    modBus_RegistersCallback_t *ApplSetter;    /**< Routine setter of requested function    */
}   modBus_RegistersConfigStruct_t;


/** \brief Count of registers handled by system */
typedef uint16_t modBus_RegistersCount_t;

/* -------------------------- Coils operations types ------------------------ */

/** \brief Configuration of one coil. Same rules as \ref modBus_RegistersConfigStruct_t */
typedef struct
{
    uint8_t                 CoilId;        /**< Coil identification (informative)       */
    modBus_ServiceStatus_t  ServiceStatus; /**< Supporting status of requested function */
    modBus_CoilsCallback_t *ApplGetter;    /**< Routine getter of requested function    */
    modBus_CoilsCallback_t *ApplSetter;    /**< Routine setter of requested function    */
}   modBus_CoilsConfigStruct_t;


/** \brief Count of coils handled by system */
typedef uint16_t modBus_CoilsCount_t;

/* ----------------------- Discrete IO operations types --------------------- */

/** \brief Configuration of one discrete input. Same rules as \ref modBus_RegistersConfigStruct_t.
 *  Setter is not used (discrete inputs are read only). */
typedef struct
{
    uint8_t                 DiscreteIoId;  /**< Discrete IO identification (informative) */
    modBus_ServiceStatus_t  ServiceStatus; /**< Supporting status of requested function  */
    modBus_CoilsCallback_t *ApplGetter;    /**< Routine getter of requested function     */
    modBus_CoilsCallback_t *ApplSetter;    /**< Not used                                 */
}   modBus_DiscreteIoConfigStruct_t;


/** \brief Count of discrete IO's handled by system */
typedef uint16_t modBus_DiscreteIoCount_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

#define MODBUS_APP_NULL_PTR                                     ( ( void*) 0u )

/*------------ Public function codes / Data Access / Bit access --------------*/

/** Read Coils (RC) function code */
#define MODBUS_FUNCTION_RC                                    ( 0x01u )
/** Read Discrete Inputs (RDI) function code */
#define MODBUS_FUNCTION_RDI                                   ( 0x02u )
/** Write Single Coil (WSC) function code */
#define MODBUS_FUNCTION_WSC                                   ( 0x05u )
/** Write Multiple Coils (WMC) function code */
#define MODBUS_FUNCTION_WMC                                   ( 0x0Fu )

/*---------- Public function codes / Data Access / 16 bit access -------------*/

/** Read Holding Registers (RHR) function code */
#define MODBUS_FUNCTION_RHR                                   ( 0x03u )
/** Read Input Register (RIR) function code */
#define MODBUS_FUNCTION_RIR                                   ( 0x04u )
/** Write Single Register (WSR) function code */
#define MODBUS_FUNCTION_WSR                                   ( 0x06u )
/** Write Multiple Registers (WMR) function code */
#define MODBUS_FUNCTION_WMR                                   ( 0x10u )
/** Mask Write Register (MWR) function code */
#define MODBUS_FUNCTION_MWR                                   ( 0x16u )
/** Read/Write Multiple Registers (RWMR) function code */
#define MODBUS_FUNCTION_RWMR                                  ( 0x17u )
/** Read FIFO queue (RFIFO) function code */
#define MODBUS_FUNCTION_RFIFO                                 ( 0x18u )

/*-------- Public function codes / Data Access / File record access ----------*/

/** Read File Record (RFR) function code */
#define MODBUS_FUNCTION_RFR                                   ( 0x14u )
/** Write File Record (WFR) function code */
#define MODBUS_FUNCTION_WFR                                   ( 0x15u )

/*----------------- Public function codes / Diagnostics ----------------------*/

/** Read Exception Status (RES) function code (serial line only) */
#define MODBUS_FUNCTION_RES                                   ( 0x07u )
/** Diagnostics (DIAG) function code (serial line only) */
#define MODBUS_FUNCTION_DIAG                                  ( 0x08u )
/** Get Communication Event Counter (GCEC) function code (serial line only) */
#define MODBUS_FUNCTION_GCEC                                  ( 0x0Bu )
/** Get Communication Event Log (GCEL) function code (serial line only) */
#define MODBUS_FUNCTION_GCEL                                  ( 0x0Cu )
/** Report Slave ID (RSID) function code (serial line only) */
#define MODBUS_FUNCTION_RSID                                  ( 0x11u )
/** Read Device Identification (RDID) function code (MEI type 0x0E) */
#define MODBUS_FUNCTION_RDID                                  ( 0x2Bu )

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

/* ====================== HANDLER CLASS PROTOTYPE =========================== */

namespace ModBus
{
    /**
     * \brief Interface of application layer function handler
     *
     * Every supported function code is implemented by one handler. Handlers
     * are registered in application core (\ref ModBus::AppLayer::Slave::AppCore).
     *
     * Message buffer layout used by \ref SlaveDecodeMessage (ModBus RTU frame
     * without CRC):
     *
     *  | Index | Content           |
     *  |-------|-------------------|
     *  | 0     | Slave address     |
     *  | 1     | Function code     |
     *  | 2...  | Function data     |
     *
     * Response is written into the same buffer (in-place). Buffer has size of
     * \ref MODBUS_FRAME_MAX_LEN bytes. Address and function code bytes are
     * kept, exception response is created by application core.
     */
    class FunctionsHandler
    {
    public:

        virtual ~FunctionsHandler() = default;

        /**
         * \brief Processes request and creates response
         *
         * \param msgBuffer [in/out]: Request, overwritten by response
         * \param msgLength [in/out]: Length of request without CRC (address
         *                            and PDU). Set to length of response
         *                            without CRC if no error is returned.
         *
         * \return \ref MODBUS_ERROR_NO_ACTIVE_ERROR on success, otherwise
         *         exception code sent to master.
         */
        virtual modBus_ErrorStatus_t SlaveDecodeMessage( modBus_Message_t * const msgBuffer, modBus_MsgLength_t &msgLength ) = 0;

        /** \brief Returns function code handled by the handler */
        virtual modBus_FunctionCode_t GetFunctionCode( void ) const = 0;

        /**
         * \brief Checks if handler handles received function code
         *
         * \param rcvdFunctionCode [in]: Received function code
         *
         * \return \ref MODBUS_FUNCTION_EQUAL if function code corresponds
         */
        modBus_FunctionStatus_t GetFunctionStatus( modBus_FunctionCode_t rcvdFunctionCode ) const
        {
            return ( ( GetFunctionCode() == rcvdFunctionCode ) ? MODBUS_FUNCTION_EQUAL : MODBUS_FUNCTION_INCORRECT );
        }

    protected:

        /** Slave address position in message buffer */
        static constexpr modBus_MsgLength_t slaveAddr_BuffPos = 0u;

        /** Function code position in message buffer */
        static constexpr modBus_MsgLength_t functionCode_BuffPos = 1u;

        /** Maximal length of message without CRC (address and PDU) */
        static constexpr modBus_MsgLength_t messageMaxLen = MODBUS_FRAME_MAX_LEN - MODBUS_CRC_LEN;

        /** Reads 16bit big-endian value from message buffer */
        static uint16_t Get_16b( const modBus_Message_t * const msgBuffer, modBus_MsgLength_t buffPos )
        {
            return ( ModBus_Get_16bFromTwo8b( msgBuffer[ buffPos ], msgBuffer[ buffPos + 1u ] ) );
        }

        /** Writes 16bit big-endian value into message buffer */
        static void Set_16b( modBus_Message_t * const msgBuffer, modBus_MsgLength_t buffPos, uint16_t value )
        {
            msgBuffer[ buffPos      ] = ModBus_Get_Upper8bFrom16b( value );
            msgBuffer[ buffPos + 1u ] = ModBus_Get_Lower8bFrom16b( value );
        }
    };
}

#endif /* APPLAYER_MODBUS_APP_TYPES_HPP */
