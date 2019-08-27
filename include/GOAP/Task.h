/*
* Copyright (C) 2017-2019, Yuriy Levchenko <irov13@mail.ru>
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#pragma once

#include "GOAP/Vector.h"
#include "GOAP/Factorable.h"
#include "GOAP/IntrusivePtr.h"
#include "GOAP/Visitable.h"

namespace GOAP
{
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<class Task> TaskPtr;
    typedef IntrusivePtr<class Source> SourcePtr;
    typedef IntrusivePtr<class Chain> ChainPtr;
    //////////////////////////////////////////////////////////////////////////
    typedef Vector<TaskPtr> VectorTasks;
    //////////////////////////////////////////////////////////////////////////
    class Task
        : public Factorable
        , public Visitable
    {
        GOAP_DECLARE_VISITABLE_BASE();

    public:
        enum ETaskState
        {
            TASK_STATE_INVALID,
            TASK_STATE_IDLE,
            TASK_STATE_RUN,
            TASK_STATE_SKIP,
            TASK_STATE_STOP,
            TASK_STATE_CANCEL,
            TASK_STATE_COMPLETE,
            TASK_STATE_ERROR,
            TASK_STATE_END
        };

    public:
        Task();
        ~Task() override;

    public:
        void setChain( const ChainPtr & _chain );
        const ChainPtr & getChain() const;

    public:
        void throwError();

    public:
        bool isIdle() const;
        bool isRunning() const;
        bool isEnd() const;
        bool isSkip() const;
        bool isError() const;

    public:
        void addNext( const TaskPtr & _task );
        VectorTasks & getNexts();
        const VectorTasks & getNexts() const;
        void popNexts( VectorTasks & _clone );
        bool injectSource( const SourcePtr & _source );
        bool forkSource( const SourcePtr & _source );

    public:
        bool run( bool _checkSkipedFalse = false );
        bool skip();

        void cancel( bool _withNexts = false );

    public:
        void complete( bool _running = true, bool _skiped = false );

    protected:
        bool onInitialize();
        void onFinalize();

    protected:
        bool onValidate() const;
        bool onCheck();
        bool onRun();
        bool onSkipable() const;
        void onSkipNoSkiped();
        bool onSkipBlock();
        void onComplete();
        bool onFastSkip();
        void onSkip();
        void onCancel();
        void onFinally();
        bool onCheckRun() const;
        bool onCheckSkip() const;

    protected:
        virtual bool _onInitialize();
        virtual void _onFinalize();

    protected:
        virtual bool _onValidate() const;
        virtual bool _onCheck();
        virtual bool _onRun();
        virtual bool _onSkipable() const;
        virtual void _onSkipNoSkiped();
        virtual bool _onSkipBlock();
        virtual void _onComplete();
        virtual bool _onFastSkip();
        virtual void _onSkip();
        virtual void _onCancel();
        virtual void _onFinally();
        virtual bool _onCheckRun() const;
        virtual bool _onCheckSkip() const;

    protected:
        void taskSkip_();

        bool prevSkip_( const TaskPtr & _task );
        void skipPrev_();
        void cancelPrev_();

        bool prevComplete_( const TaskPtr & _task );

        void addPrev_( const TaskPtr & _task );
        void removePrev_( const TaskPtr & _task );
        void removeNext_( const TaskPtr & _task );
        void unlink_( const TaskPtr & _task );
        bool hasPrev_( const TaskPtr & _task ) const;
        void finalize_();

    protected:
        void setState( ETaskState _state );

    protected:
        ETaskState m_state;

        ChainPtr m_chain;

        VectorTasks m_nexts;
        VectorTasks m_prevs;

        bool m_skip;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<Task> TaskPtr;
    //////////////////////////////////////////////////////////////////////////
    namespace Helper
    {
        template<class T, class ... Args>
        IntrusivePtr<T> makeTask( Args && ... _args )
        {
            return IntrusivePtr<T>::from( new T( std::forward<Args &&>( _args ) ... ) );
        }
    }
}