/*
 * JsonHelper.cpp
 *
 *  Created on: 04.11.2019
 *      Author: fsonntag
 */

#include "JsonHelper.h"

#include "X54AppGlobals.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int JsonHelper::getChildCountFromJson( cJSON * const obj )
{
    int count = 0;

    cJSON * child = obj->child;

    if ( child )
    {
        while ( child )
        {
            child = child->next;
            count++;
        }
    }

    return count;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool JsonHelper::getStringFromJson( const cJSON * const obj,
                                    const char * const  pszKeyName,
                                    char * const        pBuffer,
                                    const uint16_t      maxBufferLength )
{
    char * pszString = cJSON_GetStringValue( cJSON_GetObjectItem( obj, pszKeyName ) );

    if ( pszString )
    {
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pBuffer,
                                                      pszString,
                                                      maxBufferLength );
    }

    return pszString != nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void JsonHelper::setStringInJson( cJSON * const      obj,
                                  const char * const pszKeyName,
                                  const char *       pszString )
{
    cJSON * pJsonItem = cJSON_CreateString( pszString );

    if ( pJsonItem )
    {
        cJSON_ReplaceItemInObject( obj, pszKeyName, pJsonItem );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool JsonHelper::getIntegerFromJson( const cJSON * const obj,
                                     const char * const  pszKeyName,
                                     int32_t &           n32Value )
{
    bool bSuccess = false;

    cJSON * pJsonItem = cJSON_GetObjectItem( obj, pszKeyName );

    if ( pJsonItem != nullptr )
    {
        n32Value = pJsonItem->valueint;
        bSuccess = true;
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void JsonHelper::setIntegerInJson( cJSON * const      obj,
                                   const char * const pszKeyName,
                                   const int          val )
{
    cJSON * pJsonItem = cJSON_GetObjectItem( obj, pszKeyName );

    if ( pJsonItem )
    {
        cJSON_SetIntValue( pJsonItem, val );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool JsonHelper::getNumberFromJson( const cJSON * const obj,
                                    const char * const  pszKeyName,
                                    double &            dValue )
{
    bool bSuccess = false;

    cJSON * pJsonItem = cJSON_GetObjectItem( obj, pszKeyName );

    if ( pJsonItem != nullptr )
    {
        dValue   =  pJsonItem->valuedouble;
        bSuccess = true;
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void JsonHelper::setNumberInJson( cJSON * const      obj,
                                  const char * const pszKeyName,
                                  const double       val )
{
    cJSON * pJsonItem = cJSON_GetObjectItem( obj, pszKeyName );

    if ( pJsonItem )
    {
        cJSON_SetNumberValue( pJsonItem, val );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool JsonHelper::getBoolFromJson( const cJSON * const obj,
                                  const char * const  pszKeyName,
                                  bool &              bValue )
{
    bool bSuccess = false;

    cJSON * pJsonItem = cJSON_GetObjectItem( obj, pszKeyName );

    if ( pJsonItem )
    {
        if ( cJSON_True == pJsonItem->type )
        {
            bValue   = true;
            bSuccess = true;
        }
        else if ( cJSON_False == pJsonItem->type )
        {
            bValue   = false;
            bSuccess = true;
        }
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void JsonHelper::setBoolInJson( cJSON * const      obj,
                                const char * const pszKeyName,
                                const int          val )
{
    if ( 0 == val )
    {
        cJSON * pJsonItem = cJSON_CreateFalse();

        if ( pJsonItem )
        {
            cJSON_ReplaceItemInObject( obj, pszKeyName, pJsonItem );
        }
    }
    else
    {
        cJSON * pJsonItem = cJSON_CreateTrue();

        if ( pJsonItem )
        {
            cJSON_ReplaceItemInObject( obj, pszKeyName, pJsonItem );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

