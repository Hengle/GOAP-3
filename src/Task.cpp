/*
* Copyright (C) 2017-2019, Yuriy Levchenko <irov13@mail.ru>
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#include "GOAP/Task.h"
#include "GOAP/Source.h"
#include "GOAP/Chain.h"

namespace GOAP
{
    //////////////////////////////////////////////////////////////////////////
    Task::Task()
    {

    }
    //////////////////////////////////////////////////////////////////////////
    Task::~Task()
    {

    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onInitialize()
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::_onFinalize()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onValidate() const
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onCheck()
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onRun( NodeInterface * _task )
    {
        (void)_task;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onSkipable() const
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::_onSkipNoSkiped()
    {
        //Empty
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onSkipBlock()
    {
        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::_onComplete()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onFastSkip()
    {
        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::_onSkip()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::_onCancel()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::_onFinally()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onCheckRun( const NodeInterface * _task ) const
    {
        bool result = _task->isEmptyPrevs();

        return result;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onCheckSkip( const NodeInterface * _task ) const
    {
        bool result = _task->isEmptyPrevs();

        return result;
    }
}