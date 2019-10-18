/*
* Copyright (C) 2017-2019, Yuriy Levchenko <irov13@mail.ru>
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#include "GOAP/TaskCallback.h"
#include "GOAP/Node.h"

namespace GOAP
{
    //////////////////////////////////////////////////////////////////////////
    TaskCallback::TaskCallback( const CallbackProviderPtr & _provider )
        : m_provider( _provider )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    TaskCallback::~TaskCallback()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool TaskCallback::_onRun( NodeInterface * _task )
    {
        bool skip = _task->isSkip();

        CallbackObserverPtr callback = Helper::makeCallbackObserver( [_task]( bool _skip )
        {
            _task->complete( true, _skip );
        } );

        m_provider->onCallbackProvider( callback, skip );

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void TaskCallback::_onFinally()
    {
        m_provider = nullptr;
    }
    //////////////////////////////////////////////////////////////////////////
    bool TaskCallback::_onSkipable() const
    {
        return false;
    }
}