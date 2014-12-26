// speech -- Speech recognition
//
//! @file debug_ostream_operators.h
//! @brief This file contains the declaration of debug serializers.

#ifndef DEBUG_OSTREAM_OPERATORS_H
#define DEBUG_OSTREAM_OPERATORS_H

#ifdef CWDEBUG

#include <sys/time.h>
#include <iosfwd>                       // std::ostream&
#include <utility>                      // std::pair
#include <map>
#include <boost/shared_ptr.hpp>         // boost::shared_ptr
#include <boost/weak_ptr.hpp>           // boost::weak_ptr
#if CWDEBUG_LOCATION
#include <libcwd/type_info.h>
#else
#include <typeinfo>
#endif


namespace debug {

template<typename T>
inline char const* type_name_of(void)
{
#if CWDEBUG_LOCATION
  return libcwd::type_info_of<T>().demangled_name();
#else
  return typeid(T).name();
#endif
}

} // namespace debug

namespace AIAlert {
  class Error;
} // namespace AIAlert

extern std::ostream& operator<<(std::ostream& os, AIAlert::Error const& error);

struct timeval;

extern std::ostream& operator<<(std::ostream& os, timeval const& time);                         //!< Print debug info for timeval instance \a time.

#ifdef USE_LIBBOOST
//! Print debug info for boost::shared_ptr&lt;T&gt;.
template<typename T>
std::ostream& operator<<(std::ostream& os, boost::shared_ptr<T> const& data)
{
  os << "(boost::shared_ptr<" << debug::type_name_of<T>() << ">)({";
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
  return os << "(boost::weak_ptr<" << debug::type_name_of<T>() << ">)({ " << *boost::shared_ptr<T>(data) << "})";
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
  os << "{map<" << debug::type_name_of<T1>() <<
      ", " << debug::type_name_of<T2>() <<
      ", " << debug::type_name_of<T3>() <<">:";
  typedef std::map<T1, T2, T3> map_type;
  for (typename map_type::const_iterator iter = data.begin(); iter != data.end(); ++iter)
    os << '{' << *iter << '}';
  return os << '}';
}

#endif // CWDEBUG
#endif // DEBUG_OSTREAM_OPERATORS_H
