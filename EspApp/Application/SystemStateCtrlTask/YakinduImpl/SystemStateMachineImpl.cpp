/*
 * SystemStateMachineImpl.cpp
 *
 *  Created on: 13.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateMachineImpl.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateMachineImpl::SystemStateMachineImpl( SystemStateCtrlTask * pSystemStateCtrlTask )
    : m_pSystemStateCtrlTask( pSystemStateCtrlTask )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateMachineImpl::~SystemStateMachineImpl()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

sc_integer SystemStateMachineImpl::getNumberOfParallelTimeEvents()
{
    return 10;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
