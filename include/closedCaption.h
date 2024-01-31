/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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
* @file closedCaption.h
*
* @brief Closed Caption Public API
*
* This API defines the HAL for the Closed Caption interface
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
 * @defgroup Closed_Caption Closed Caption
 * @{
 * The Closed Caption hal provides an interface to the caller to start the 
 * closed caption data acquisition with decoder handle as input. 
 * Data can be read directly or by registering a call back function 
 * based on the platform API support.
 * The caller registers callback function with the Closed Caption hal to 
 * get the ccdata. The Closed Caption hal reads CC data from the cc port 
 * of the decoder and passes the data to the caller through the 
 * registered callback function.
 *
 * Described herein are the Closed Caption HAL 
 * types and functions that are part of the Closed Caption sub-system.
 *
 * The Closed Caption hal implementation delivers closed caption data to the 
 * caller using a callback mechanism.
 */ 

#ifndef __CLOSED_CAPTION_H__
#define __CLOSED_CAPTION_H__
#endif
/**
 * @brief Closed Caption events
 *
 * This enumeration lists closed caption status events
 */
typedef enum
{
  CLOSEDCAPTION_EVENT_CONTENT_PRESENTING = 0, /**< Closed caption decoding started event */
  CLOSEDCAPTION_EVENT_PRESENTATION_SHUTDOWN   /**< Closed caption decoding stopped event */
}closedCaption_event_t;

/**
 * @brief Closed Caption data types
 *
 * This enumeration lists closed caption data types
 */
typedef enum
{
  CLOSEDCAPTION_DATA_TYPE_608 = 0, /**< CEA-608 standard closed captions */
  CLOSEDCAPTION_DATA_TYPE_708 = 1, /**< CEA-708 standard closed captions */
  CLOSEDCAPTION_DATA_TYPE_XDS = 2, /**< CEA-608 Extended Data Services (XDS) metadata */
  CLOSEDCAPTION_DATA_TYPE_MAX      /**< Out of range */
}closedCaption_data_t;

/**
 * @brief Closed Caption status
 *
 * This enumeration lists closed caption status
 */
 typedef enum
 {
  CLOSEDCAPTION_STATUS_OK = 0,                    /**< Success */
  CLOSEDCAPTION_STATUS_INVALID_PARAM,             /**< Invalid parameter error */
  CLOSEDCAPTION_STATUS_FAILED_TO_START_DECODING,  /**< Failed to start decoding error */
  CLOSEDCAPTION_STATUS_NOT_REGISTERED,            /**< Callback not registered error */
  CLOSEDCAPTION_STATUS_ALREADY_REGISTERED,        /**< Callback already registered error */
  CLOSEDCAPTION_STATUS_ALREADY_STARTED,           /**< Decoding already started error */
  CLOSEDCAPTION_STATUS_NOT_STARTED                /**< Decoding not started error */
 }closedCaption_status_t;


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
 * @param [in] pContext      Context pointer that was passed to ::closedCaption_register()
 * @param [in] eDataType     Type of closed caption data, closedCaption_data_t(eg: CLOSEDCAPTION_DATA_TYPE_608 or CLOSEDCAPTION_DATA_TYPE_708)
 * @param [in] pDataBuffer   Pointer to the buffer holding the closed caption data
 * @param [in] uDataLength   Size of the buffer in bytes
 * @param [in] lPTS          Local PTS value
 *
 * @return None
 */
typedef void (* closedCaption_dataCallback) (void *pContext, closedCaption_data_t eDataType,
                                 unsigned char* pDataBuffer, uint32_t uDataLength,
                                 int64_t lPTS);


/**
 * @brief Callback function used to notify start and stop of decoding
 * 
 * When decoding is started, the event parameter will be set to CLOSEDCAPTION_EVENT_CONTENT_PRESENTING
 * When decoding is stopped, the event parameter will be set to CLOSEDCAPTION_EVENT_PRESENTATION_SHUTDOWN
 *
 * @param [in] pContext       Context pointer that was passed to ::closedCaption_register()
 * @param [in] eStatusEvent   Event type(CLOSEDCAPTION_EVENT_CONTENT_PRESENTING or CLOSEDCAPTION_EVENT_PRESENTATION_SHUTDOWN)
 *
 * @return None
 */
typedef void (* closedCaption_decodeCallback) (void *pContext, closedCaption_event_t eStatusEvent);


/*
 * Interfaces
 */

  /**
  * @brief Registers callback functions for closed caption handling
  *
  * This function allows the caller to register two types of callback functions:
  * - dataCallback: A callback function that is called when new closed caption data is available
  * - decodeCallback: A callback function that is called to notify the caller about the start
  *              or stop of closed caption decoding
  * 
  * @param [in] pContext          A context pointer to be forwarded to the callback calls
  * @param [in] dataCallback      Pointer to the callback function for handling new closed caption data
  * @param [in] decodeCallback    Pointer to the callback function for decode start/stop notifications
  *
  * @return closedCaption_status_t - Status
  * @retval CLOSEDCAPTION_STATUS_OK  Successfully registered callback functions
  * @retval CLOSEDCAPTION_STATUS_INVALID_PARAM  Invalid Param
  * @retval CLOSEDCAPTION_STATUS_ALREADY_REGISTERED Already registered with same values    
  *
  * @note The dataCallback() will be invoked whenever new closed caption data is available,
  * allowing the caller to process the data accordingly
  * 
  * The decodeCallback() will be triggered when closed caption decoding starts or stops.
  * Events like ::CLOSEDCAPTION_EVENT_CONTENT_PRESENTING or ::CLOSEDCAPTION_EVENT_PRESENTATION_SHUTDOWN will be conveyed
  * to the caller on ::closedCaption_start() and ::closedCaption_stop() calls.
  * 
  */
closedCaption_status_t closedCaption_register(void *pContext,  closedCaption_dataCallback dataCallback,
                           closedCaption_decodeCallback decodeCallback);


/**
  * @brief Starts closed caption decoding for the specified video decoder
  *
  * This function initiates closed caption decoding for a given video decoder. 
  * After invoking this call, dataCallback() will be triggered as new
  * closed caption data becomes available.
  *
  * @param [in] pVideoDecoderHandle Handle of the video decoder to retrieve the
  * closed caption data from
  *
  * @return closedCaption_status_t - Status
  * @retval CLOSEDCAPTION_STATUS_OK  Successfully started decoding
  * @retval CLOSEDCAPTION_STATUS_INVALID_PARAM  Invalid Param
  * @retval CLOSEDCAPTION_STATUS_FAILED_TO_START_DECODING Failed to start decoding
  * @retval CLOSEDCAPTION_STATUS_NOT_REGISTERED Callbacks not registered
  * @retval CLOSEDCAPTION_STATUS_ALREADY_STARTED Already started decoding
  *
  * @pre closedCaption_register()
  * @note Before invoking this function, ensure that closedCaption_register() has been called
  *       to register the required callback functions. Starting decoding without
  *       proper registration may lead to unexpected behavior or incorrect data processing.
  * this should used across all APIs in this file
  *
  */
closedCaption_status_t closedCaption_start(void* pVideoDecoderHandle);


/**
  * @brief Stops closed caption decoding
  *
  * This function is used to stop closed caption decoding. After invoking this call, 
  * data ready callbacks for closed caption data will be halted.
  *
  * @return closedCaption_status_t - Status
  * @retval CLOSEDCAPTION_STATUS_OK  Successfully stopped decoding
  * @retval CLOSEDCAPTION_STATUS_NOT_REGISTERED Callbacks not registered
  * @retval CLOSEDCAPTION_STATUS_NOT_STARTED Decoding not started
  *
  * @pre closedCaption_start()
  * @note Before invoking this function, ensure that `closedCaption_start`
          has been called to initiate decoding. Stopping decoding without first
  *       starting it may lead to unexpected behavior or incorrect data processing.
  *
  */
closedCaption_status_t closedCaption_stop(void);

#ifdef __cplusplus
}
#endif
#endif // End of __CLOSED_CAPTION_H__

/** @} */ // End of Closed Caption
/** @} */ // End of Closed Captions HAL
/** @} */ // End of Closed Captions Module
/** @} */ // End of HPK

