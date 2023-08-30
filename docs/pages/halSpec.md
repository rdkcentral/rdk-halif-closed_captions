# Closed Captions HAL Documentation

## Version and Version History

| Date (DD-MM-YY) | Comment | Version |
| --- | --------- | --- |
| 02-08-23 | First Release | 1.0.0 |

## Table of Contents

- [Closed Captions HAL Documentation](#Closed-Captions-hal-documentation)
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
- `CONTENT_PRESENTING_EVENT` - Event to notify start of Closed Caption data decoding
- `PRESENTATION_SHUTDOWN_EVENT` - Event to notify stop of Closed Caption data decoding

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
    A[caller] -->|Handle| B[Closed Captions HAL]
    B --> |data| A
 ```

## Component Runtime Execution Requirements

These requirements ensure that this interface executes correctly within the run-time environment that it will be used in.

### Initialization and Startup

`Caller` must have complete control over the lifecycle of this interface(from start to stop).

### Threading Model

This interface is required to be thread-safe and may be invoked from multiple `caller` threads.

### Process Model

The interface is expected to support a single instantiation with a single process.

### Memory Model

Closed Captions `HAL` is responsible for its own memory management. The buffer used to pass Closed Caption data through `ccDataCallback()` must be managed after the callback returns.

### Power Management Requirements

This interface is not required to be involved in any power management funtionality.

### Asynchronous Notification Model

Events like `CONTENT_PRESENTING_EVENT` or `PRESENTATION_SHUTDOWN_EVENT` shall be conveyed to the `caller` to indicate start and stop of Closed Caption decoding. These events are sent using `ccDecodeCallBack()` function.

### Blocking calls

The following callbacks may block depending on the `caller's` internal operations, but will endeavour to return within few milli seconds.

  1. `ccDataCallback()` - Invoked whenever new Closed Caption data is available.
  2. `ccDecodeCallBack()` - Invoked during start and stop of Closed Caption data decoding.
 
### Internal Error Handling

All `APIs` must return errors synchronously as a return argument. The interface is responsible for managing its internal errors.

### Persistence Model

There is no requirement to persist any settings information.

## Non-functional requirements

The following non-functional requirements are required to be supported by this interface:

### Logging and debugging requirements

This interface is required to support DEBUG, INFO, WARNING, TRACE and ERROR messages. INFO, TRACE and DEBUG should be disabled by default and enabled when required.

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

The Closed Caption header file license is CLOSED.

### Build Requirements

This interface is required to be built into shared library. The shared library must be named `librdkCCReader.so`. The building mechanism must be independent of Yocto.

### Variability Management

Any new `API` introduced should be implemented by all the 3rd party modules. Currently there is little to no variability expected across various implementations. Any change to the interface must be reviewed and approved by component architects and owners.

### Platform or Product Customization

This interface is not required to have any platform or product customizations.


## Interface API Documentation

`API` documentation is provided via doxygen comments in the header file.

### Theory of operation

`Caller` will initialize Closed Captions `HAL` interface with the necessary information. `HAL` will deliver Closed Caption data packets via the registered callbacks aligned with the corresponding video frame. Data can be read directly or by registering a call back function based on the platform `API` support. As per the spec, Closed Caption data packet is sent in this byte order :  cc_type,cc_data_1,cc_data_2. `HAL` shall check `process_cc_data_flag` bit as per CEA-708 spec and shall ignore the packets with this flag set to 0. The `HAL` shall parse the `cc_valid` bit as per CEA-708 spec and only the packets with `cc_valid` set to 1 shall be sent to the `caller`.

Following is a typical sequence of operation:
1. Register callbacks using  `vlhal_cc_Register()`.
2. Start Closed Caption data decoding using `media_closeCaptionStart()`. The interface will continuously deliver Closed Caption data to `caller` in real time via callback `ccDataCallback()`.
4. When the Closed Caption data  no longer needed, stop caption decoding using `media_closeCaptionStop()`. This will stop the `HAL` callbacks.
5. Start and stop of decoding is notified to the `caller` using `ccDecodeCallBack()`.
6. `vlhal_cc_DecodeSequence()` can be called to get the decode sequence number whenever required.

### Diagrams

#### Operational call sequence


```mermaid
   sequenceDiagram
    participant caller
    participant HAL
    participant Driver
    caller->>HAL: vlhal_cc_Register()
    caller->>HAL:media_closeCaptionStart()
    HAL->>Driver:Initialize driver
    HAL-->> caller : ccDecodeCallBack()
    loop data decoding
        HAL->>Driver : Query for data
        Driver-->>HAL : Closed Caption data
        HAL-->>caller: ccDataCallback()
        caller->>caller:consume buffer
    end
    caller->> HAL: vlhal_cc_DecodeSequence()
    HAL-->>caller:Decode sequence number
    caller->>HAL: media_closeCaptionStop()
    HAL->>Driver: Stop indication
    HAL-->>caller: ccDecodeCallBack()
 ```

