# ModBus Module

ModBus RTU middleware written in C++ (C++17, no dynamic memory, no exceptions)
for embedded targets. The module contains a complete **slave** stack (physical
layer interface, transport layer, application layer with handlers of all
public function codes and serial line diagnostics) and a **master** transport
layer.

## Repository structure

The component follows the same layout as other vendored middlewares (e.g.
FreeRTOS) - it is located in `Middlewares/ThirdParty/ModBus` of the project:

```text
Middlewares/
├── ModBus/                  Project handler module - created from Example/,
│   ├── CMakeLists.txt       owned by the project, never overwritten
│   ├── ModBus.cpp / .h      (stack composition, configuration)
│   ├── ModBus_Port.h        (ModBus_Init / ModBus_Deinit / ModBus_Task)
│   ├── ModBus_Types.h
│   ├── ModBus_Data.cpp/.hpp (application data callbacks)
│   ├── ModBus_PhyUsart.*    (physical layer over ModBus_Hal.h)
│   └── ModBus_Hal.c / .h    (hardware interface - implement for the board)
└── ThirdParty/ModBus/       This repository
    ├── CMakeLists.txt       Creates the handler module, adds the stack
    ├── Example/             Handler module template
    ├── ModBus/              ModBus RTU stack library (ModBusUser_Lib)
    │   ├── AppLayer/
    │   ├── TransportLayer/
    │   └── PhysicalLayer/
    └── Tests/UnitTests/     Host unit tests
```

### Handler module creation

During project configuration the root `CMakeLists.txt` checks the handler
module folder `Middlewares/ModBus`:

| State of `Middlewares/ModBus`                        | Action |
|------------------------------------------------------|--------|
| `ModBus.cpp` exists (example already copied)          | nothing |
| Folder does not exist or it is empty                  | example is copied and added to the build |
| Unmodified scaffold of EmBi platform (`ModBus.c` with empty `ModBus_Task`) | scaffold is replaced by the example, configuration stops once with request to configure again (the scaffold has already been processed by `Middlewares.cmake`) |
| Anything else (user's own handler module)             | nothing, warning is printed |

The handler module is part of the project - adapt `ModBus.h` (slave address,
bus configuration, task period), `ModBus_Data.cpp` (data map) and implement
`ModBus_Hal.c` (USART / DMA / receiver timeout / RS-485 driver enable). To get
a fresh copy of the example delete the folder `Middlewares/ModBus`.

When the component is not located in `Middlewares/ThirdParty` (e.g. standalone
unit testing), no handler module is created.

## Supported function codes (slave)

| Code | Name                            | Handler class | Configuration                         |
|------|---------------------------------|---------------|---------------------------------------|
| 0x01 | Read Coils                      | `RC`          | `modBus_CoilsConfigStruct_t[]`        |
| 0x02 | Read Discrete Inputs            | `RDI`         | `modBus_DiscreteIoConfigStruct_t[]`   |
| 0x03 | Read Holding Registers          | `RHR`         | `modBus_RegistersConfigStruct_t[]`    |
| 0x04 | Read Input Registers            | `RIR`         | `modBus_RegistersConfigStruct_t[]`    |
| 0x05 | Write Single Coil               | `WSC`         | `modBus_CoilsConfigStruct_t[]`        |
| 0x06 | Write Single Register           | `WSR`         | `modBus_RegistersConfigStruct_t[]`    |
| 0x07 | Read Exception Status           | `RES`         | `modBus_ExceptionStatusCallback_t`    |
| 0x08 | Diagnostics                     | `DIAG`        | `CommDiagnostics`                     |
| 0x0B | Get Comm Event Counter          | `GCEC`        | `CommDiagnostics`                     |
| 0x0C | Get Comm Event Log              | `GCEL`        | `CommDiagnostics`                     |
| 0x0F | Write Multiple Coils            | `WMC`         | `modBus_CoilsConfigStruct_t[]`        |
| 0x10 | Write Multiple Registers        | `WMR`         | `modBus_RegistersConfigStruct_t[]`    |
| 0x11 | Report Slave ID                 | `RSID`        | `modBus_App_RSID_t`                   |
| 0x14 | Read File Record                | `RFR`         | `modBus_FileRecordCallback_t`         |
| 0x15 | Write File Record               | `WFR`         | `modBus_FileRecordCallback_t`         |
| 0x16 | Mask Write Register             | `MWR`         | `modBus_RegistersConfigStruct_t[]`    |
| 0x17 | Read/Write Multiple Registers   | `RWMR`        | `modBus_RegistersConfigStruct_t[]`    |
| 0x18 | Read FIFO Queue                 | `RFIFO`       | `modBus_FifoCallback_t`               |
| 0x2B | Read Device Identification      | `RDID`        | `modBus_App_RDID_t[]` (MEI 0x0E)      |

Handlers are optional - the application registers only the functions it
needs. Any other function code is answered by exception `0x01`.

## Architecture

```text
 Application data (callbacks)
          |
 +--------v---------------------------------------------+
 | AppLayer   AppCore -> FunctionsHandler (RC, RHR, ...) |  ModBus_App*.hpp
 |            CommDiagnostics (counters, event log)      |
 +--------^---------------------------------------------+
          | request / response (address + PDU, no CRC)
 +--------v---------------------------------------------+
 | TransportLayer  Slave::TransportLayer (FSM)          |  ModBus_Slave_TL.hpp
 |                 Master::TransportLayer (FSM)         |  ModBus_Master_TL.hpp
 |                 Crc                                  |  ModBus_Crc.hpp
 +--------^---------------------------------------------+
          | frame (address + PDU + CRC)
 +--------v---------------------------------------------+
 | PhysicalLayer   interface implemented by application |  ModBus_Phy.hpp
 +------------------------------------------------------+
```

* **Physical layer** (`ModBus::PhysicalLayer`) - interface implemented by the
  application (handler module `ModBus_PhyUsart.cpp` over `ModBus_Hal.h`).
  Transport layers use it only by polling, no callbacks into the stack are
  needed.
* **Slave transport layer** - receives frames, checks length, CRC and address
  (own address or broadcast `0`), passes the request to the application layer,
  appends CRC to the response and transmits it. If no frame is received for 30
  minutes, the physical layer is re-initialized.
* **Application core** (`ModBus::AppLayer::Slave::AppCore`) - dispatches
  requests to handlers, creates exception responses, suppresses responses to
  broadcast requests and in Listen Only Mode and updates diagnostics.
* **Master transport layer** - sends one request at a time, waits for the
  response (timeout), checks CRC and slave address, handles broadcast
  turnaround delay. The master application layer (request builders) is not
  part of the module yet.

Common C types of the stack are in `ModBus_Core_Types.h` (the handler module
has its own `ModBus_Types.h`).

## Integration (slave)

The complete working integration is the handler module (`Example/ModBus.cpp`).
Its main steps:

1. **Physical layer** - derive from `ModBus::PhysicalLayer`. Reception buffer
   has to have `PhysicalLayer::messageBufferMaxLen` (256) bytes - the slave
   builds the response directly in it.
2. **Application data** - configuration arrays are indexed by the data address
   (array item 0 = address 0):

   ```cpp
   static const modBus_RegistersConfigStruct_t holdingRegisters[] =
   {
       /* Id, service status,           getter,        setter      */
       { 0u, MODBUS_SERVICE_SUPPORTED, Get_SetPoint,  Set_SetPoint },
       { 1u, MODBUS_SERVICE_SUPPORTED, Get_Mode,      Set_Mode     },
   };
   ```

3. **Stack objects** (static):

   ```cpp
   static ModBus::UsartPhysicalLayer        phy;
   static ModBus::Slave::TransportLayer     tl( phy );
   static ModBus::AppLayer::CommDiagnostics diag( tl );     /* optional */
   static ModBus::AppLayer::RHR             rhr( holdingRegisters, 2u );
   static ModBus::AppLayer::WSR             wsr( holdingRegisters, 2u );
   static ModBus::AppLayer::DIAG            diagFc( diag );

   static ModBus::FunctionsHandler * const handlers[] = { &rhr, &wsr, &diagFc };

   static ModBus::AppLayer::Slave::AppCore  core( tl, handlers, 3u, &diag );
   ```

4. **Slave address** (`tl.Set_SlaveAddr( 1...247 )`) and bus configuration of
   the physical layer before the first task.
5. **Cyclic task** - `core.Task( taskTimeMs )`. One request is processed in
   approximately 4 tasks, so the task period has to be short compared to the
   master response timeout (1 ms task is recommended).

### Data access rules

| Situation                                                   | Exception |
|-------------------------------------------------------------|-----------|
| Unsupported function code                                   | `0x01`    |
| Address (range) outside of configuration array              | `0x02`    |
| Item `MODBUS_SERVICE_NOT_SUPPORTED`                         | `0x02`    |
| Getter missing for read / setter missing for write          | `0x02`    |
| Invalid quantity, byte count, value or request length       | `0x03`    |
| Callback returned `MODBUS_REQUEST_ERROR`                    | `0x04`    |

The whole address range of a request is checked before the first callback is
called, so a request with an invalid address has no side effects. A value the
application does not accept is signaled by the setter returning
`MODBUS_REQUEST_ERROR` (exception `0x04`).

Broadcast requests (address `0`) are executed but never answered (not even by
an exception).

### Diagnostics

`ModBus::AppLayer::CommDiagnostics` is needed by `DIAG`, `GCEC` and `GCEL` and
is passed to `AppCore` as the last (optional) parameter. It provides:

* bus message count and CRC error count (from transport layer statistics),
* exception, slave message, no response, NAK and busy counters,
* communication event counter and event log (64 events),
* Listen Only Mode - entered by Diagnostics sub-function `0x04`, left only by
  Restart Communications Option (`0x01`); no responses are sent in this mode,
* diagnostic register content set by the application
  (`Set_DiagnosticRegister`).

Character overrun is not detected, its counter is always `0`. Diagnostics
sub-function `0x03` (ASCII delimiter) is answered by exception `0x01` (RTU only).

### Writing a custom handler

Derive from `ModBus::FunctionsHandler` and implement `GetFunctionCode()` and
`SlaveDecodeMessage()`:

* `msgBuffer` contains the request without CRC: `[0]` slave address,
  `[1]` function code, `[2...]` data. `msgLength` is the request length.
* Write the response into the same buffer (keep bytes 0 and 1) and set
  `msgLength` to the response length without CRC. The CRC is added by the
  transport layer.
* Return `MODBUS_ERROR_NO_ACTIVE_ERROR` or an exception code - the exception
  response is created by `AppCore`.

## Master transport layer

```cpp
ModBus::Master::TransportLayer master( phy );

uint8_t tx[ 8 ]  = { 0x11, 0x03, 0x00, 0x00, 0x00, 0x02 };    /* +2 bytes for CRC */
uint8_t rx[ ModBus::PhysicalLayer::messageBufferMaxLen ];

master.StartTransmit( 6u, tx, rx );

/* cyclically */
master.Task( 1u );

if( MODBUS_MASTER_REQ_BUSY != master.Get_RequestStatus() )
{
    /* MODBUS_MASTER_REQ_DONE - rx contains response (Get_RxMessageStatus gives length),
       otherwise TX_ERROR / TIMEOUT / CRC_ERROR / ADDR_ERROR */
}
```

Defaults: response timeout 1000 ms (`Set_ResponseTimeout`), broadcast
turnaround delay 100 ms (`Set_TurnaroundDelay`).

## Unit tests

Host unit tests (Unity) are in [Tests/UnitTests](Tests/UnitTests), registered
by `UnitTesting_AddPath()` in the root `CMakeLists.txt`:

| Test executable          | Scope                                               |
|--------------------------|-----------------------------------------------------|
| `UT_ModBus_Crc`          | CRC calculation, check and append                   |
| `UT_ModBus_SlaveTL`      | Slave transport layer state machine                 |
| `UT_ModBus_MasterTL`     | Master transport layer state machine                |
| `UT_ModBus_AppCore`      | Complete slave stack (dispatch, exceptions, ...)    |
| `UT_ModBus_AppBits`      | RC, RDI, WSC, WMC                                   |
| `UT_ModBus_AppRegisters` | RHR, RIR, WSR, WMR, MWR, RWMR, RFIFO                |
| `UT_ModBus_AppFile`      | RFR, WFR                                            |
| `UT_ModBus_AppDiag`      | RES, DIAG, GCEC, GCEL, RSID, RDID                   |
| `UT_ModBus_Example`      | Handler module template with fake `ModBus_Hal.h`    |

Physical layer is replaced by `UtModBus::FakePhy`
(`Tests/UnitTests/UtModBus_FakePhy.hpp`). Examples from the MODBUS Application
Protocol Specification are used as test vectors where available.

Standalone (only EmBi platform and this repository, see
`EmBi_Platform/CMake/UnitTesting/README.md`):

```sh
git clone <EmBi_Platform repository> EmBi_Platform
git clone <ModBus repository> ModBus
cmake -DMODULE_PATH=ModBus -DTOOLS=SYSTEM -P EmBi_Platform/CMake/UnitTesting/Standalone/RunModuleTests.cmake
```

In project UnitTest build:

```sh
# Any board unit test preset, ModBus tests do not depend on MCU
cmake --preset nucleo-h503rb-unittest
cmake --build --preset nucleo-h503rb-unittest
ctest --preset nucleo-h503rb-unittest -L ModBus
```

## Limitations

* RTU framing only (no ASCII, no TCP).
* Inter-character / inter-frame timing (1.5 / 3.5 characters) is the
  responsibility of the physical layer implementation.
* One request is processed at a time.
* Master application layer (request builders and response decoders) is not
  implemented.

## Changes against the previous version

* Repository layout - stack moved to `ModBus/`, root contains the component
  `CMakeLists.txt` and the handler module template `Example/`, which is copied
  into `Middlewares/ModBus` of the project.
* `ModBus_Types.h` of the stack was renamed to `ModBus_Core_Types.h` (name of
  the handler module types header).
* `Crc::Add_Crc` appended CRC over `length - 1` bytes and overwrote the last
  data byte - every response except Read Holding / Input Registers was sent
  corrupted. CRC is now appended behind the data.
* Unsupported function code blocked the slave forever - it is answered by
  exception `0x01` now.
* `WSR` accessed configuration out of bounds (`address == count`), `RC` / `RDI`
  packed bits incorrectly, request lengths were not checked by any handler.
* Broadcast requests are supported (executed, no response).
* Slave standby timeout never expired, `PhyInit` was called in every task while
  the slave was inactive, statistics counters were not initialized.
* Master transport layer never delivered a response; it was rewritten.
* `RDID` did not create any response; it was rewritten.
* Handlers `WSC`, `WMC`, `RES`, `DIAG`, `GCEC`, `GCEL`, `RFR`, `WFR`, `MWR`,
  `RWMR`, `RFIFO` were added, together with `CommDiagnostics`.

API changes:

* `PhysicalLayer::Set_Transmission_Start( buffer, length )` gets the buffer to
  transmit, `Get_TxMessageBuff` was removed.
* Lengths exchanged between transport and application layer are without CRC;
  `Set_MessageReady( buffer, 0 )` means "no response".
* `FunctionsHandler::MasterEncodeMessage` and the not implemented
  `AppLayer::Master::AppCore` were removed.
* `Slave::TransportLayer::Set_SlaveAddr` returns `modBus_RequestState_t` and
  accepts addresses 1 - 247 only.
* Master: `StartTransmit` returns `modBus_RequestState_t`, new
  `Get_RequestStatus` / `Set_TurnaroundDelay`, callbacks were removed.

## License

This project is licensed under the MIT License.
