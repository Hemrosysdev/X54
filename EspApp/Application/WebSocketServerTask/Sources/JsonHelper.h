/*
 * JsonHelper.h
 *
 *  Created on: 04.11.2019
 *      Author: fsonntag
 */

#ifndef JsonHelper_h
#define JsonHelper_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
#include <string.h>
#include <cJSON.h>

#include "WebSocketServerQueue.h"


class JsonHelper
{
public:

    static int getChildCountFromJson( cJSON * const obj );

    static bool getStringFromJson( const cJSON * const obj,
                                   const char * const  pszKeyName,
                                   char * const        pBuffer,
                                   const uint16_t      maxBufferLength );

    static void setStringInJson( cJSON * const      obj,
                                 const char * const pszKeyName,
                                 const char *       pszString );

    static bool getIntegerFromJson( const cJSON * const obj,
                                    const char * const  pszKeyName,
                                    int32_t &           n32Value );

    static void setIntegerInJson( cJSON * const      obj,
                                  const char * const pszKeyName,
                                  const int          val );

    static bool getNumberFromJson( const cJSON * const obj,
                                   const char * const  pszKeyName,
                                   double &            dValue );

    static void setNumberInJson( cJSON * const      obj,
                                 const char * const pszKeyName,
                                 const double       val );

    static bool getBoolFromJson( const cJSON * const obj,
                                 const char * const  pszKeyName,
                                 bool &              bValue );

    static void setBoolInJson( cJSON * const      obj,
                               const char * const pszKeyName,
                               const int          val );

private:

    JsonHelper() = delete;

};


#endif /* JsonHelper_h */
