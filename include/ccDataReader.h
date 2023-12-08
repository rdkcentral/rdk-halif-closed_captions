/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/***************************************************************************

/**
* @addtogroup HPK Hardware Porting Kit
* @{
**/

/**
* @defgroup Closed_Captions Closed Captions Module
* @{
**/

/**
* @defgroup Closed_Captions_HAL Closed Captions HAL
* @{
**/

/**
* @file ccDataReader.h
*
* @brief Closed Caption Data Reader Public API
*
* This API defines the HAL for the Closed Caption Data Reader interface
*
* @par Document
* Document reference
*
* @par Open Issues (in no particular order)
* -# None
*
* @par Assumptions
* -# None
*
* @par Abbreviations
* - CC:      Closed Caption
* - HAL:     Hardware Abstraction Layer
* - RDK:     Reference Design Kit
*
* @par Implementation Notes
* -# None
*
*/ 
/**
 * @defgroup CC_Data_Reader CC Data Reader
 * @{
 * The CC Data Reader provides an interface to the caller to start the 
 * closed caption data acquisition with decoder handle as input. 
 * Data can be read directly or by registering a call back function 
 * based on the platform API support.
 * The caller registers callback function with the CC Data Reader to 
 * get the ccdata. The CC Data Reader reads CC data from the cc port 
 * of the decoder and passes the data to the caller through the 
 * registered callback function.
 *
 * Described herein are the Closed Caption Data Reader (ccDataReader) HAL 
 * types and functions that are part of the Closed Caption sub-system.
 *
 * The ccDataReader implementation delivers closed caption data to the 
 * caller using a callback mechanism.
 */ 

#ifndef __CC_DATA_READER_H__
#define __CC_DATA_READER_H__

/**
 * @brief Closed caption decoding started event
 *
 * This event is used to signal the start of closed caption decoding
 */
#define CONTENT_PRESENTING_EVENT 0x05

/**
 * @brief Closed caption decoding stopped event
 *
 * This event is used to signal the stop of closed caption decoding
 */
#define PRESENTATION_SHUTDOWN_EVENT  0x08

/**
 * @brief Closed Caption data types
 *
 * This enumeration lists closed caption data types
 */
typedef enum _CLOSEDCAPTION_DATA_TYPE
{
  CLOSEDCAPTION_DATA_TYPE_608 = 0, /**< CEA-608 standard closed captions */
  CLOSEDCAPTION_DATA_TYPE_708 = 1, /**< CEA-708 standard closed captions */
  CLOSEDCAPTION_DATA_TYPE_XDS = 2, /**< CEA-608 Extended Data Services (XDS) metadata */
  CLOSEDCAPTION_DATA_TYPE_MAX      /**< Out of range */
}CLOSEDCAPTION_DATA_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Callbacks
 */

/**
 * @brief Callback function used to deliver closed caption data to the caller
 * 
 * The callback will not take ownership of ccData Buffer. It is the responsibility 
 * of the Hal to free/manage this memory.
 *
 * @param [in] ctx           Context pointer that was passed to ::Closedcaption_Register()
 * @param [in] ccDataType    Type of closed caption data, CLOSEDCAPTION_DATA_TYPE(eg: CLOSEDCAPTION_DATA_TYPE_608 or CLOSEDCAPTION_DATA_TYPE_708)
 * @param [in] ccDataBuffer  Pointer to the buffer holding the closed caption data
 * @param [in] ccDataLength  Size of the buffer in bytes
 * @param [in] lPts          Local PTS value
 *
 * @return None
 */
typedef void (* closedcaptionDataCallback) (void *ctx, CLOSEDCAPTION_DATA_TYPE ccDataType,
                                 unsigned char* ccDataBuffer, unsigned ccDataLength,
                                 long long lPts);


/**
 * @brief Callback function used to notify start and stop of decoding
 * 
 * When decoding is started, the event parameter will be set to CONTENT_PRESENTING_EVENT
 * When decoding is stopped, the event parameter will be set to PRESENTATION_SHUTDOWN_EVENT
 *
 * @param [in] ctx         Context pointer that was passed to ::Closedcaption_Register()
 * @param [in] statusEvent Event type(CONTENT_PRESENTING_EVENT or PRESENTATION_SHUTDOWN_EVENT)
 *
 * @return None
 */
typedef void (* closedcaptionDecodeCallback) (void *ctx, int statusEvent);


/*
 * Interfaces
 */

  /**
  * @brief Registers callback functions for closed caption handling
  *
  * This function allows the caller to register two types of callback functions:
  * - data_cb: A callback function that is called when new closed caption data is available
  * - decode_cb: A callback function that is called to notify the caller about the start
  *              or stop of closed caption decoding
  * 
  * @param [in] ctx          A context pointer to be forwarded to the callback calls
  * @param [in] data_cb      Pointer to the callback function for handling new closed caption data
  * @param [in] decode_cb    Pointer to the callback function for decode start/stop notifications
  *
  * @return int - Status
  * @retval 0  Successfully registered callback functions
  * @retval -1 Failed to register callback functions 
  *
  * @note The data_cb() will be invoked whenever new closed caption data is available,
  * allowing the caller to process the data accordingly
  * 
  * The decode_cb() will be triggered when closed caption decoding starts or stops.
  * Events like ::CONTENT_PRESENTING_EVENT or ::PRESENTATION_SHUTDOWN_EVENT will be conveyed
  * to the caller on ::Closedcaption_Start() and ::Closedcaption_Stop() calls.
  * 
  */
int Closedcaption_Register(void *ctx,  closedcaptionDataCallback data_cb,
                           closedcaptionDecodeCallBack decode_cb);


/**
  * @brief Starts closed caption decoding for the specified video decoder
  *
  * This function initiates closed caption decoding for a given video decoder. 
  * After invoking this call, data_cb() will be triggered as new
  * closed caption data becomes available.
  *
  * @param [in] vDecHandle Handle of the video decoder to retrieve the
  * closed caption data from
  *
  * @return int - Status
  * @retval 0  Successfully started decoding
  * @retval -1 Failed to start decoding
  *
  * @pre Closedcaption_Register()
  * @note Before invoking this function, ensure that Closedcaption_Register() has been called
  *       to register the required callback functions. Starting decoding without
  *       proper registration may lead to unexpected behavior or incorrect data processing.
  * this should used across all APIs in this file
  *
  */
int Closedcaption_Start(void* vDecHandle);


/**
  * @brief Stops closed caption decoding
  *
  * This function is used to stop closed caption decoding. After invoking this call, 
  * data ready callbacks for closed caption data will be halted.
  *
  * @return int - Status
  * @retval 0  Successfully stopped decoding
  * @retval -1 Failed to stop decoding
  *
  * @pre Closedcaption_Start()
  * @note Before invoking this function, ensure that `Closedcaption_Start`
          has been called to initiate decoding. Stopping decoding without first
  *       starting it may lead to unexpected behavior or incorrect data processing.
  *
  */
int Closedcaption_Stop(void);

#ifdef __cplusplus
}
#endif
#endif // End of __CC_DATA_READER_H__

/** @} */ // End of CC Data Reader
/** @} */ // End of Closed Captions HAL
/** @} */ // End of Closed Captions Module
/** @} */ // End of HPK

