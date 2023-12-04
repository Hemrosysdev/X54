/*
 * E54TestApp.h
 *
 *  Created on: 18.11.2019
 *      Author: mschmidl
 */

#ifndef E54TESTAPP_H
#define E54TESTAPP_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
#include "EspLog.h"

#include "CliTask.h"
#include "CliUartTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class E54TestApp : public EspLog
{
public:

    E54TestApp();
    virtual ~E54TestApp();

    void create();

    CliTask & cliTask();

    CliUartTask & cliUartTask();

private:

private:

    CliTask            m_cliTask;

    CliUartTask        m_cliUartTask;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

extern E54TestApp e54TestApp;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* E54TESTAPP_H */