/*
 * PluginTaskAt24c16Distributor.h
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

#ifndef PluginTaskAt24c16Distributor_H
#define PluginTaskAt24c16Distributor_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTaskDistributor.h"

#include <string>
#include <freertos/FreeRTOS.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskAt24c16Distributor : public PluginTaskDistributor
{   

public:

    PluginTaskAt24c16Distributor();

    virtual ~PluginTaskAt24c16Distributor();

    BaseType_t  sendEepromData( const bool          bValid,
                                const std::string & strProductNo,
                                const std::string & strSerialNo );
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskAt24c16Distributor_H */
