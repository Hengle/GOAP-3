/*
* Copyright (C) 2017, Yuriy Levchenko <irov13@mail.ru>
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#	include "GOAP/TaskWhile.h"
#	include "GOAP/Source.h"
#	include "GOAP/Chain.h"
#	include "GOAP/ChainProvider.h"

namespace GOAP
{
    //////////////////////////////////////////////////////////////////////////
    TaskWhile::TaskWhile( const ScopeProviderPtr & _providerScope )
        : m_providerScope( _providerScope )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    TaskWhile::~TaskWhile()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    template<class F>
    static ChainPtr __makeChain( const SourcePtr & _source, F f )
    {
        ChainPtr chain = GOAP_NEW Chain( _source );

        chain->setCallback( f );

        return chain;
    }
    //////////////////////////////////////////////////////////////////////////
    void TaskWhile::_onFinalize()
    {
        m_providerScope = nullptr;

        if( m_chainWhile != nullptr )
        {
            ChainPtr chain = m_chainWhile;
            m_chainWhile = nullptr;
            chain->cancel();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool TaskWhile::_onRun()
    {
        GOAP::SourcePtr sourceWhile = GOAP_NEW GOAP::Source();

        bool skip = this->isSkip();
        sourceWhile->setSkip( skip );

        if( m_providerScope->onScope( sourceWhile ) == false )
        {
            return true;
        }

        ChainPtr chainWhile = __makeChain( sourceWhile, [this]( bool _skip ){this->whileComplete_( _skip ); } );

        m_chainWhile = chainWhile;

        if( m_chainWhile->run() == false )
        {
            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void TaskWhile::whileComplete_( bool _skip )
    {
        if( m_providerScope == nullptr )
        {
            return;
        }

        GOAP::SourcePtr sourceWhile = GOAP_NEW GOAP::Source();

        sourceWhile->setSkip( _skip );

        if( m_providerScope->onScope( sourceWhile ) == false )
        {
            this->complete( true, _skip );

            return;
        }

        ChainPtr chainWhile = __makeChain( sourceWhile, [this]( bool _skip ){this->whileComplete_( _skip ); } );

        m_chainWhile = chainWhile;

        if( m_chainWhile->run() == false )
        {
            this->complete( true, _skip );

            return;
        }
    }
}
