@mainpage

# Closedcaptions HAL Documentation

## Version and Version History

| Date (YYYY-mm-dd) | Author | Comment | Version |
| --- | --------- | --- | --- |
| 2023-07-20 | Anaswara Kookkal | First Release | 1.0.0 |

## Table of Contents

- [Closedcaptions HAL Documentation](#closedcaption-hal-documentation)
  - [Version and Version History](#version-and-version-history)
  - [Table of Contents](#table-of-contents)
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
      - [Architecture Diagram](#architecture-diagram)
    - [Diagrams](#diagrams)
      - [Operational call sequence](#operational-call-sequence)
      - [State machine of closedcaption interface](#state-machine-of-audiocapture-interface)

## Acronyms, Terms and Abbreviations

- `API` - Application Programming Interface
- `HAL` - Hardware Abstraction layer
- `RDK` - Reference Development Kit
- `STB` - Set Top Box
- `CC`  - Closedcaption
- `cb`  - Callback

## Description

Closedcaption `HAL` must deliver closedcaption data to the caller. The `CC` `HAL` provides an interface to the caller to start the closed caption data acquisition with video decoder handle as the input. Data can be read directly or by registering a call back function based on the platform API support.

## Component Runtime Execution Requirements

These requirements ensure that the `HAL` executes correctly within the run-time environment that it will be used in.

### Initialization and Startup

Caller is expected to have complete control over the lifecycle of Closedcaption `HAL` (from start to stop).

### Threading Model

This interface is required to be thread-safe and may be invoked from multiple caller threads.

### Process Model

Caller will take care of Closedcaption `HAL` initialization. The interface is expected to support a single instantiation with a single process.

### Memory Model

Closedcaption `HAL` is responsible for its own memory management. The buffer used to pass cc data through `ccDataCallback()` must be managed after the callback returns.

### Power Management Requirements

This interface is not required to be involved in any power management funtionality.

### Asynchronous Notification Model

Events like `CONTENT_PRESENTING_EVENT` or `PRESENTATION_SHUTDOWN_EVENT` will be conveyed to the caller to indicate start and stop of closedcaption decoding. These events are send using `ccDecodeCallBack` function.

### Blocking calls

The following callbacks may block depending on the caller's internal operations, but will endeavour to return as soon as possible.

  1. `ccDataCallback()`
  2. `ccDecodeCallBack()`
 
### Internal Error Handling

All APIs must return errors synchronously as a return argument. The interface is responsible for managing its internal errors.

### Persistence Model

There is no requirement to persist any settings information. The necessary parameters will be passed with `media_closecaption_start()` for every cc session.

## Non-functional requirements

The following non-functional requirements are required to be supported by this interface:

### Logging and debugging requirements

This interface shall log all errors, warnings and critical informative messages that are necessary to debug/triage.

### Memory and performance requirements

This interface is required to use only minimal memory/CPU resources while in stopped state.

### Quality Control

* The implementation is required to perform static analysis, our preferred tool is Coverity.
* Open-source copyright validation is required to be performed, e.g.: Black duck, FossID.
* Have a zero-warning policy with regards to compiling. All warnings are required to be treated as errors.
* Use of memory analysis tools like Valgrind are encouraged, to identify leaks/corruption.
* Tests will endeavour to create worst case scenarios to assist investigations.
* Improvements by any party to the testing suite are required to be fed back.

### Licensing

The Closedcaption header file license is CLOSED.

### Build Requirements

This interface is required to build into shared library. The shared library must be named `librdkCCReader.so`. The building mechanism must be independent of Yocto.

### Variability Management

Any new `API` introduced should be implemented by all the 3rd party modules. Currently there is little to no variability expected across various implementations. Any change to the interface must be reviewed and approved by component architects and owners.

### Platform or Product Customization

No such requirements.


## Interface API Documentation

`API` documentation is provided via doxygen comments in the header file.

### Theory of operation

Caller will initialize cc hal interface with the necessary information. `HAL` will deliver cc data packets via the registered callbacks in a timely fashion.

#### Architecture Diagram

```mermaid
flowchart
    A[caller] -->|Handle| B[CC HAL]
    B --> |data| A
 ```

Following is a typical sequence of operation:
1. Register callbacks using  `hal_cc_Register`.
2. Start cc data decoding using `media_closeCaptionStart()`. The interface will continuously deliver cc data to caller in real time via callback `ccDataCallback()`.
4. When the cc data  no longer needed, stop caption decoding using `media_closeCaptionStop()`. This will stop the `HAL` callbacks.
5. Start and stop of decoding is notified to the caller using `ccDecodeCallBack()`.

### Diagrams

#### Operational call sequence


```mermaid
   sequenceDiagram
    participant caller
    participant HAL
    caller->>HAL: hal_cc_Register(data cb, decode cb)
    caller->>HAL:media_closeCaptionStart(handle)
    HAL-->> caller : ccDecodeCallBack(start event)
    loop data decoding
        HAL-->>caller: ccDataCallback(cc data buffer)
        caller->>caller:consume buffer
    end
    caller->>HAL: media_closeCaptionStop
    HAL-->>caller: ccDecodeCallBack(stop event)
 ```

#### State machine of Closedcaption interface


```mermaid
stateDiagram-v2
    [*] --> Started: Start()
    Started: Started\n(data decoding)
    Started --> [*]: Stop()
```

