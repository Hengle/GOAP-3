/*
* Copyright (C) 2017, Levchenko Yuriy <irov13@mail.ru>
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#	include "GOAP/Task.h"
#   include "GOAP/Source.h"
#	include "GOAP/Chain.h"

#   include <algorithm>

namespace GOAP
{
	//////////////////////////////////////////////////////////////////////////
	Task::Task()
		: m_state( TASK_STATE_IDLE )
        , m_events( 0 )
		, m_skip( false )
	{
	}
    //////////////////////////////////////////////////////////////////////////
    Task::Task( uint32_t _events )
        : m_state( TASK_STATE_IDLE )
        , m_events( _events )
        , m_skip( false )
    {
    }
	//////////////////////////////////////////////////////////////////////////
	Task::~Task()
	{
	}
    //////////////////////////////////////////////////////////////////////////
    void Task::setEvents( uint32_t _events )
    {
        m_events = _events;
    }
    //////////////////////////////////////////////////////////////////////////
    uint32_t Task::getEvents() const
    {
        return m_events;
    }
    //////////////////////////////////////////////////////////////////////////
	void Task::setChain( const ChainPtr & _chain )
	{
		m_chain = _chain;
	}
	//////////////////////////////////////////////////////////////////////////
	const ChainPtr & Task::getChain() const
	{
		return m_chain;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::isIdle() const
	{
		return m_state == TASK_STATE_IDLE;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::isRunning() const
	{
		return m_state == TASK_STATE_RUN;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::isEnd() const
	{
		return m_state == TASK_STATE_END;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::isSkip() const
	{
		return m_skip;
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::addNext( const TaskPtr & _task )
	{
		m_nexts.push_back( _task );
		_task->addPrev_( this );
	}
	//////////////////////////////////////////////////////////////////////////
	TVectorTasks & Task::getNexts()
	{
		return m_nexts;
	}
	//////////////////////////////////////////////////////////////////////////
	const TVectorTasks & Task::getNexts() const
	{
		return m_nexts;
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::popNexts( TVectorTasks & _clone )
	{
		for( TVectorTasks::iterator
			it = m_nexts.begin(),
			it_end = m_nexts.end();
		it != it_end;
		++it )
		{
			const TaskPtr & next = *it;

			next->removePrev_( this );
		}

		_clone.swap( m_nexts );

		m_nexts.clear();
	}
    //////////////////////////////////////////////////////////////////////////
    bool Task::injectSource( const SourcePtr & _source )
    {
        TVectorTasks nexts;
        this->popNexts( nexts );

        const ChainPtr & chain = this->getChain();

        TaskPtr task = _source->parse( chain, this );

        if( task == nullptr )
        {
            return false;
        }

        for( TVectorTasks::iterator
            it = nexts.begin(),
            it_end = nexts.end();
            it != it_end;
            ++it )
        {
            const TaskPtr & next = *it;

            task->addNext( next );
        }

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	bool Task::run( bool _checkSkipedFalse )
	{
		if( m_state != TASK_STATE_IDLE )
		{
			return false;
		}

		if( this->onValidate() == false )
		{
			return false;
		}

		this->setState( TASK_STATE_RUN );

		m_chain->runTask( this );

		if( this->onInitialize() == false )
		{
			return false;
		}

		if( this->onCheck() == false )
		{
			this->complete( false );

			return true;
		}

		bool done = this->onRun();

		if( m_state != TASK_STATE_RUN )
		{
			return true;
		}

		if( done == false )
		{
			return true;
		}

		if( _checkSkipedFalse == true )
		{
			if( this->onSkipBlock() == true )
			{
				this->complete( true, false );

				return true;
			}

			if( this->onSkipable() == false )
			{
				return false;
			}
		}

		this->complete( true, false );

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::skip()
	{
		if( m_state == TASK_STATE_SKIP )
		{
			return true;
		}

		if( m_state == TASK_STATE_END )
		{
			return true;
		}

		if( m_skip == true )
		{
			return false;
		}

		if( this->onSkipable() == false )
		{
			if( m_state == TASK_STATE_IDLE )
			{
				if( this->run( true ) == false )
				{
					return false;
				}
			}
			else
			{
				this->onSkipNoSkiped();
			}

			return true;
		}

		m_skip = true;

		switch( m_state )
		{
		case TASK_STATE_IDLE:
			{
				if( this->onInitialize() == false )
				{
					return false;
				}

				m_chain->runTask( this );

                if( this->onFastSkip() == false )
                {
                    this->setState( TASK_STATE_RUN );

                    if( this->onCheck() == true )
                    {
                        bool done = this->onRun();

                        if( m_state == TASK_STATE_END )
                        {
                            return true;
                        }

                        this->setState( TASK_STATE_SKIP );

                        if( done == false )
                        {
                            this->onSkip();
                        }
                    }

                    this->onFinally();
                }

				this->taskSkip_();
			}break;
		case TASK_STATE_RUN:
			{
				this->setState( TASK_STATE_SKIP );

				this->onSkip();
				this->onFinally();

				this->taskSkip_();
			}break;
		default:
			break;
		}

		if( m_state != TASK_STATE_END )
		{
			this->finalize_();
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::cancel( bool _withNexts )
	{
		if( m_state == TASK_STATE_END )
		{
			return;
		}

		m_skip = true;

		if( m_state == TASK_STATE_RUN )
		{
			this->setState( TASK_STATE_CANCEL );

			this->onSkip();
			this->onCancel();
			this->onFinally();

			m_chain->completeTask( this );
		}

        if( _withNexts == true )
        {
            TVectorTasks copy_nexts = m_nexts;

            for( TVectorTasks::const_iterator
                it = copy_nexts.begin(),
                it_end = copy_nexts.end();
                it != it_end;
                ++it )
            {
                const TaskPtr & task = *it;

                task->cancel( _withNexts );
            }
        }

		this->finalize_();
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::taskSkip_()
	{
		if( m_state == TASK_STATE_END )
		{
			return;
		}

		m_chain->completeTask( this );

		TVectorTasks copy_nexts = m_nexts;

		for( TVectorTasks::iterator
			it = copy_nexts.begin(),
			it_end = copy_nexts.end();
		it != it_end;
		++it )
		{
			const TaskPtr & next = *it;

			if( next->prevSkip_( this ) == true )
			{
				m_chain->processTask( next, true );
			}
		}

        m_nexts.clear();
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::complete( bool _running, bool _skiped )
	{
		switch( m_state )
		{
		case TASK_STATE_SKIP:
		case TASK_STATE_END:
		case TASK_STATE_CANCEL:
			return;
		default:
			break;
		};

		if( m_state != TASK_STATE_RUN )
		{
			return;
		}

		if( _skiped == true )
		{
			m_skip = true;
			this->setState( TASK_STATE_SKIP );
		}
		else
		{
			this->setState( TASK_STATE_COMPLETE );
		}

		if( _running == true )
		{
			this->onComplete();
			this->onFinally();
		}

		this->setState( TASK_STATE_END );

		if( m_skip == false )
		{
			TVectorTasks copy_nexts = m_nexts;

			for( TVectorTasks::iterator
				it = copy_nexts.begin(),
				it_end = copy_nexts.end();
			it != it_end;
			++it )
			{
				const TaskPtr & next = *it;

				if( next->prevComplete_( this ) == true )
				{
					m_chain->processTask( next, false );
				}
			}

            m_nexts.clear();
		}
		else
		{
			TVectorTasks copy_nexts = m_nexts;

			for( TVectorTasks::iterator
				it = copy_nexts.begin(),
				it_end = copy_nexts.end();
			it != it_end;
			++it )
			{
				const TaskPtr & next = *it;

				if( next->prevSkip_( this ) == true )
				{
					m_chain->processTask( next, true );
				}
			}

            m_nexts.clear();
		}

		ChainPtr chain = m_chain;

		this->finalize_();

		chain->completeTask( this );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::prevSkip_( Task * _task )
	{
		if( m_state == TASK_STATE_END )
		{
			return false;
		}

#	ifdef _DEBUG
		if( this->hasPrev_( _task ) == false )
		{
			return false;
		}
#	endif

		this->unlink_( _task );

		if( this->onCheckSkip() == false )
		{
			return false;
		}

		this->cancelPrev_();

		if( m_state == TASK_STATE_END )
		{
			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::skipPrev_()
	{
		TVectorTasks clone_prevs = m_prevs;

		for( TVectorTasks::iterator
			it = clone_prevs.begin(),
			it_end = clone_prevs.end();
		it != it_end;
		++it )
		{
			const TaskPtr & prev = *it;

			switch( prev->m_state )
			{
			case TASK_STATE_IDLE:
				{
					prev->skipPrev_();
					prev->cancelPrev_();
				}break;
			case  TASK_STATE_RUN:
				{
					prev->skip();
					prev->cancel();
				}break;
			default:
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::cancelPrev_()
	{
		TVectorTasks clone_prevs = m_prevs;

		for( TVectorTasks::iterator
			it = clone_prevs.begin(),
			it_end = clone_prevs.end();
		it != it_end;
		++it )
		{
			const TaskPtr & prev = *it;

			switch( prev->m_state )
			{
			case TASK_STATE_IDLE:
				{
					prev->cancelPrev_();
					prev->cancel();
                    this->unlink_( prev.get() );
				}break;
			case  TASK_STATE_RUN:
				{
					prev->cancel();
                    this->unlink_( prev.get() );
				}break;
			default:
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::prevComplete_( Task *_task )
	{
		if( m_state != TASK_STATE_IDLE )
		{
			return false;
		}

#	ifdef _DEBUG
		if( this->hasPrev_( _task ) == false )
		{
			return false;
		}
#	endif

		this->unlink_( _task );

		if( this->onCheckRun() == false )
		{
			return false;
		}

		this->cancelPrev_();

		if( m_state == TASK_STATE_END )
		{
			return false;
		}

		return true;
	}
    //////////////////////////////////////////////////////////////////////////
#   define TASK_EVENTR( Event, R, M )\
    return (m_events & Event) ? R : this-> _##M ()
#   define TASK_EVENT( Event, M )\
    TASK_EVENTR(Event, (void)(0), M)
	//////////////////////////////////////////////////////////////////////////
	bool Task::onInitialize()
	{
        TASK_EVENTR( TASK_EVENT_INITIALIZE, true, onInitialize );
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::onFinalize()
	{
        TASK_EVENT( TASK_EVENT_FINALIZE, onFinalize );
	}
    //////////////////////////////////////////////////////////////////////////
    bool Task::onValidate() const
    {
        TASK_EVENTR( TASK_EVENT_VALIDATE, true, onValidate );
    }
	//////////////////////////////////////////////////////////////////////////
	bool Task::onCheck() const
	{
        TASK_EVENTR( TASK_EVENT_CHECK, true, onCheck );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::onRun()
	{
        TASK_EVENTR( TASK_EVENT_RUN, true, onRun );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::onSkipable() const
	{
        TASK_EVENTR( TASK_EVENT_SKIPABLE, false, onSkipable );
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::onSkipNoSkiped()
	{
        TASK_EVENT( TASK_EVENT_SKIP_NO_SKIPED, onSkipNoSkiped );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::onSkipBlock()
	{
        TASK_EVENTR( TASK_EVENT_SKIP_BLOCK, false, onSkipBlock );
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::onComplete()
	{
        TASK_EVENT( TASK_EVENT_COMPLETE, onComplete );
	}
    //////////////////////////////////////////////////////////////////////////
    bool Task::onFastSkip()
    {
        TASK_EVENTR( TASK_EVENT_FAST_SKIP, false, onFastSkip );
    }
	//////////////////////////////////////////////////////////////////////////
	void Task::onSkip()
	{
        TASK_EVENT( TASK_EVENT_SKIP, onSkip );
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::onCancel()
	{
        TASK_EVENT( TASK_EVENT_CANCEL, onCancel );
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::onFinally()
	{
        TASK_EVENT( TASK_EVENT_FINALLY, onFinally );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::onCheckRun() const
	{
        TASK_EVENTR( TASK_EVENT_CHECK_RUN, m_prevs.empty(), onCheckRun );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Task::onCheckSkip() const
	{
        TASK_EVENTR( TASK_EVENT_CHECK_SKIP, m_prevs.empty(), onCheckSkip );
	}
    //////////////////////////////////////////////////////////////////////////
#   undef TASK_EVENTR
#   undef TASK_EVENT
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
    bool Task::_onCheck() const
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onRun()
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onSkipable() const
    {
        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::_onSkipNoSkiped()
    {
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
    bool Task::_onCheckRun() const
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Task::_onCheckSkip() const
    {
        return true;
    }
//////////////////////////////////////////////////////////////////////////
	void Task::addPrev_( Task * _task )
	{
		m_prevs.push_back( _task );
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::removePrev_( Task * _task )
	{
		TVectorTasks::iterator it_erase = std::find( m_prevs.begin(), m_prevs.end(), _task );

		m_prevs.erase( it_erase );
	}
    //////////////////////////////////////////////////////////////////////////
    void Task::removeNext_( Task * _task )
    {
        TVectorTasks::iterator it_erase = std::find( m_nexts.begin(), m_nexts.end(), _task );

        if( it_erase == m_nexts.end() )
        {
            return;
        }

        m_nexts.erase( it_erase );
    }
    //////////////////////////////////////////////////////////////////////////
    void Task::unlink_( Task * _task )
    {
        this->removePrev_( _task );
        //_task->removeNext_( this );
    }
	//////////////////////////////////////////////////////////////////////////
	bool Task::hasPrev_( const Task * _task ) const
	{
		TVectorTasks::const_iterator it_found = std::find( m_prevs.begin(), m_prevs.end(), _task );

		return it_found != m_prevs.end();
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::finalize_()
	{
		this->setState( TASK_STATE_END );

		this->onFinalize();

		m_chain = nullptr;

		m_nexts.clear();
		m_prevs.clear();
	}
	//////////////////////////////////////////////////////////////////////////
	void Task::setState( ETaskState _state )
	{
		m_state = _state;
	}
}