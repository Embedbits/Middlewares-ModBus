/**
 * \author Mr.Nobody
 * \file ModBus_App.cpp
 * \ingroup ModBus
 * \brief ModBus module application layer core (slave) functionality
 *
 */
/* ============================= INCLUDES =================================== */
#include "ModBus_App.hpp"               /* Self include                       */
#include "ModBus_App_Types.hpp"         /* Module types definition            */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

/* ========================= SYMBOLIC CONSTANTS ============================= */

/* ============================== MACROS ==================================== */

/* ========================= EXPORTED VARIABLES ============================= */

/* ========================== LOCAL VARIABLES =============================== */

/* ======================== EXPORTED FUNCTIONS ============================== */

/**
 * \brief Application layer core constructor. Activates transport layer.
 *
 * \param transportLayerObj [in]: Slave transport layer
 * \param functionHandlers  [in]: Array of function handlers. Null entries are
 *                                ignored.
 * \param functionsCount    [in]: Count of items in function handlers array
 * \param commDiagnostics   [in]: Communication diagnostics object. Required by
 *                                handlers DIAG, GCEC and GCEL, otherwise it can
 *                                be null pointer.
 */
ModBus::AppLayer::Slave::AppCore::AppCore( ModBus::Slave::TransportLayer &transportLayerObj,
                                           ModBus::FunctionsHandler * const functionHandlers[],
                                           uint8_t functionsCount,
                                           ModBus::AppLayer::CommDiagnostics *commDiagnostics ) :
    transportLayer( transportLayerObj ),
    functionHandlers( functionHandlers ),
    diagnostics( commDiagnostics ),
    functionCount( functionsCount )
{
    if( nullptr == functionHandlers )
    {
        functionCount = 0u;
    }
    else
    {
        /* Handlers assigned */
    }

    transportLayer.Activate();
}


/**
 * \brief Cyclic ModBus handler.
 *
 * The transport layer task handler is executed with corresponding order.
 *
 * \param taskTime [in]: Time between two executions in ms [0.001s]
 */
void ModBus::AppLayer::Slave::AppCore::Task( modBus_Time_t taskTime )
{
    transportLayer.Task( taskTime );

    if( nullptr != diagnostics )
    {
        diagnostics->Update();
    }
    else
    {
        /* Diagnostics not used */
    }

    NewMessageHandler();
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Handler of new message requests.
 *
 */
void ModBus::AppLayer::Slave::AppCore::NewMessageHandler( void )
{
    if( MODBUS_APP_MSG_NEW_PENDING == transportLayer.Get_NewMessageStatus() )
    {
        modBus_ErrorStatus_t      errorStatus  = MODBUS_ERROR_NO_ACTIVE_ERROR;
        modBus_Message_t   *const msgBuffer    = transportLayer.Get_RxMessagePtr();
        modBus_MsgLength_t        msgLen       = transportLayer.Get_NewMessageLen();
        const modBus_DeviceAddr_t slaveAddr    = msgBuffer[ slaveAddr_BuffPos ];
        const modBus_FunctionCode_t funcCode   = msgBuffer[ functionCode_BuffPos ];
        bool                      listenOnly   = false;
        bool                      sendResponse = false;
        bool                      processed    = false;

        if( nullptr != diagnostics )
        {
            diagnostics->Set_RequestReceived( slaveAddr );

            listenOnly = diagnostics->Get_ListenOnlyMode();
        }
        else
        {
            /* Diagnostics not used */
        }

        if( ( true  == listenOnly                                    ) &&
            ( false == IsAllowedInListenOnlyMode( msgBuffer, msgLen ) )    )
        {
            /* Request is ignored in Listen Only Mode */
        }
        else
        {
            ModBus::FunctionsHandler * const handler = FindHandler( funcCode );

            processed = true;

            if( nullptr != handler )
            {
                errorStatus = handler->SlaveDecodeMessage( msgBuffer, msgLen );
            }
            else
            {
                /* Function code is not supported */
                errorStatus = MODBUS_ERROR_ILLEGAL_FUNCTION;
            }
        }

        /* No response for broadcast request and in Listen Only Mode (also if it was just entered) */
        if( ( MODBUS_BROADCAST_ADDR != slaveAddr  ) &&
            ( false                 == listenOnly )    )
        {
            sendResponse = ( nullptr == diagnostics ) || ( false == diagnostics->Get_ListenOnlyMode() );
        }
        else
        {
            sendResponse = false;
        }

        if( true == sendResponse )
        {
            if( MODBUS_ERROR_NO_ACTIVE_ERROR != errorStatus )
            {
                ProcessException( msgBuffer, errorStatus, msgLen );
            }
            else
            {
                /* Response prepared by handler */
            }
        }
        else
        {
            msgLen = 0u;
        }

        if( nullptr == diagnostics )
        {
            /* Diagnostics not used */
        }
        else if( true == processed )
        {
            diagnostics->Set_RequestProcessed( funcCode, errorStatus, sendResponse );
        }
        else
        {
            diagnostics->Set_RequestIgnored();
        }

        transportLayer.Set_MessageReady( msgBuffer, msgLen );
    }
    else
    {
        /* No new message for processing */
    }
}


/**
 * \brief Finds handler of function code
 *
 * \param functionCode [in]: Received function code
 *
 * \return Handler or null pointer if function code is not supported
 */
ModBus::FunctionsHandler *ModBus::AppLayer::Slave::AppCore::FindHandler( modBus_FunctionCode_t functionCode ) const
{
    ModBus::FunctionsHandler *handler = nullptr;

    for( uint8_t functionId = 0u; functionCount > functionId; functionId++ )
    {
        if( ( nullptr               != functionHandlers[ functionId ]                                 ) &&
            ( MODBUS_FUNCTION_EQUAL == functionHandlers[ functionId ]->GetFunctionStatus( functionCode ) ) )
        {
            handler = functionHandlers[ functionId ];

            break;
        }
        else
        {
            /* Continue with finding correct function handler */
        }
    }

    return ( handler );
}


/**
 * \brief Checks if request is processed in Listen Only Mode
 *
 * Only Diagnostics - Restart Communications Option is processed.
 *
 * \param msgBuffer [in]: Request
 * \param msgLength [in]: Request length without CRC
 *
 * \return True if request shall be processed
 */
bool ModBus::AppLayer::Slave::AppCore::IsAllowedInListenOnlyMode( const modBus_Message_t * const msgBuffer,
                                                                  modBus_MsgLength_t msgLength ) const
{
    bool isAllowed = false;

    if( ( MODBUS_FUNCTION_DIAG == msgBuffer[ functionCode_BuffPos ] ) &&
        ( ( diagSubFunction_BuffPos + 2u ) <= msgLength             ) &&
        ( diagRestartSubFunction == ModBus_Get_16bFromTwo8b( msgBuffer[ diagSubFunction_BuffPos ],
                                                             msgBuffer[ diagSubFunction_BuffPos + 1u ] ) ) )
    {
        isAllowed = true;
    }
    else
    {
        isAllowed = false;
    }

    return ( isAllowed );
}


/**
 * \brief Generates exception response.
 *
 * \param msgBuffer [in/out]: Pointer to message buffer
 * \param errorId       [in]: Error identification code
 * \param msgLength    [out]: Length of message without CRC
 */
void ModBus::AppLayer::Slave::AppCore::ProcessException( modBus_Message_t * const msgBuffer, modBus_ErrorStatus_t errorId, modBus_MsgLength_t &msgLength )
{
    if( nullptr != msgBuffer )
    {
        msgBuffer[ functionCode_BuffPos ] = msgBuffer[ functionCode_BuffPos ] | errorMask;

        msgBuffer[ exceptionCode_BuffPos ] = (uint8_t)errorId;

        msgLength = ExceptionMessageLen;
    }
    else
    {
        msgLength = 0u;
    }
}

/* =============================== TASKS =================================== */
