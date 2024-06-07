# Closed Captions HAL Documentation

## Version and Version History

| Date (DD-MM-YY) | Comment | Version |
| --- | --------- | --- |
| 02-08-23 | First Release | 1.0.0 |

## Table of Contents

  - [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
  - [References](#references)
  - [Description](#description)
  - [Component Runtime Execution Requirements](#component-runtime-execution-requirements)
    - [Initialization and Startup](#initialization-and-startup)
    - [Threading Model](#threading-model)
    - [Process Model](#process-model)
    - [Memory Model](#memory-model)
    - [Power Management Requirements](#power-management-requirements)
    - [Asynchronous Notification Model](#asynchronous-notification-model)
    - [Blocking calls](#blocking-calls)
    - [Internal Error Handling](#internal-error-handling)
    - [Persistence Model](#persistence-model)
  - [Non-functional requirements](#non-functional-requirements)
    - [Logging and debugging requirements](#logging-and-debugging-requirements)
    - [Memory and performance requirements](#memory-and-performance-requirements)
    - [Quality Control](#quality-control)
    - [Licensing](#licensing)
    - [Build Requirements](#build-requirements)
    - [Variability Management](#variability-management)
    - [Platform or Product Customization](#platform-or-product-customization)
  - [Interface API Documentation](#interface-api-documentation)
    - [Theory of operation](#theory-of-operation)
    - [Diagrams](#diagrams)
      - [Operational call sequence](#operational-call-sequence)

## Acronyms, Terms and Abbreviations

- `API` - Application Programming Interface
- `HAL` - Hardware Abstraction layer
- `RDK` - Reference Development Kit
- `STB` - Set Top Box
- `cb`  - Callback
- `CPU` - Central Processing Unit
- `CONTENT_PRESENTING_EVENT` - Event to notify start of closed caption data decoding
- `PRESENTATION_SHUTDOWN_EVENT` - Event to notify stop of closed caption data decoding
-  `Caller` - Any user of the interface via the `APIs`

## References

- `CEA-608 Specification` document - ANSI/CEA standard, Line 21 Data Services, ANSI/CEA-608-E
   - Link : https://webstore.ansi.org/standards/cea/cea6082008ansi
- `CEA-708 Specification` document - CEA Standard, Digital Television (DTV) Closed Captioning, CEA-708-E
   - Link : https://webstore.ansi.org/standards/cea/cea7082013

## Description

This `HAL` provides an interface to the `caller` to start and stop the closed caption data acquisition with video decoder handle as the input.

```mermaid
%%{init: {"flowchart": {"curve": "linear"}} }%%
flowchart
    Caller <--> HALIF[HAL Interface `HALIF.h`]
    HALIF <--> VendorWrapper[HAL IF Implementation 'Vendor']
    VendorWrapper <--> VendorDrivers[Vendor Drivers]
 ```
    

## Component Runtime Execution Requirements

These requirements ensure that this interface executes correctly within the run-time environment that it will be used in.
Since closed caption data is coming from video decoder, make sure that video decoders are initialized and available for further operations before the Closed Caption module is up.

### Initialization and Startup

`Caller` must call `closedCaption_register()` before calling any other `API`. Any platform specific driver initialization must be done from closedCaption_start()`. The `Caller` will have complete control over the lifecycle of this interface (from start to stop).

### Threading Model

This interface is required to be thread-safe.

### Process Model

The interface will be called by a single instantiation within a single process.

### Memory Model

Closed Captions HAL is responsible for managing all memory including the memory passed through the `closedCaption_dataCallback()`, which will be freed by the HAL once the callback returns.

### Power Management Requirements

This interface is not required to be involved in any power management funtionality.

### Asynchronous Notification Model

Events `CLOSEDCAPTION_EVENT_CONTENT_PRESENTING` and `CLOSEDCAPTION_EVENT_PRESENTATION_SHUTDOWN` must be conveyed to the caller to indicate start and stop of closed caption decoding respectively. These events are sent using `closedCaption_decodeCallback()` function.

### Blocking calls

The following callbacks may be blocked depending on the caller's internal operations, but the expectation is that these will return as fast as possible, ideally within a few milliseconds.

  1. `closedCaption_dataCallback()` - Invoked whenever new closed caption data is available
  2. `closedCaption_decodeCallback()` - Invoked during start and stop of closed caption data decoding
 
### Internal Error Handling

All `APIs` must return errors synchronously as a return argument. All internal errors should be returned to the caller.

### Persistence Model

There is no requirement to persist any settings information.

## Non-functional requirements

The following non-functional requirements are required to be supported by this interface:

### Logging and debugging requirements

This interface is required to support DEBUG, INFO, WARNING, TRACE and ERROR messages. INFO, TRACE and DEBUG should be disabled by default and enabled when required. 

If systemd is enabled, `sd_journal_print()` should be used for logging, else printf can be used.

Logging format should be as follows:

|Module name|Log level|Thread Id| Message|
|---|---|---|---|
|CC_HAL|INFO|xxx| Message|

- Module name - Name of the module
- Log Level -  ERROR, DEBUG etc
- Thread Id - Currently executing thread's Id
- Message - String Message

  Example : CC_HAL INFO 1234 <log message>

### Memory and performance requirements

This interface is required to use only minimal memory/`CPU` resources while in stopped state.

### Quality Control

* This interface is required to perform static analysis, our preferred tool is Coverity.
* Have a zero-warning policy with regards to compiling. All warnings are required to be treated as errors.
* Copyright validation is required to be performed, e.g.: Black duck, FossID.
* Use of memory analysis tools like Valgrind are encouraged to identify leaks/corruptions.
* `HAL` Tests will endeavour to create worst case scenarios to assist investigations.
* Improvements by any party to the testing suite are required to be fed back.

### Licensing

The closed caption header file is released under Apache 2.0 license. The implementation may use any license compatible with the aforementioned header file.

### Build Requirements

This interface is required to be built into shared library. The shared library must be named `librdkClosedCaption.so`. The building mechanism must be independent of Yocto.

### Variability Management

Any new `API` introduced should be implemented by all the 3rd party modules. Currently there is little to no variability expected across various implementations. Any change to the interface must be reviewed and approved by component architects and owners.

### Platform or Product Customization

This interface is not required to have any platform or product customizations.


## Interface API Documentation

`API` documentation is provided via doxygen comments in the header file.

### Theory of operation

`Caller` will initialize closed captions `HAL` interface with the callback functions and video decoder handle. `HAL` will deliver closed caption data packets via the registered callbacks aligned with the corresponding video frame. The `HAL` can read data directly from the driver or by registering a call back function based on the platform `API` support. As per the spec, closed caption data packet is sent in this byte order :  cc_type,cc_data_1,cc_data_2. `HAL` must check `process_cc_data_flag` bit as per [CEA-708/CEA-608 spec](#references) and must ignore the packets with this flag set to 0. The `HAL` must parse the `cc_valid` bit as per [CEA-708/CEA-608 spec](#references) and only the packets with `cc_valid` set to 1 must be sent to the `caller`. Caller can clear the registered callbacks by calling `closedCaption_register()` by passing NULL pointers.

Following is a typical sequence of operation:
1. Register callbacks using  `closedCaption_register()`.
2. Start closed caption data decoding using `closedCaption_start()`. The interface will continuously deliver closed caption data to `caller` in real time via callback `closedCaption_dataCallback()`.
4. When the closed caption data is no longer needed, stop caption decoding using `closedCaption_stop()`. This will stop the `HAL` callbacks.
5. Start and stop of decoding is notified to the `caller` using `closedCaption_decodeCallback()`.

### Diagrams

#### Operational call sequence


```mermaid
   sequenceDiagram
    participant caller
    participant HAL
    participant Driver
    caller->>HAL: closedCaption_register()
    caller->>HAL:closedCaption_start()
    HAL->>Driver:Initialize/setup driver to fetch Closed Caption data
    HAL-->> caller : closedCaption_decodeCallback()
    loop data decoding
        HAL->>Driver : Query for data
        Driver-->>HAL : Closed Caption data
        HAL-->>caller: closedCaption_dataCallback()
        caller->>caller:consume buffer
    end
    caller->>HAL: closedCaption_stop()
    HAL->>Driver: Stop indication
    HAL-->>caller: closedCaption_decodeCallback()
 ```

