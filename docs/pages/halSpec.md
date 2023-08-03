# Closed Captions HAL Documentation

## Version and Version History

| Date (DD-MM-YY) | Comment | Version |
| --- | --------- | --- |
| 02-08-23 | First Release | 1.0.0 |

## Table of Contents

- [Closed Captions HAL Documentation](#closed-captions-hal-documentation)
  - [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
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
      - [State machine of closed caption interface](#state-machine-of-closed-caption-interface)

## Acronyms, Terms and Abbreviations

- `API` - Application Programming Interface
- `HAL` - Hardware Abstraction layer
- `RDK` - Reference Development Kit
- `STB` - Set Top Box
- `cb`  - Callback
- `CPU` - Central Processing Unit
- `CONTENT_PRESENTING_EVENT` - Event to notify start of closed caption data decoding
- `PRESENTATION_SHUTDOWN_EVENT` - Event to notify stop of closed caption data decoding

## Description

Closed Captions `HAL` must deliver closed caption data to the `caller`. The Closed Captions `HAL` provides an interface to the `caller` to start the closed caption data acquisition with video decoder handle as the input.

```mermaid
%%{init: {"flowchart": {"curve": "linear"}} }%%
flowchart
    A[caller] -->|Handle| B[Closed Captions HAL]
    B --> |data| A
 ```

## Component Runtime Execution Requirements

These requirements ensure that this interface executes correctly within the run-time environment that it will be used in.

### Initialization and Startup

`Caller` must have complete control over the lifecycle of Closed Caption `HAL` (from start to stop).

### Threading Model

This interface is required to be thread-safe and may be invoked from multiple `caller` threads.

### Process Model

The interface is expected to support a single instantiation with a single process.

### Memory Model

Closed Captions `HAL` is responsible for its own memory management. The buffer used to pass closed caption data through `ccDataCallback()` must be managed after the callback returns.

### Power Management Requirements

This interface is not required to be involved in any power management funtionality.

### Asynchronous Notification Model

Events like `CONTENT_PRESENTING_EVENT` or `PRESENTATION_SHUTDOWN_EVENT` will be conveyed to the `caller` to indicate start and stop of closed caption decoding. These events are send using `ccDecodeCallBack()` function.

### Blocking calls

The following callbacks may block depending on the `caller`'s internal operations, but will endeavour to return as soon as possible.

  1. `ccDataCallback()` - Invoked whenever new closed caption data is available.
  2. `ccDecodeCallBack()` - Invoked during start and stop of closed caption data decoding.
 
### Internal Error Handling

All `API`s must return errors synchronously as a return argument. The interface is responsible for managing its internal errors.

### Persistence Model

There is no requirement to persist any settings information. The necessary parameters will be passed with `media_closeCaptionStart()` for every closed captions session.

## Non-functional requirements

The following non-functional requirements are required to be supported by this interface:

### Logging and debugging requirements

This interface is required to support DEBUG, INFO, WARNING, TRACE and ERROR messages. INFO, TRACE and DEBUG should be disabled by default and enabled when required.

### Memory and performance requirements

This interface is required to use only minimal memory/`CPU` resources while in stopped state.

### Quality Control

* This interface is required to perform static analysis, our preferred tool is Coverity.
* Have a zero-warning policy with regards to compiling. All warnings are required to be treated as error.
* Copyright validation is required to be performed, e.g.: Black duck, FossID.
* Use of memory analysis tools like Valgrind are encouraged, to identify leaks/corruptions.
* `HAL` Tests will endeavour to create worst case scenarios to assist investigations.
* Improvements by any party to the testing suite are required to be fed back.

### Licensing

The Closed Caption header file license is CLOSED.

### Build Requirements

This interface is required to be built into shared library. The shared library must be named `librdkCCReader.so`. The building mechanism must be independent of Yocto.

### Variability Management

Any new `API` introduced should be implemented by all the 3rd party modules. Currently there is little to no variability expected across various implementations. Any change to the interface must be reviewed and approved by component architects and owners.

### Platform or Product Customization

No such requirements.


## Interface API Documentation

`API` documentation is provided via doxygen comments in the header file.

### Theory of operation

`Caller` will initialize Closed Captions `HAL` interface with the necessary information. `HAL` will deliver closed caption data packets via the registered callbacks in a timely fashion. Data can be read directly or by registering a call back function based on the platform API support.

Following is a typical sequence of operation:
1. Register callbacks using  `vlhal_cc_Register()`.
2. Start closed caption data decoding using `media_closeCaptionStart()`. The interface will continuously deliver closed caption data to `caller` in real time via callback `ccDataCallback()`.
4. When the closed caption data  no longer needed, stop caption decoding using `media_closeCaptionStop()`. This will stop the `HAL` callbacks.
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
        Driver-->>HAL : closed caption data
        HAL-->>caller: ccDataCallback()
        caller->>caller:consume buffer
    end
    caller->>HAL: media_closeCaptionStop
    HAL->>Driver: Stop indication
    HAL-->>caller: ccDecodeCallBack()
 ```

#### State machine of closed caption interface


```mermaid
stateDiagram-v2
    [*] --> Started: Start()
    Started: Started\n(data decoding)
    Started --> [*]: Stop()
```

