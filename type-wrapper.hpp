/*
  This file is part of A minimalist Guile c++ wrapper.

  Copyright (C) 2017 yann asset shinra@electric-dragons.org,
  Electric-Dragons.
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License and the GUN General Public License along with this
  program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <type_traits>

#include <libguile.h>
#include <guile/gh.h>

namespace guile
{
  struct SCM_convertible
  {
    SCM_convertible (SCM data):
      data_field(data)
    { }

    /// implicite conversion to SCM.
    operator SCM()
    { return data_field; }

    SCM data_field;
  }; 

  /** @brief The guile::wrap class, handle a important subset of guile
      SCM subjacent types.

      The idea here is to keep type safety active as much as possible
      on the c++ side. You can never be sure about what scheme give
      you (especialy when passing pointer type) but having
      semi-automatic transtyping could help a lot.
  */
  template<class WrappedType, class Enable = void>
  class wrap {}; // primary template
 
  template <class WrappedType>
  struct wrap<WrappedType,
              typename std::enable_if<std::is_pointer<WrappedType>::value>::type>:
    public SCM_convertible
  {
    /// 64 bits pointers are converted to scheme integer via uint64_t.
    wrap(WrappedType ptr):
      SCM_convertible(scm_from_uint64(reinterpret_cast<uint64_t> (ptr)))
    { }
    
    WrappedType
    operator->()
    { return reinterpret_cast<WrappedType> (scm_to_uint64(data_field)); }

    
    template <int Arg>
    void
    check_as_arg(char const * scheme_proc) const
    {
      SCM_ASSERT (scm_is_integer(data_field),
                  data_field, 
                  Arg, 
                  scheme_proc);
    }
  };

  template <class WrappedType>
  struct wrap<WrappedType,
              typename std::enable_if<std::is_same<WrappedType,
                                                  std::string>::value>::type>:
    public SCM_convertible
  {
    wrap(std::string str):
      SCM_convertible(scm_from_locale_string(str.c_str()))
    { }

    wrap(SCM scm_str):
      SCM_convertible(scm_str)
    { }

    template <int Arg>
    void
    check_as_arg(char const * scheme_proc) const
    {
      SCM_ASSERT (scm_is_string(data_field),
                  data_field, 
                  Arg, 
                  scheme_proc);
    } 
  }; 

  
  template <class WrappedType>
  struct wrap<WrappedType,
              typename std::enable_if<std::is_same<WrappedType,
                                                   int>::value>::type>:
    public SCM_convertible
  {
    wrap(int i):
      SCM_convertible(scm_from_int(i))
    { }

    template <int Arg>
    void
    check_as_arg(char const * scheme_proc) const
    {
      SCM_ASSERT (scm_is_integer(data_field),
                  data_field, 
                  Arg, 
                  scheme_proc);
    }
  };
}
