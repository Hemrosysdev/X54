/*
 * TraceObserverImpl.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef TRACEOBSERVERIMPL_H
#define TRACEOBSERVERIMPL_H

#include "EspLog.h"
#include "sc_tracing.h"

template<typename T>
class TraceObserverImpl : public sc::trace::TraceObserver<T>, public UxEspCppLibrary::EspLog
{
public:

    TraceObserverImpl() : UxEspCppLibrary::EspLog( "TraceObserverImpl" )
    {
    }

    virtual ~TraceObserverImpl() override
    {
    }

    void stateEntered( T state ) override
    {
        vlogInfo( "stateEntered %d", state );
    }

    void stateExited( T state ) override
    {
        vlogInfo( "stateExited %d", state );
    }

};

#endif /* TRACEOBSERVERIMPL_H */
