// speech -- Speech recognition
//
//! @file debug_ostream_operators.h
//! @brief This file contains the declaration of debug serializers.

#ifndef DEBUG_OSTREAM_OPERATORS_H
#define DEBUG_OSTREAM_OPERATORS_H

#ifdef CWDEBUG

#ifndef USE_PCH
#include <sys/time.h>
#include <iosfwd>                       // std::ostream&
#include <utility>                      // std::pair
#include <map>
#include <boost/shared_ptr.hpp>         // boost::shared_ptr
#include <boost/weak_ptr.hpp>           // boost::weak_ptr
#endif

struct timeval;

extern std::ostream& operator<<(std::ostream& os, timeval const& time);                         //!< Print debug info for timeval instance \a time.

#ifdef USE_LIBBOOST
//! Print debug info for boost::shared_ptr&lt;T&gt;.
template<typename T>
std::ostream& operator<<(std::ostream& os, boost::shared_ptr<T> const& data)
{
  os << "(boost::shared_ptr<" << libcwd::type_info_of<T>().demangled_name() << ">)({";
  if (data.get())
    os << *data;
  else
    os << "<NULL>";
  return os << "})";
}

//! Print debug info for boost::weak_ptr&lt;T&gt;.
template<typename T>
std::ostream& operator<<(std::ostream& os, boost::weak_ptr<T> const& data)
{
  return os << "(boost::weak_ptr<" << libcwd::type_info_of<T>().demangled_name() << ">)({ " << *boost::shared_ptr<T>(data) << "})";
}
#endif // USE_LIBBOOST

//! Print debug info for std::pair&lt;&gt; instance \a data.
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, std::pair<T1, T2> const& data)
{
  return os << "{first:" << data.first << ", second:" << data.second << '}';
}

//! Print a whole map.
template<typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, std::map<T1, T2, T3> const& data)
{
#if CWDEBUG_LOCATION
  os << "{map<" << libcwd::type_info_of<T1>().demangled_name() <<
      ", " << libcwd::type_info_of<T2>().demangled_name() <<
      ", " << libcwd::type_info_of<T3>().demangled_name() <<">:";
#else
  os << "{map<>:";
#endif
  typedef std::map<T1, T2, T3> map_type;
  for (typename map_type::const_iterator iter = data.begin(); iter != data.end(); ++iter)
    os << '{' << *iter << '}';
  return os << '}';
}

#endif // CWDEBUG
#endif // DEBUG_OSTREAM_OPERATORS_H
