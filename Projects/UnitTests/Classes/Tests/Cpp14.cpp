/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "DAVAEngine.h"

#include "UnitTests/UnitTests.h"

#include <memory>

// disable for now
#if __cplusplus >= 201500L

using namespace DAVA;

auto f() -> int;

auto f() -> int
{
    return 42;
}

DAVA_TESTCLASS(Cpp14Test)
{
    DAVA_TEST(CompileTest)
    {
        TEST_VERIFY(f() == 42);
    }

    DAVA_TEST(ScopeExit)
    {
        int32 i = 0;
        {
            SCOPE_EXIT{++i;};
            TEST_VERIFY(0 == i);
        }
        TEST_VERIFY(1 == i);
    }

    DAVA_TEST(MakeUnique)
    {
    	std::unique_ptr<int> ptr = std::make_unique<int>();
    	*ptr = 10;
    	TEST_VERIFY(10 == *(ptr.get()));

    	int* raw_ptr = ptr.release();
    	TEST_VERIFY(10 == *raw_ptr);
    	delete raw_ptr;
    }
};

#endif
