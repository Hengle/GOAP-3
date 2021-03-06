/*
* Copyright (C) 2017-2019, Yuriy Levchenko <irov13@mail.ru>
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#pragma once

#include "GOAP/Mixin.h"

namespace GOAP
{
    class Allocator;

    class Factorable
        : public Mixin
    {
    public:
        Factorable();
        virtual ~Factorable();

    public:
        void setAllocator( Allocator * _allocator );
        Allocator * getAllocator() const;

    public:
        uint32_t incref() override;
        void decref() override;
        uint32_t getrefcount() const override;

    protected:
        Allocator * m_allocator;

        uint32_t m_reference;
    };
}