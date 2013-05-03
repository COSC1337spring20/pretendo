
#include <OS.h>
#include <iostream>
#include <stdexcept>
#include <string.h>

#include "SimpleMutex.h"


SimpleMutex::SimpleMutex (char const *debugname)
{
	fMutex = create_sem(1, debugname);
	if(!fMutex) {
		throw std::runtime_error("SimpleMutex::SimpleMutex() Failed");	
	}
}


SimpleMutex::~SimpleMutex()
{
	delete_sem (fMutex);
}


status_t
SimpleMutex::Lock (void)
{
	return acquire_sem(fMutex);
}


status_t
SimpleMutex::Unlock (void)
{
	return release_sem(fMutex);
}
