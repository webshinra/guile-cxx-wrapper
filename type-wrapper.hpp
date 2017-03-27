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

#pragma once

#include <type_traits>

#include <libguile.h>
#include <guile/gh.h>

namespace guile
{

  template<bool Protect = false>
  class SCM_convertible {};

  constexpr bool GC_Protected = true;
  constexpr bool GC_NotProtected = false; 
  
  template <>
  struct SCM_convertible<false>
  {
    SCM_convertible (SCM data):
      data_field(data)
    { }

    /// implicite conversion to SCM.
    operator SCM()
    { return data_field; }

    SCM data_field;

    void* operator new(size_t) = delete;          
    void* operator new(size_t, void*) = delete;   
    void* operator new[](size_t) = delete;        
    void* operator new[](size_t, void*) = delete; 
  };

  template <>
  struct SCM_convertible<true>
  {
    SCM_convertible (SCM data):
      data_field(scm_gc_protect_object(data_field))
    { }

    ~SCM_convertible()
    { scm_gc_unprotect_object(data_field); } 

    
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
  template<class WrappedType, bool GC_Protected = false, class Enable = void>
  class wrap: public SCM_convertible<GC_Protected> {}; // primary template
 
  template <class WrappedType, bool GC_Protected>
  struct wrap<WrappedType, GC_Protected,
              typename std::enable_if<std::is_pointer<WrappedType>::value>::type>:
    public SCM_convertible<GC_Protected>
  {
    using Inherited = SCM_convertible<GC_Protected>;
    
    /// 64 bits pointers are converted to scheme integer via
    /// uint64_t. Please keep in mind it induce that no meaningfull
    /// pointer-validity check can be done.
    wrap(WrappedType ptr):
      SCM_convertible<GC_Protected>(scm_from_uint64(reinterpret_cast<uint64_t> (ptr)))
    { }
    
    WrappedType
    operator->()
    { return reinterpret_cast<WrappedType> (scm_to_uint64(Inherited::data_field)); }

    WrappedType
    check()
    {
      if(!scm_is_integer(Inherited::data_field))
        throw std::bad_cast {};
      
      return reinterpret_cast<WrappedType> (scm_to_uint64(Inherited::data_field)); 
    } 
    
    template <int Arg>
    void
    check_as_arg(char const * scheme_proc) const
    {
      SCM_ASSERT (scm_is_integer(Inherited::data_field),
                  Inherited::data_field, 
                  Arg, 
                  scheme_proc);
    }
  };

  template <class WrappedType, bool GC_Protected>
  struct wrap<WrappedType, GC_Protected,
              typename std::enable_if<std::is_same<WrappedType,
                                                   std::string>::value>::type>:
    public SCM_convertible<GC_Protected>
  {
    using Inherited = SCM_convertible<GC_Protected>;
    
    wrap(std::string str):
      SCM_convertible<GC_Protected>(scm_from_locale_string(str.c_str()))
    { }

    wrap(SCM scm_str):
      SCM_convertible<GC_Protected>(scm_str)
    { }

    WrappedType
    check()
    {
      if(!scm_is_string(Inherited::data_field))
        throw std::bad_cast {};
      
      return std::string(scm_to_locale_string(Inherited::data_field));
    } 
    
    template <int Arg>
        void
        check_as_arg(char const * scheme_proc) const
      {
        SCM_ASSERT (scm_is_string(Inherited::data_field),
                    Inherited::data_field, 
                    Arg, 
                    scheme_proc);
      }
  };
  
  template <class WrappedType, bool GC_Protected>
  struct wrap<WrappedType, GC_Protected,
              typename std::enable_if<std::is_same<WrappedType,
                                                   int>::value>::type>:
    public SCM_convertible<GC_Protected>
  {
    using Inherited = SCM_convertible<GC_Protected>;
    
    wrap(WrappedType i):
      SCM_convertible<GC_Protected>(scm_from_int(i))
    { }
    
    WrappedType
    check()
    {
      if(!scm_is_integer(Inherited::data_field))
        throw std::bad_cast {};
      
      return scm_to_int(Inherited::data_field);
    }
    
    template <int Arg>
    void
    check_as_arg(char const * scheme_proc) const
    {
      SCM_ASSERT (scm_is_integer(Inherited::data_field),
                  Inherited::data_field, 
                  Arg, 
                  scheme_proc);
    }
  };
}
