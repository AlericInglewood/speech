// speech -- Speech recognition
//
//! @file debug_ostream_operators.cc
//! @brief This file contains the definition of debug serializers.

#include "sys.h"        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.

#ifdef CWDEBUG

#include <iostream>
#include "debug.h"
#include <libcwd/buf2str.h>
#include <libcwd/cwprint.h>

//! For debugging purposes. Write a timeval to \a os.
std::ostream& operator<<(std::ostream& os, timeval const& time)
{
  return os << "{tv_sec:" << time.tv_sec << ", tv_usec:" << time.tv_usec << '}';
}

#endif // CWDEBUG
