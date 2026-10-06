/**
 * \author Mr.Nobody
 * \file ModBus_Data.hpp
 * \ingroup ModBus
 * \brief Application data accessed through ModBus (callbacks used by
 *        function handlers configuration).
 *
 * Data map of the example:
 *
 * | Type             | Address | Access | Content                          |
 * |------------------|---------|--------|----------------------------------|
 * | Holding register | 0       | R/W    | Temperature set-point [0.01 °C]  |
 * | Holding register | 1       | R/W    | Operation mode (0 - 2)           |
 * | Holding register | 2       | R/W    | Configuration flags              |
 * | Input register   | 0       | R      | Measured temperature [0.01 °C]   |
 * | Input register   | 1       | R      | Uptime [s]                       |
 * | Coil             | 0       | R/W    | Relay output                     |
 * | Coil             | 1       | R/W    | Status LED                       |
 * | Discrete input   | 0       | R      | User button                      |
 * | Discrete input   | 1       | R      | Door switch                      |
 * | FIFO queue       | 0       | R      | Last application events          |
 * | File             | 1       | R/W    | 16 records of user data          |
 */

#ifndef MODBUS_MODBUS_DATA_HPP
#define MODBUS_MODBUS_DATA_HPP

/* ============================= INCLUDES =================================== */
#include "ModBus_App_Types.hpp"             /* Application layer types        */
#include "ModBus_App_RSID.hpp"              /* Run status type                */
/* ======================== EXPORTED FUNCTIONS ============================== */

/* Holding registers */
modBus_RequestState_t  ModBus_Data_Get_SetPoint      ( modBus_RegistersValue_t *value );
modBus_RequestState_t  ModBus_Data_Set_SetPoint      ( modBus_RegistersValue_t *value );
modBus_RequestState_t  ModBus_Data_Get_Mode          ( modBus_RegistersValue_t *value );
modBus_RequestState_t  ModBus_Data_Set_Mode          ( modBus_RegistersValue_t *value );
modBus_RequestState_t  ModBus_Data_Get_ConfigFlags   ( modBus_RegistersValue_t *value );
modBus_RequestState_t  ModBus_Data_Set_ConfigFlags   ( modBus_RegistersValue_t *value );

/* Input registers */
modBus_RequestState_t  ModBus_Data_Get_Temperature   ( modBus_RegistersValue_t *value );
modBus_RequestState_t  ModBus_Data_Get_Uptime        ( modBus_RegistersValue_t *value );

/* Coils */
modBus_RequestState_t  ModBus_Data_Get_Relay         ( modBus_CoilsRetValue_t *value );
modBus_RequestState_t  ModBus_Data_Set_Relay         ( modBus_CoilsRetValue_t *value );
modBus_RequestState_t  ModBus_Data_Get_Led           ( modBus_CoilsRetValue_t *value );
modBus_RequestState_t  ModBus_Data_Set_Led           ( modBus_CoilsRetValue_t *value );

/* Discrete inputs */
modBus_RequestState_t  ModBus_Data_Get_Button        ( modBus_CoilsRetValue_t *value );
modBus_RequestState_t  ModBus_Data_Get_DoorSwitch    ( modBus_CoilsRetValue_t *value );

/* Exception status, FIFO queue, file records */
modBus_RequestState_t  ModBus_Data_Get_ExceptionStatus( uint8_t *exceptionStatus );
modBus_RequestState_t  ModBus_Data_Get_EventFifo     ( uint16_t fifoAddress, modBus_RegistersValue_t *values, uint16_t *fifoCount );
modBus_RequestState_t  ModBus_Data_Read_FileRecords  ( uint16_t fileNumber, uint16_t recordNumber,
                                                         modBus_RegistersValue_t *values, uint16_t recordLength );
modBus_RequestState_t  ModBus_Data_Write_FileRecords ( uint16_t fileNumber, uint16_t recordNumber,
                                                         modBus_RegistersValue_t *values, uint16_t recordLength );

/* Identification */
modBus_App_RunStatus_t ModBus_Data_Get_RunStatus     ( void );
modBus_RequestState_t  ModBus_Data_Get_VendorName    ( const uint8_t **objData, uint8_t *objLength );
modBus_RequestState_t  ModBus_Data_Get_ProductCode   ( const uint8_t **objData, uint8_t *objLength );
modBus_RequestState_t  ModBus_Data_Get_Revision      ( const uint8_t **objData, uint8_t *objLength );

/* Simulation of application (called cyclically) */
void                   ModBus_Data_Task         ( modBus_Time_t taskTime );

#endif /* MODBUS_MODBUS_DATA_HPP */
