/*!*****************************************************************************
*
* @file CommonHelper.cpp
*
* @brief Implementation file of class CommonHelper.
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Forschung & Entwicklung
*
* @created 03.06.2021
*
* @copyright Copyright 2021 by Ultratronik GmbH.
*
* This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
* With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
* software may not be copied, redistributed or used in any other way than stated in the conditions of the
* SoLiT.
*
*******************************************************************************/

#include "CommonHelper.h"

#include <cstdio>                   // NOSONAR required for printf
#include <cJSON.h>
#include <esp_log.h>

#define HEX_NUMBERS_PER_LINE 16
#define MAX_FULL_DUMP_SIZE   1024
#define MIN_DUMP_LINES_NUM   5

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace ApplicationGlobals
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const char CommonHelper::m_cBase64EncodingTable[CommonHelper::Base64EncodingTableSize] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

const int CommonHelper::m_nBase64ModTable[CommonHelper::Base64ModTableSize] = { 0, 2, 1 };

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CommonHelper::~CommonHelper()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void CommonHelper::dumpHexData( const char * const    pszPrompt,
                                const uint8_t * const pData,
                                const size_t          u32DataSize )
{
    printf( "(%d) %s: size %zd", esp_log_timestamp(), pszPrompt, u32DataSize );    // NOSONAR  printf used for uncolored debug output

    if ( u32DataSize < MAX_FULL_DUMP_SIZE )
    {
        for ( size_t i = 0; i < u32DataSize; i++ )
        {
            if ( ( i % HEX_NUMBERS_PER_LINE ) == 0 )
            {
                printf( "\n    %5zd: ", i );                         // NOSONAR  printf used for uncolored debug output
            }

            printf( "%02x ", pData[i] );                    // NOSONAR  printf used for uncolored debug output
        }
    }
    else
    {
        for ( size_t i = 0; i < MIN_DUMP_LINES_NUM * HEX_NUMBERS_PER_LINE; i++ )
        {
            if ( ( i % HEX_NUMBERS_PER_LINE ) == 0 )
            {
                printf( "\n    %5zd: ", i );                         // NOSONAR  printf used for uncolored debug output
            }

            printf( "%02x ", pData[i] );                    // NOSONAR  printf used for uncolored debug output
        }
        printf( "\n                ..." );                          // NOSONAR  printf used for uncolored debug output
        for ( size_t i = ( ( u32DataSize / HEX_NUMBERS_PER_LINE ) - MIN_DUMP_LINES_NUM ) * HEX_NUMBERS_PER_LINE;
              i < u32DataSize;
              i++ )
        {
            if ( ( i % HEX_NUMBERS_PER_LINE ) == 0 )
            {
                printf( "\n    %5zd: ", i );                         // NOSONAR  printf used for uncolored debug output
            }

            printf( "%02x ", pData[i] );                    // NOSONAR  printf used for uncolored debug output
        }
    }
    printf( "\n" );                                     // NOSONAR  printf used for uncolored debug output
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
CommonHelper::stringlist CommonHelper::splitString( const std::string & strSubject,
                                                    const std::string & strDelimiter )
{
    stringlist listResult;

    size_t i = 0;
    while ( true )
    {
        const size_t u32Found = strSubject.find( strDelimiter, i );

        if ( u32Found == std::string::npos )
        {
            listResult.push_back( strSubject.substr( i ) );
            break;
        }

        listResult.push_back( strSubject.substr( i, u32Found - i ) );
        i = u32Found + strDelimiter.size();
    }

    return listResult;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void CommonHelper::cJSON_PrintToStdString( const cJSON * const pJsonObj,
                                           std::string &       strDest )
{
    char * const pszJson = cJSON_Print( pJsonObj );
    strDest = pszJson;
    free( pszJson );        // NOSONAR cJSON_Print creates an object on the heap which has to be freed
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
CommonHelper::Base64EncodeError CommonHelper::safeBase64Encode( const uint8_t * const pu8InputData,
                                                                const size_t          u32InputDataLength,
                                                                char * const          pszOutputData,
                                                                const size_t          u32OutputBufferLength,
                                                                size_t * const        pu32OutputDataLength )
{
    Base64EncodeError nReturn = Base64EncodeError::Ok;

    if ( pu8InputData == nullptr )
    {
        nReturn = Base64EncodeError::InputBufferNull;
    }
    else if ( pszOutputData == nullptr )
    {
        nReturn = Base64EncodeError::OutputBufferNull;
    }

    else
    {
        pszOutputData[0] = '\0';

        *pu32OutputDataLength = 4 * ( ( u32InputDataLength + 2 ) / 3 );

        if ( *pu32OutputDataLength >= u32OutputBufferLength )
        {
            nReturn = Base64EncodeError::OutputBufferTooSmall;
        }
        else
        {
            char *                pszOutputDataTemp = pszOutputData;
            const uint8_t *       pu8InputDataTemp  = pu8InputData;
            const uint8_t * const pu8InputDataEnd   = pu8InputData + u32InputDataLength;
            while ( pu8InputDataTemp < pu8InputDataEnd )
            {
                uint32_t u32OctetA = 0;
                if ( pu8InputDataTemp < pu8InputDataEnd )
                {
                    u32OctetA = *pu8InputDataTemp++;
                }
                uint32_t u32OctetB = 0;
                if ( pu8InputDataTemp < pu8InputDataEnd )
                {
                    u32OctetB = *pu8InputDataTemp++;
                }
                uint32_t u32OctetC = 0;
                if ( pu8InputDataTemp < pu8InputDataEnd )
                {
                    u32OctetC = *pu8InputDataTemp++;
                }

                const uint32_t u32Triple = ( u32OctetA << 0x10 ) | ( u32OctetB << 0x08 ) | u32OctetC;

                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( 3 * 6 ) ) & 0x3F ];
                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( 2 * 6 ) ) & 0x3F ];
                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( 1 * 6 ) ) & 0x3F ];
                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( 0 * 6 ) ) & 0x3F ];
            }

            for ( int i = 0; i < m_nBase64ModTable[u32InputDataLength % Base64ModTableSize]; i++ )
            {
                pszOutputData[*pu32OutputDataLength - 1 - i] = '=';
            }

            pszOutputData[*pu32OutputDataLength] = '\0';
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace ApplicationGlobals */

