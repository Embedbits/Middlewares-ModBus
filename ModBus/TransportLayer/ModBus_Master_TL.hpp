/**
 * \author Mr.Nobody
 * \file ModBus_Master_TL.hpp
 * \ingroup ModBus
 * \brief ModBus Master Transport Layer (TL) header file.
 *
 * Transport layer of master device. One request is processed at a time:
 * 1. \ref ModBus::Master::TransportLayer::StartTransmit starts the request
 *    (CRC is appended by transport layer).
 * 2. \ref ModBus::Master::TransportLayer::Task cyclically handles transmission,
 *    waiting for response, response check (length, CRC, slave address) and
 *    response timeout. Broadcast request (address 0) waits only for turnaround
 *    delay, no response is expected.
 * 3. \ref ModBus::Master::TransportLayer::Get_RequestStatus signals result of
 *    the request. Response (also exception response) is copied into the user
 *    reception buffer.
 */

#ifndef MODBUS_MASTER_HPP
#define MODBUS_MASTER_HPP
/* ============================= INCLUDES =================================== */
#include "ModBus_Core_Types.h"              /* Module types definition        */
#include "ModBus_Phy.hpp"                   /* Physical layer definition      */
#include "ModBus_Crc.hpp"                   /* CRC functionality              */
/* ============================= TYPEDEFS =================================== */

/** \brief Status of request processed by master transport layer */
typedef enum
{
    MODBUS_MASTER_REQ_IDLE = 0u,  /**< No request has been started                          */
    MODBUS_MASTER_REQ_BUSY,       /**< Request is being processed                           */
    MODBUS_MASTER_REQ_DONE,       /**< Response received (broadcast: turnaround delay over) */
    MODBUS_MASTER_REQ_TX_ERROR,   /**< Transmission was not finished in time                */
    MODBUS_MASTER_REQ_TIMEOUT,    /**< Slave did not respond within response timeout        */
    MODBUS_MASTER_REQ_CRC_ERROR,  /**< Response has invalid CRC or length                   */
    MODBUS_MASTER_REQ_ADDR_ERROR  /**< Response was received from other slave               */
}   modBus_MasterRequestStatus_t;

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ========================= EXPORTED MACROS ================================ */

/* ========================= EXPORTED VARIABLES ============================= */

/* ======================== EXPORTED FUNCTIONS ============================== */

namespace ModBus
{
namespace Master
{
    class TransportLayer : private Crc
    {
    public:
        TransportLayer( PhysicalLayer &physicalLayerObj );

        void Task( modBus_Time_t taskTime );

        modBus_RequestState_t StartTransmit( modBus_MsgLength_t messageLen,
                                             uint8_t * const txBufferPtr,
                                             uint8_t * const rxBufferPtr );

        void Set_ResponseTimeout( modBus_Time_t timeout );
        void Set_TurnaroundDelay( modBus_Time_t delay );

        modBus_MasterRequestStatus_t Get_RequestStatus( void ) const;
        modBus_RxStatus_t            Get_RxMessageStatus( modBus_MsgLength_t * const msgLen ) const;

        /** Default timeout for waiting for response from slave in ms [0.001s] */
        static constexpr modBus_Time_t modBus_DefaultResponseTimeout_ms = 1000u;

        /** Default delay after broadcast request in ms [0.001s] */
        static constexpr modBus_Time_t modBus_DefaultTurnaroundDelay_ms = 100u;

        /** Maximal time of frame transmission in ms [0.001s]. Full frame
         * (256 bytes) at 9600 Bd takes approximately 300 ms. */
        static constexpr modBus_Time_t modBus_TransmissionTimeout_ms = 1000u;

    private:

        /* ========================= TYPEDEFS =============================== */

        /** Define the module specific states */
        typedef enum
        {
            STATE_INIT = 0u,     /**< Initialization state                      */
            STATE_STANDBY,       /**< Standby state                             */
            STATE_TRANSMIT,      /**< Request transmit state                    */
            STATE_RESPONSE_WAIT, /**< Wait for response reception state         */
            STATE_FRAME_CHECK,   /**< Response check state                      */
            STATE_TURNAROUND     /**< Turnaround delay after broadcast request  */
        }   sm_States_t;


        /** Function pointer datatype to be used for the entry, execution and checkLeave routines
         *  used by the state machine  */
        typedef void (ModBus::Master::TransportLayer::*sm_PtrToRoutine_t)( void );


        /** Structure data type that integrates the function pointers to entry,
         *  execution and checkLeave routines of one state. */
        typedef struct
        {
            sm_PtrToRoutine_t entry;      /**< Entry part of state       */
            sm_PtrToRoutine_t execute;    /**< Execute part of state     */
            sm_PtrToRoutine_t checkLeave; /**< Check leave part of state */
            sm_PtrToRoutine_t leave;      /**< Leave part of state       */
        }   routinesStruct_t;


        /** Structure data type that links the function pointers of entry,
         *  execute and checkLeave routines to its specific state */
        typedef struct
        {
            sm_States_t      state;   /**< State ID (name)                    */
            routinesStruct_t routine; /**< Routines list for current state ID */
        }   stateLinkedRoutines_t;

        /* ==================== FORWARD DECLARATIONS ======================== */

        void StateHandler( void );

        void State_Init_Entry( void );
        void State_Init_Execute( void );
        void State_Init_CheckLeave( void );
        void State_Init_Leave( void );

        void State_Standby_Entry( void );
        void State_Standby_Execute( void );
        void State_Standby_CheckLeave( void );
        void State_Standby_Leave( void );

        void State_Transmit_Entry( void );
        void State_Transmit_Execute( void );
        void State_Transmit_CheckLeave( void );
        void State_Transmit_Leave( void );

        void State_ResponseWait_Entry( void );
        void State_ResponseWait_Execute( void );
        void State_ResponseWait_CheckLeave( void );
        void State_ResponseWait_Leave( void );

        void State_FrameCheck_Entry( void );
        void State_FrameCheck_Execute( void );
        void State_FrameCheck_CheckLeave( void );
        void State_FrameCheck_Leave( void );

        void State_Turnaround_Entry( void );
        void State_Turnaround_Execute( void );
        void State_Turnaround_CheckLeave( void );
        void State_Turnaround_Leave( void );

        void IncrementStateCounter( void );

        /* ====================== LOCAL VARIABLES =========================== */

        /** holds the current state of the state machine */
        sm_States_t sm_ActualState = STATE_INIT;

        /** holds the desired next state */
        sm_States_t sm_NewState = STATE_INIT;

        /** Result of actual (last) request */
        modBus_MasterRequestStatus_t requestStatus = MODBUS_MASTER_REQ_IDLE;

        /** New request is waiting for transmission */
        bool requestPending = false;

        /** Transmission of request finished / response received in actual state */
        bool phyEventDone = false;

        /** User buffer with request */
        uint8_t *txFrameData = nullptr;

        /** Length of request (including CRC after transmission start) */
        modBus_MsgLength_t txFrameLength = 0u;

        /** User buffer for response */
        uint8_t *rxFrameData = nullptr;

        /** Length of response without CRC */
        modBus_MsgLength_t rxFrameLength = 0u;

        /** Holds value of state counter in ms [0.001s] */
        modBus_Time_t stateCounter = 0u;

        /** Message response timeout value in ms [0.001s] */
        modBus_Time_t responseTimeout = modBus_DefaultResponseTimeout_ms;

        /** Delay after broadcast request in ms [0.001s] */
        modBus_Time_t turnaroundDelay = modBus_DefaultTurnaroundDelay_ms;

        /** Value of cyclic execution of task handler in ms [0.001s] */
        modBus_Time_t handlerTaskTime = 0u;

        /** Object of physical layer handler */
        PhysicalLayer &physicalLayer;
    };
} /* namespace Master */
} /* namespace ModBus */

#endif /* MODBUS_MASTER_HPP */
