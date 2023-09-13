/*
* ============================================================================
* RDK MANAGEMENT, LLC CONFIDENTIAL AND PROPRIETARY
* ============================================================================
* This file (and its contents) are the intellectual property of RDK Management, LLC.
* It may not be used, copied, distributed or otherwise  disclosed in whole or in
* part without the express written permission of RDK Management, LLC.
* ============================================================================
* Copyright (c) 2014 RDK Management, LLC. All rights reserved.
* ============================================================================
*/

/**
* @addtogroup HPK HPK
* @{
**/
/**
* @defgroup CLOSED_CAPTIONS_HAL Closed Captions HAL
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
 * @defgroup CCREADER CC Reader
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

#ifndef CCDATA_H
#define CCDATA_H
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
typedef enum _VL_CC_DATA_TYPE
{
  VL_CC_DATA_TYPE_608 = 0, /**< CEA-608 standard closed captions */
  VL_CC_DATA_TYPE_708 = 1, /**< CEA-708 standard closed captions */
  VL_CC_DATA_TYPE_XDS = 2, /**< CEA-608 Extended Data Services (XDS) metadata */
  VL_CC_DATA_TYPE_MAX      /**< Out of range */
}VL_CC_DATA_TYPE;

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
 * @param [in] context        Context pointer that was passed to ::vlhal_cc_Register()
 * @param [in] decoderIndex   Decoder ID from where this closed caption data comes from
 * @param [in] eType          Type of closed caption data, VL_CC_DATA_TYPE(eg: VL_CC_DATA_TYPE_608 or VL_CC_DATA_TYPE_708)
 * @param [in] ccData         Pointer to the buffer holding the closed caption data
 * @param [in] dataLength     Size of the buffer in bytes
 * @param [in] sequenceNumber Current decode sequence number
 * @param [in] localPts       Local PTS value
 *
 * @return None
 */
typedef void (* ccDataCallback) (void *context, int decoderIndex, VL_CC_DATA_TYPE eType,
                                 unsigned char* ccData, unsigned dataLength,
                                 int sequenceNumber, long long localPts);


/**
 * @brief Callback function used to notify start and stop of decoding
 * 
 * When decoding is started, the event parameter will be set to CONTENT_PRESENTING_EVENT
 * When decoding is stopped, the event parameter will be set to PRESENTATION_SHUTDOWN_EVENT
 *
 * @param [in] context       Context pointer that was passed to ::vlhal_cc_Register()
 * @param [in] decoderIndex  Decoder ID from where this notification comes from
 * @param [in] event         Event type(CONTENT_PRESENTING_EVENT or PRESENTATION_SHUTDOWN_EVENT)
 *
 * @return None
 */
typedef void (* ccDecodeCallBack) (void *context, int decoderIndex, int event);


/*
 * Interfaces
 */


  /**
  * @brief Registers callback functions for closed caption handling
  *
  * This function allows the caller to register two types of callback functions:
  * - data_callback: A callback function that is called when new closed caption data is available
  * - decode_callback: A callback function that is called to notify the caller about the start
  *                    or stop of closed caption decoding
  * 
  * @param [in] decoderIndex    The Decoder ID to obtain the closed caption data from
  * @param [in] context         A context pointer to be forwarded to the callback calls
  * @param [in] data_callback   Pointer to the callback function for handling new closed caption data
  * @param [in] decode_callback Pointer to the callback function for decode start/stop notifications
  *
  * @return int - Status
  * @retval 0  Successfully registered callback functions
  * @retval -1 Failed to register callback functions 
  *
  * @note The data_callback() will be invoked whenever new closed caption data is available,
  * allowing the caller to process the data accordingly
  * 
  * The decode_callback() will be triggered when closed caption decoding starts or stops.
  * Events like ::CONTENT_PRESENTING_EVENT or ::PRESENTATION_SHUTDOWN_EVENT will be conveyed
  * to the caller on ::media_closeCaptionStart() and ::media_closeCaptionStop() calls.
  * 
  * @todo Common prefix to be added for all APIs in this header
  */
int vlhal_cc_Register(int decoderIndex, void *context,  ccDataCallback data_callback,
                           ccDecodeCallBack decode_callback);
   

/**
  * @brief Returns the current decode sequence number
  *
  * This function is used to retrieve the current decode sequence number, which is a
  * unique identifier for the current state of the decoder
  *
  * The decode sequence number is updated when the decoder starts and stops using the
  * following pseudo code:
  * @code
    {
        gCCDecodeSeq++;
        if (gCCDecodeSeq == 65535)
            gCCDecodeSeq = 0;
    }
    @endcode
  *
  * @note The decode sequence number is also passed as the sequenceNumber in the ccDataCallback().
  *       This allows the caller to identify and associate received closed caption data with the
  *       corresponding decode sequence number.
  *
  * @note The decode sequence number can be useful when dealing with multiple decoders.
  * The caller can use this number to distinguish between correct and potentially 
  * outdated or irrelevant events.
  * @note Will be deprecated in future version
  *
  * @return int The current decode sequence number
  * @retval range 0-65535
  */
int vlhal_cc_DecodeSequence(void);


/**
  * @brief Starts closed caption decoding for the specified video decoder
  *
  * This function initiates closed caption decoding for a given video decoder. 
  * After invoking this call, data_callback() will be triggered as new 
  * closed caption data becomes available. The decode sequence number
  * will also be incremented.
  *
  * @param [in] pVidDecHandle Handle of the video decoder to retrieve the
  * closed caption data from
  *
  * @return int - Status
  * @retval 0  Successfully started decoding
  * @retval -1 Failed to start decoding
  *
  * @see vlhal_cc_DecodeSequence()
  * @note Before invoking this function, ensure that vlhal_cc_Register() has been called 
  *       to register the required callback functions. Starting decoding without
  *       proper registration may lead to unexpected behavior or incorrect data processing.
  * @todo Handle validity check - Return unique values for all error scenarios as enums and 
  * this should used across all APIs in this file
  *
  */
int media_closeCaptionStart(void* pVidDecHandle);


/**
  * @brief Stops closed caption decoding
  *
  * This function is used to stop closed caption decoding. After invoking this call, 
  * data ready callbacks for closed caption data will be halted.
  * The decode sequence number will also be incremented.
  *
  * @return int - Status
  * @retval 0  Successfully stopped decoding
  * @retval -1 Failed to stop decoding
  *
  * @see vlhal_cc_DecodeSequence()
  * @note Before invoking this function, ensure that `media_closeCaptionStart` 
          has been called to initiate decoding. Stopping decoding without first
  *       starting it may lead to unexpected behavior or incorrect data processing.
  * @todo pass handle as input argument
  *
  */
int media_closeCaptionStop(void);

#ifdef __cplusplus
}
#endif
#endif //CCDATA_H     
/** @} */ // End of CC Reader
/** @} */ // End of Closed Captions Hal
/** @} */ // End of HPK

