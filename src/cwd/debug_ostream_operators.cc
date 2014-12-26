// speech -- Speech recognition
//
//! @file debug_ostream_operators.cc
//! @brief This file contains the definition of debug serializers.

#include "sys.h"        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.

#ifdef CWDEBUG

#include "utils/AIAlert.h"

#include <iostream>
#include "debug.h"
#include <libcwd/buf2str.h>
#include <libcwd/cwprint.h>

//! For debugging purposes. Write a AIAlert::Error to \a os.
std::ostream& operator<<(std::ostream& os, AIAlert::Error const& error)
{
  os << "AIAlert: ";
  int lines = error.lines().size();
  int count = 0;
  for (AIAlert::Error::lines_type::const_iterator line = error.lines().begin(); line != error.lines().end(); ++line)
  {
    if (++count < lines && lines > 1) os << "\n    ";
    os << translate::getString(line->getXmlDesc(), line->args());
  }
  return os;
}

//! For debugging purposes. Write a timeval to \a os.
std::ostream& operator<<(std::ostream& os, timeval const& time)
{
  return os << "{tv_sec:" << time.tv_sec << ", tv_usec:" << time.tv_usec << '}';
}

#endif // CWDEBUG
