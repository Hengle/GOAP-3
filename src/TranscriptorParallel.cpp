/*
* Copyright (C) 2017, Levchenko Yuriy <irov13@mail.ru>
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#	include "TranscriptorParallel.h"

#	include "TaskParallelNeck.h"

#	include "GOAP/Source.h"

#	include "GOAP/Task.h"

namespace GOAP
{
    //////////////////////////////////////////////////////////////////////////
    TranscriptorParallel::TranscriptorParallel( size_t _count )
    {
        m_sources.resize( _count );

        for( VectorSources::iterator
            it = m_sources.begin(),
            it_end = m_sources.end();
            it != it_end;
            ++it )
        {
            *it = GOAP_NEW Source();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    const VectorSources & TranscriptorParallel::getSources() const
    {
        return m_sources;
    }
    //////////////////////////////////////////////////////////////////////////
    TaskPtr TranscriptorParallel::generate( const ChainPtr & _chain, const TaskPtr & _task )
    {
        if( m_sources.empty() == true )
        {
            return _task;
        }

        TaskPtr task_parallel_neck = GOAP_NEW TaskParallelNeck();
        task_parallel_neck->setChain( _chain );

        for( VectorSources::iterator
            it = m_sources.begin(),
            it_end = m_sources.end();
            it != it_end;
            ++it )
        {
            const SourcePtr & parallel_source = *it;

            TaskPtr task = parallel_source->parse( _chain, _task );

            task->addNext( task_parallel_neck );
        }

        return task_parallel_neck;
    }
}