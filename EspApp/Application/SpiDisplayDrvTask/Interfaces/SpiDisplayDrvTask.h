/*
 * SpiDisplayDrvTask.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef SpiDisplayDrvTask_h
#define SpiDisplayDrvTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"
#include "SpiDisplayDrvQueue.h"
#include "FreeRtosQueueTimer.h"

class Tlc5955Cluster;
class Tlc5955SpiCom;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SpiDisplayDrvTask : public UxEspCppLibrary::FreeRtosQueueTask
{
public:

    SpiDisplayDrvTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~SpiDisplayDrvTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    SpiDisplayDrvQueue * spiDisplayDrvQueue( void );

    const SpiDisplayDrvQueue * spiDisplayDrvQueue( void ) const;

    Tlc5955Cluster & ledCluster( void );

    void setStatus( const X54::spiDisplayStatus nStatus,
                    const std::string &         strContextStr );

private:

    SpiDisplayDrvTask() = delete;

private:

    UxEspCppLibrary::FreeRtosQueueTimer m_timerCommonBlink;

    Tlc5955Cluster *                    m_pLedCluster { nullptr };

    Tlc5955SpiCom *                     m_pSpiCom { nullptr };

    X54::spiDisplayStatus               m_nStatus { X54::spiDisplayStatus::spiDisplayStatus_Unknown };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* SpiDisplayDrvTask_h */
