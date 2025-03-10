#pragma once

#include <cstdint>

namespace DAVA
{
/*! brief Semaphore wrapper class compatible with Thread class. Supports Win32, MacOS, iPhone, Android platforms. */
class Semaphore
{
public:
    /**
		The initial count for the semaphore object(0). This value must be greater than or equal to zero.
	*/
    Semaphore(uint32_t count = 0U);
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    ~Semaphore();

    /**
		Increases the count of the specified semaphore object by a specified amount. (default 1)
	*/
    void Post(uint32_t count = 1);
    /**
		Wait till semaphore becomes more then 0 and decrement it with 1.
	*/
    void Wait();

protected:
    //!< platform dependent semaphore handle
    uintptr_t semaphore = 0;
};

} // end namespace DAVA
