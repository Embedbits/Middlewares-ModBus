/**
 * \author Mr.Nobody
 * \file ModBus_App_CommDiag.hpp
 * \ingroup ModBus
 * \brief ModBus application layer communication diagnostics (serial line).
 *
 * Holds diagnostic counters, communication event counter, communication event
 * log, diagnostic register and Listen Only Mode state of slave device. The
 * object is updated by application core (\ref ModBus::AppLayer::Slave::AppCore)
 * and it is read / controlled by functions:
 * - Diagnostics (0x08)                     - \ref ModBus::AppLayer::DIAG
 * - Get Communication Event Counter (0x0B) - \ref ModBus::AppLayer::GCEC
 * - Get Communication Event Log (0x0C)     - \ref ModBus::AppLayer::GCEL
 *
 * Bus counters (bus message count, CRC errors) are taken from transport layer.
 * Bus character overrun is not detected by the module, its counter is always 0.
 */

#ifndef APPLAYER_MODBUS_APP_COMMDIAG_HPP
#define APPLAYER_MODBUS_APP_COMMDIAG_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
#include "ModBus_Slave_TL.hpp"          /* Slave transport layer              */
/* ============================= TYPEDEFS =================================== */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Count of events stored in communication event log */
#define MODBUS_COMM_EVENT_LOG_SIZE                            ( 64u )

/* ------------------------ Communication event bytes ----------------------- */

/** Receive event - bit always set */
#define MODBUS_COMM_EVENT_RCV                                 ( 0x80u )
/** Receive event - communication error (CRC) */
#define MODBUS_COMM_EVENT_RCV_COMM_ERROR                      ( 0x02u )
/** Receive event - character overrun */
#define MODBUS_COMM_EVENT_RCV_CHAR_OVERRUN                    ( 0x10u )
/** Receive event - currently in Listen Only Mode */
#define MODBUS_COMM_EVENT_RCV_LISTEN_ONLY                     ( 0x20u )
/** Receive event - broadcast received */
#define MODBUS_COMM_EVENT_RCV_BROADCAST                       ( 0x40u )

/** Send event - bit always set */
#define MODBUS_COMM_EVENT_SEND                                ( 0x40u )
/** Send event - read exception sent (exception codes 1-3) */
#define MODBUS_COMM_EVENT_SEND_READ_EXCEPTION                 ( 0x01u )
/** Send event - slave abort exception sent (exception code 4) */
#define MODBUS_COMM_EVENT_SEND_SLAVE_ABORT                    ( 0x02u )
/** Send event - slave busy exception sent (exception codes 5-6) */
#define MODBUS_COMM_EVENT_SEND_SLAVE_BUSY                     ( 0x04u )
/** Send event - slave program NAK exception sent (exception code 7) */
#define MODBUS_COMM_EVENT_SEND_SLAVE_NAK                      ( 0x08u )
/** Send event - currently in Listen Only Mode */
#define MODBUS_COMM_EVENT_SEND_LISTEN_ONLY                    ( 0x20u )

/** Event - slave entered Listen Only Mode */
#define MODBUS_COMM_EVENT_LISTEN_ONLY_ENTERED                 ( 0x04u )
/** Event - communication restart */
#define MODBUS_COMM_EVENT_COMM_RESTART                        ( 0x00u )

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace AppLayer
{
    class CommDiagnostics
    {
    public:

        CommDiagnostics( ModBus::Slave::TransportLayer &transportLayerObj );

        /* ------------------ Interface of application core ----------------- */

        void Update( void );
        void Set_RequestReceived( modBus_DeviceAddr_t slaveAddr );
        void Set_RequestProcessed( modBus_FunctionCode_t functionCode,
                                   modBus_ErrorStatus_t errorStatus,
                                   bool responseSent );
        void Set_RequestIgnored( void );

        /* ------------------ Interface of function handlers ---------------- */

        bool Get_ListenOnlyMode( void ) const;
        void Set_ListenOnlyMode( void );
        void Restart( bool clearEventLog );
        void Clear_Counters( void );
        void Clear_OverrunCounter( void );

        modBus_EventCount_t Get_BusMessageCount       ( void ) const;
        modBus_EventCount_t Get_BusCommErrorCount     ( void ) const;
        modBus_EventCount_t Get_BusExceptionErrorCount( void ) const;
        modBus_EventCount_t Get_SlaveMessageCount     ( void ) const;
        modBus_EventCount_t Get_SlaveNoResponseCount  ( void ) const;
        modBus_EventCount_t Get_SlaveNakCount         ( void ) const;
        modBus_EventCount_t Get_SlaveBusyCount        ( void ) const;
        modBus_EventCount_t Get_BusCharOverrunCount   ( void ) const;
        modBus_EventCount_t Get_EventCounter          ( void ) const;

        uint8_t Get_EventLog( uint8_t * const eventBuffer, uint8_t bufferSize ) const;

        /* --------------------- Interface of application ------------------- */

        void     Set_DiagnosticRegister( uint16_t regValue );
        uint16_t Get_DiagnosticRegister( void ) const;

    private:

        void AddEvent( uint8_t eventByte );

        /** Transport layer providing bus counters */
        ModBus::Slave::TransportLayer &transportLayer;

        /** Communication event log (ring buffer) */
        uint8_t eventLog[ MODBUS_COMM_EVENT_LOG_SIZE ] = { 0u };

        /** Position of next event in event log */
        uint8_t eventLogHead = 0u;

        /** Count of valid events in event log */
        uint8_t eventLogCount = 0u;

        /** Value of transport layer frames counter at last counters clear */
        modBus_Counter_t busMessageBase = 0u;

        /** Value of transport layer CRC errors counter at last counters clear */
        modBus_Counter_t busCommErrorBase = 0u;

        /** Value of transport layer CRC errors counter at last update */
        modBus_Counter_t lastCommErrorCount = 0u;

        /** Count of exception responses sent */
        modBus_EventCount_t busExceptionErrorCount = 0u;

        /** Count of requests addressed to slave (incl. broadcast) */
        modBus_EventCount_t slaveMessageCount = 0u;

        /** Count of requests addressed to slave without response */
        modBus_EventCount_t slaveNoResponseCount = 0u;

        /** Count of Negative Acknowledge exception responses */
        modBus_EventCount_t slaveNakCount = 0u;

        /** Count of Slave Device Busy exception responses */
        modBus_EventCount_t slaveBusyCount = 0u;

        /** Communication event counter (successfully completed requests) */
        modBus_EventCount_t eventCounter = 0u;

        /** Diagnostic register content (application specific) */
        uint16_t diagnosticRegister = 0u;

        /** Listen Only Mode is active */
        bool listenOnlyMode = false;
    };
} /* namespace AppLayer */
} /* namespace ModBus */

#endif /* APPLAYER_MODBUS_APP_COMMDIAG_HPP */
