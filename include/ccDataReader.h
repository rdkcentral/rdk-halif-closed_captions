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
* @file ccDataReader.h
*
* @brief Closed Caption Data Reader Public API.
*
* This API defines the HAL for the Closed Caption Data Reader interface.
*
* @par Document
* Document reference.
*
* @par Open Issues (in no particular order)
* -# None
*
* @par Assumptions
* -# None
*
* @par Abbreviations
* - CC:      Closed Caption.
* - HAL:     Hardware Abstraction Layer.
* - RDK:     Reference Design Kit.
*
* @par Implementation Notes
* -# None
*
*/ 
/**
 * @defgroup CCREADER CC Reader
 * The CC Data Reader provides an interface to the caller to start the closed caption
 * data acquisition activity with input as the decoder ID. Data can be read directly or by registering
 * a call back function based on the platform API support.
 * - With the decoder ID, the stack gets the port id for closed caption data from the video decoder.
 * - The caller registers callback function with the CC Data Reader to get the ccdata.
 * - The CC Data Reader starts a thread to read data from the cc port of the decoder and passes the data to
 * the caller through the registered callback function.
 *
 * @ingroup CLOSED_CAPTION
 *
 * @defgroup CCREADER_API CC Data Reader API
 * Described herein are the Closed Caption Data Reader (ccDataReader) HAL types and functions
 * that are part of the Closed Caption sub-system.
 *
 * The ccDataReader implementation delivers closed caption data to the caller using a callback 
 * mechanism.
 * @ingroup CCREADER
 */ 
/**
* @defgroup closedcaption
* @{
* @defgroup include
* @{
**/
#ifndef CCDATA_H
#define CCDATA_H
/**
 * @addtogroup CCREADER_TYPES CC Reader Data Types
 * @ingroup CCREADER
 * @{
 */
/**
 * @brief Closed caption decoding started event.
 *
 * This event is used to notify starting closed caption decoding.
 */
#define CONTENT_PRESENTING_EVENT 0x05 /*CCDecodeStarted*/ // The requested streams or a subset of those streams are presenting.
/**
 * @brief Closed caption decoding stopped event.
 *
 * This event is used to notify stopping closed caption decoding.
 */
#define PRESENTATION_SHUTDOWN_EVENT  0x08 /*CCDecodeStopped*/
/**
 * @brief Closed Caption data types.
 *
 * This enumartion lists closed caption data types.
 */
typedef enum _VL_CC_DATA_TYPE
{
  VL_CC_DATA_TYPE_608 = 0, /**< CEA-608 standard closed captions. */
  VL_CC_DATA_TYPE_708 = 1, /**< CEA-708 standard closed captions. */
  VL_CC_DATA_TYPE_XDS = 2, /**< CEA-608 Extended Data Services (XDS) metadata. */
}VL_CC_DATA_TYPE;
/** @} */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Callbacks
 */


/**
 * @brief Callback function used to notify new closed caption data.
 * 
 * This function is used by the ccDataReader implementation to notify and deliver new closed caption
 * data to the caller.
 * The callback will not take ownership of CCDataBuffer. It is the responsibility of the caller to free/manage this memory.
 *
 * @param [in] context        Context pointer that was passed to ::vlhal_cc_Register().
 * @param [in] decoderIndex   Decoder ID from where this closed caption data comes from.
 * @param [in] eType          Type of closed caption data (e.g. 607 or 608).
 * @param [in] ccData         Pointer to the buffer holding the closed caption data.
 * @param [in] dataLength     Size of the buffer in bytes.
 * @param [in] sequenceNumber Current decode sequence number (see ::vlhal_cc_DecodeSequence() for details).
 * @param [in] localPts       Local PTS value (not used now).
 *
 * @return None.
 * @ingroup CCREADER_API
 */
typedef void (* ccDataCallback) (void *context, int decoderIndex, VL_CC_DATA_TYPE eType, unsigned char* ccData, 
                                 unsigned dataLength, int sequenceNumber, long long localPts);
/**
 * @brief Callback function used to notify start and stop of decoding.
 * 
 * This function is used by the ccDataReader implementation to notify caller of starting 
 * and stopping of decoding as a result of ::media_closeCaptionStart() and 
 * ::media_closeCaptionStop() calls.
 *
 * @param [in] context       Context pointer that was passed to ::vlhal_cc_Register().
 * @param [in] decoderIndex  Decoder ID from where this notification comes from.
 * @param [in] event         Event type (::CONTENT_PRESENTING_EVENT if decoding started, 
 *                           ::PRESENTATION_SHUTDOWN_EVENT if decoding stopped).
 *
 * @return None.
 * @ingroup CCREADER_API
 */
typedef void (* ccDecodeCallBack) (void *context, int decoderIndex, int event);


/*
 * Interfaces
 */


/**
  * @brief Registers callback functions.
  *
  * This function is used to register data ready and decode start/stop callbacks.
  * 
  * @param [in] decoderIndex    Decoder ID to get the closed caption data from.
  * @param [in] context         Context pointer to be forwarded to the callback calls (i.e. private data).
  * @param [in] data_callback   Pointer to closed caption data ready callback function.
  * @param [in] decode_callback Pointer to decode start/stop notification callback function.
  *
  * @return Error code.
  * @retval 0  Successfully registered callback functions.
  * @retval -1 Failed to register callback functions. 
  *
  * @note Events like ::CONTENT_PRESENTING_EVENT or ::PRESENTATION_SHUTDOWN_EVENT will be conveyed
  * to the caller on ::media_closeCaptionStart and ::media_closeCaptionStop calls. ::ccDecodeCallBack can be 
  * called when a change in the presentation happens with respective decoder events (i.e. caller will 
  * do closed caption text clean up on channel change).
  * @ingroup CCREADER_API
  */
extern int vlhal_cc_Register(int decoderIndex, void *context,  ccDataCallback data_callback,  ccDecodeCallBack decode_callback);
   
/**
  * @brief Returns the current decode sequence.
  *
  * This function is used to get the current decode sequence number.
  *
  * The decode sequence is a number which is updated when starting and stopping the decoder. 
  * ::media_closeCaptionStart() and ::media_closeCaptionStop() functions increment this number by 
  * using following pseudo code:
  * @code
    {
        gCCDecodeSeq++;
        if(gCCDecodeSeq == 65535)
            gCCDecodeSeq=0;
    }
    @endcode
  *
  * @note This number is also returned in the ::ccDataCallback as the @a sequenceNumber. 
  *
  * @note Decode sequence number will be used by the caller to ignore incorrect events, 
  * probably in case of multiple decoders.
  *
  * @param None.
  *
  * @return Current decode sequence number.
  * @ingroup CCREADER_API
  */
int vlhal_cc_DecodeSequence(void);

/**
  * @brief Starts closed caption decoding.
  *
  * This function is used to start closed caption decoding. Data ready callbacks will be started after this 
  * call. Decode sequence number will be incremented (see ::vlhal_cc_DecodeSequence() for details).
  *
  * @param [in] pVidDecHandle Handle of the video decoder to retrieve the closed caption data from.
  *
  * @return Error code.
  * @retval 0  Successfully started decoding.
  * @retval -1 Failed to start decoding.
  * @pre Need to call vlhal_cc_Register before invoking this function.
  * @ingroup CCREADER_API
  */
int media_closeCaptionStart (void* pVidDecHandle);
/**
  * @brief Stops closed caption decoding.
  *
  * This function is used to stop closed caption decoding. Data ready callbacks will be stopped after this 
  * call. Decode sequence number will be incremented (see ::vlhal_cc_DecodeSequence() for details).
  *
  * @param  None.
  *
  * @return Error code.
  * @retval 0  Successfully stopped decoding.
  * @retval -1 Failed to stop decoding.
  * @pre Need to call media_closeCaptionStart before invoking this function.
  * @ingroup CCREADER_API
  */
int media_closeCaptionStop  (void);
#ifdef __cplusplus
}
#endif
#endif //CCDATA_H     
/** @} */
/** @} */
Powered by Gitiles
txt
json