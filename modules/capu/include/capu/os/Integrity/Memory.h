/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CAPU_INTEGRITY_MEMORY_H_
#define CAPU_INTEGRITY_MEMORY_H_

#include <capu/os/Generic/Memory.h>
#include <INTEGRITY.h>

namespace capu
{
    namespace os
    {
        class Memory: private capu::generic::Memory
        {
        public:
            using capu::generic::Memory::Set;
            using capu::generic::Memory::Move;
            using capu::generic::Memory::MoveObject;
            using capu::generic::Memory::Compare;
            using capu::generic::Memory::CopyObject;
            using capu::generic::Memory::Copy;
            static uint_t CurrentMemoryUsage();
        };

        inline uint_t Memory::CurrentMemoryUsage()
        {
            // no support for integrity right now
            return 0u;
        }
    }
}


#endif // CAPU_INTEGRITY_MEMORY_H_
