// Locale support (codecvt) -*- C++ -*-

// Copyright (C) 2000 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

//
// ISO C++ 14882: 22.2.1.5 Template class codecvt
//

// Warning: this file is not meant for user inclusion.  Use <locale>.

// Written by Benjamin Kosnik <bkoz@cygnus.com>

#ifndef _CPP_BITS_CODECVT_H
#define _CPP_BITS_CODECVT_H	1

#ifdef _GLIBCPP_USE_WCHAR_T
#include <iconv.h>		// For iconv, iconv_t
#include <langinfo.h>
#endif

namespace std
{
  // XXX __enc_traits may need to move up the locale header heirarchy,
  // depending on if ctype ends up using it.
#ifdef _GLIBCPP_USE_WCHAR_T
  // Extensions to use icov for dealing with character encodings,
  // including conversions and comparisons between various character
  // sets.  This object encapsulates data that may need to be shared between
  // char_traits, codecvt and ctype.
  class __enc_traits
  {
  public:
    // Types:
    typedef iconv_t	__conv_type;
    typedef mbstate_t	__state_type;
    
  protected:
    // Data Members:
    // Max size of charset encoding name
    static const int 	__max_size = 32;
    // Name of internal character set encoding.
    char	       	__intc_enc[__max_size];
    // Name of external character set encoding.
    char  	       	__extc_enc[__max_size];

    // Conversion descriptor between external encoding to internal encoding.
    __conv_type		__in_desc;
    // Conversion descriptor between internal encoding to external encoding.
    __conv_type		__out_desc;

  public:
    __enc_traits() : __in_desc(0), __out_desc(0)
    {
      // __intc_end = whatever we are using internally, which is
      // UCS4 (linux) 
      // UCS2 == UNICODE  (microsoft, java, aix, whatever...)
      // XXX Currently don't know how to get this data from target system...
      strcpy(__intc_enc, "UCS4");

      // __extc_end = external codeset in current locale
      strcpy(__extc_enc, nl_langinfo(CODESET));
    }

    __enc_traits(const char* __int, const char* __ext)
    : __in_desc(0), __out_desc(0)
    {
      strcpy(__intc_enc, __int);
      strcpy(__extc_enc, __ext);
    }

    ~__enc_traits()
    {
      iconv_close(__in_desc);
      iconv_close(__out_desc);
    } 

    // Initializes
    void
    _M_init()
    {
      __in_desc = iconv_open(__intc_enc, __extc_enc);
      __out_desc = iconv_open(__extc_enc, __intc_enc);
      if (__out_desc == iconv_t(-1) || __in_desc == iconv_t(-1))
	{
	  // XXX Extended error checking.
	}
    }

    bool
    _M_good()
    { 
      return __out_desc && __in_desc 
	     && __out_desc != iconv_t(-1) && __in_desc != iconv_t(-1);
    }

    const __conv_type* 
    _M_get_in_descriptor()
    { return &__in_desc; }

    const __conv_type* 
    _M_get_out_descriptor()
    { return &__out_desc; }

   const char* 
    _M_get_internal_enc()
    { return __intc_enc; }

    const char* 
    _M_get_external_enc()
    { return __extc_enc; }

  protected:
    // 21.1.2 traits typedefs
    // p4
    // typedef STATE_T state_type
    // requires: state_type shall meet the requirements of
    // CopyConstructible types (20.1.3)
    // XXX because of this, these might actually need to be filled out.
    __enc_traits(const __enc_traits&);
  };
#endif //_GLIBCPP_USE_WCHAR_T


  //  22.2.1.5  Template class codecvt
  class codecvt_base
  {
  public:
    enum result
    {
      ok,
      partial,
      error,
      noconv
    };
  };

  // Template class __codecvt_abstract_base
  // NB: An abstract base class that fills in the public inlines, so
  // that the specializations don't have to re-copy the public
  // interface.
  template<typename _InternT, typename _ExternT, typename _StateT>
    class __codecvt_abstract_base 
    : public locale::facet, public codecvt_base
    {
    public:
      // Types:
      typedef codecvt_base::result			result;
      typedef _InternT 					intern_type;
      typedef _ExternT 					extern_type;
      typedef _StateT  					state_type;
      
      // 22.2.1.5.1 codecvt members
      result
      out(state_type& __state, const intern_type* __from, 
	  const intern_type* __from_end, const intern_type*& __from_next,
	  extern_type* __to, extern_type* __to_end, 
	  extern_type*& __to_next) const
      { 
	return this->do_out(__state, __from, __from_end, __from_next, 
			    __to, __to_end, __to_next); 
      }

      result
      unshift(state_type& __state, extern_type* __to, extern_type* __to_end,
	      extern_type*& __to_next) const
      { return this->do_unshift(__state, __to,__to_end,__to_next); }

      result
      in(state_type& __state, const extern_type* __from, 
	 const extern_type* __from_end, const extern_type*& __from_next,
	 intern_type* __to, intern_type* __to_end, 
	 intern_type*& __to_next) const
      { 
	return this->do_in(__state, __from, __from_end, __from_next,
			   __to, __to_end, __to_next); 
      }

      int 
      encoding() const throw()
      { return this->do_encoding(); }

      bool 
      always_noconv() const throw()
      { return this->do_always_noconv(); }

      int
      length(const state_type& __state, const extern_type* __from,
	     const extern_type* __end, size_t __max) const
      { return this->do_length(__state, __from, __end, __max); }

      int 
      max_length() const throw()
      { return this->do_max_length(); }

    protected:
      explicit 
      __codecvt_abstract_base(size_t __refs = 0) : locale::facet(__refs) { }

      virtual 
      ~__codecvt_abstract_base() { }

      virtual result
      do_out(state_type& __state, const intern_type* __from, 
	     const intern_type* __from_end, const intern_type*& __from_next,
	     extern_type* __to, extern_type* __to_end,
	     extern_type*& __to_next) const = 0;

      virtual result
      do_unshift(state_type& __state, extern_type* __to, 
		 extern_type* __to_end, extern_type*& __to_next) const = 0;
      
      virtual result
      do_in(state_type& __state, const extern_type* __from, 
	    const extern_type* __from_end, const extern_type*& __from_next, 
	    intern_type* __to, intern_type* __to_end, 
	    intern_type*& __to_next) const = 0;
      
      virtual int 
      do_encoding() const throw() = 0;

      virtual bool 
      do_always_noconv() const throw() = 0;

      virtual int 
      do_length(const state_type&, const extern_type* __from, 
		const extern_type* __end, size_t __max) const = 0;

      virtual int 
      do_max_length() const throw() = 0;
    };

  // 22.2.1.5 Template class codecvt
  // NB: Generic, mostly useless implementation.
  template<typename _InternT, typename _ExternT, typename _StateT>
    class codecvt 
    : public __codecvt_abstract_base<_InternT, _ExternT, _StateT>
    {
    public:      
      // Types:
      typedef codecvt_base::result			result;
      typedef _InternT intern_type;
      typedef _ExternT extern_type;
      typedef _StateT  state_type;

      // Data Members:
      static locale::id id;

      explicit 
      codecvt(size_t __refs = 0) 
      : __codecvt_abstract_base<_InternT,_ExternT,_StateT> (__refs) { }

    protected:
      virtual 
      ~codecvt() { }
    };

  template<typename _InternT, typename _ExternT, typename _StateT>
    locale::id codecvt<_InternT, _ExternT, _StateT>::id;

  // partial specialization
  // This specialization takes advantage of iconv to provide code
  // conversions between a large number of character encodings.
  template<typename _InternT, typename _ExternT>
    class codecvt<_InternT, _ExternT, __enc_traits>
    : public __codecvt_abstract_base<_InternT, _ExternT, __enc_traits>
    {
    public:      
      // Types:
      typedef codecvt_base::result			result;
      typedef _InternT 					intern_type;
      typedef _ExternT 					extern_type;
      typedef __enc_traits 				state_type;
      typedef __enc_traits::__conv_type 		__conv_type;
      typedef __enc_traits				__enc_type;

      // Data Members:
      static locale::id 		id;

      explicit 
      codecvt(size_t __refs = 0)
      : __codecvt_abstract_base<intern_type, extern_type, state_type>(__refs)
      { }

      explicit 
      codecvt(__enc_type* __enc, size_t __refs = 0)
      : __codecvt_abstract_base<intern_type, extern_type, state_type>(__refs)
      { }

    protected:
      virtual 
      ~codecvt() { }

      virtual result
      do_out(state_type& __state, const intern_type* __from, 
	     const intern_type* __from_end, const intern_type*& __from_next,
	     extern_type* __to, extern_type* __to_end,
	     extern_type*& __to_next) const;

      virtual result
      do_unshift(state_type& __state, extern_type* __to, 
		 extern_type* __to_end, extern_type*& __to_next) const;

      virtual result
      do_in(state_type& __state, const extern_type* __from, 
	    const extern_type* __from_end, const extern_type*& __from_next,
	    intern_type* __to, intern_type* __to_end, 
	    intern_type*& __to_next) const;

      virtual int 
      do_encoding() const throw();

      virtual bool 
      do_always_noconv() const throw();

      virtual int 
      do_length(const state_type&, const extern_type* __from, 
		const extern_type* __end, size_t __max) const;

      virtual int 
      do_max_length() const throw();
    };

  template<typename _InternT, typename _ExternT>
    locale::id 
    codecvt<_InternT, _ExternT, __enc_traits>::id;

  template<typename _InternT, typename _ExternT>
    codecvt_base::result
    codecvt<_InternT, _ExternT, __enc_traits>::
    do_out(state_type& __state, const intern_type* __from, 
	   const intern_type* __from_end, const intern_type*& __from_next,
	   extern_type* __to, extern_type* __to_end,
	   extern_type*& __to_next) const
    {
      result __ret = error;
      if (__state._M_good())
	{
	  typedef state_type::__conv_type	__conv_type;
	  const __conv_type* __desc = __state._M_get_out_descriptor();
	  const size_t __fmultiple = sizeof(intern_type) / sizeof(char);
	  size_t __flen = __fmultiple * (__from_end - __from);
	  const size_t __tmultiple = sizeof(extern_type) / sizeof(char);
	  size_t __tlen = __tmultiple * (__to_end - __to); 
	  
	  // Argument list for iconv specifies a byte sequence. Thus,
	  // all to/from arrays must be brutally casted to char*.
	  const char* __cfrom = reinterpret_cast<const char*>(__from);
	  char* __cto = reinterpret_cast<char*>(__to);
	  size_t __conv = iconv(*__desc, &__cfrom, &__flen, &__cto, &__tlen); 
	  
	  if (__conv != size_t(-1))
	    {
	      __from_next = reinterpret_cast<const intern_type*>(__cfrom);
	      __to_next = reinterpret_cast<extern_type*>(__cto);
	      __ret = ok;
	    }
	  else 
	    {
	      if (__flen < __from_end - __from)
		{
		  __from_next = reinterpret_cast<const intern_type*>(__cfrom);
		  __to_next = reinterpret_cast<extern_type*>(__cto);
		  __ret = partial;
		}
	      else
		__ret = error;
	    }
	}
      return __ret; 
    }

  template<typename _InternT, typename _ExternT>
    codecvt_base::result
    codecvt<_InternT, _ExternT, __enc_traits>::
    do_unshift(state_type& __state, extern_type* __to, 
	       extern_type* __to_end, extern_type*& __to_next) const
    {
      result __ret = error;
      if (__state._M_good())
	{
	  typedef state_type::__conv_type	__conv_type;
	  const __conv_type* __desc = __state._M_get_in_descriptor();
	  const size_t __tmultiple = sizeof(intern_type) / sizeof(char);
	  size_t __tlen = __tmultiple * (__to_end - __to); 
	  
	  // Argument list for iconv specifies a byte sequence. Thus,
	  // all to/from arrays must be brutally casted to char*.
	  char* __cto = reinterpret_cast<char*>(__to);
	  size_t __conv = iconv(*__desc, NULL, NULL, &__cto, &__tlen); 
	  
	  if (__conv != size_t(-1))
	    {
	      __to_next = reinterpret_cast<extern_type*>(__cto);
	      if (__tlen == __tmultiple * (__to_end - __to))
		__ret = noconv;
	      else if (__tlen == 0)
		__ret = ok;
	      else
		__ret = partial;
	    }
	  else 
	    __ret = error;
	}
      return __ret; 
    }
   
  template<typename _InternT, typename _ExternT>
    codecvt_base::result
    codecvt<_InternT, _ExternT, __enc_traits>::
    do_in(state_type& __state, const extern_type* __from, 
	  const extern_type* __from_end, const extern_type*& __from_next,
	  intern_type* __to, intern_type* __to_end, 
	  intern_type*& __to_next) const
    { 
      result __ret = error;
      if (__state._M_good())
	{
	  typedef state_type::__conv_type	__conv_type;
	  const __conv_type* __desc = __state._M_get_in_descriptor();
	  const size_t __fmultiple = sizeof(extern_type) / sizeof(char);
	  size_t __flen = __fmultiple * (__from_end - __from);
	  const size_t __tmultiple = sizeof(intern_type) / sizeof(char);
	  size_t __tlen = __tmultiple * (__to_end - __to); 
	  
	  // Argument list for iconv specifies a byte sequence. Thus,
	  // all to/from arrays must be brutally casted to char*.
	  const char* __cfrom = reinterpret_cast<const char*>(__from);
	  char* __cto = reinterpret_cast<char*>(__to);
	  size_t __conv = iconv(*__desc, &__cfrom, &__flen, &__cto, &__tlen); 
	  
	  if (__conv != size_t(-1))
	    {
	      __from_next = reinterpret_cast<const extern_type*>(__cfrom);
	      __to_next = reinterpret_cast<intern_type*>(__cto);
	      __ret = ok;
	    }
	  else 
	    {
	      if (__flen < __from_end - __from)
		{
		  __from_next = reinterpret_cast<const extern_type*>(__cfrom);
		  __to_next = reinterpret_cast<intern_type*>(__cto);
		  __ret = partial;
		}
	      else
		__ret = error;
	    }
	}
      return __ret; 
    }
  
  template<typename _InternT, typename _ExternT>
    int 
    codecvt<_InternT, _ExternT, __enc_traits>::
    do_encoding() const throw()
    { return 0; }
  
  template<typename _InternT, typename _ExternT>
    bool 
    codecvt<_InternT, _ExternT, __enc_traits>::
    do_always_noconv() const throw()
    { return false; }
  
  template<typename _InternT, typename _ExternT>
    int 
    codecvt<_InternT, _ExternT, __enc_traits>::
    do_length(const state_type& __state, const extern_type* __from, 
	      const extern_type* __end, size_t __max) const
    { return min(__max, static_cast<size_t>(__end - __from)); }

#ifdef _GLIBCPP_RESOLVE_LIB_DEFECTS
// 74.  Garbled text for codecvt::do_max_length
  template<typename _InternT, typename _ExternT>
    int 
    codecvt<_InternT, _ExternT, __enc_traits>::
    do_max_length() const throw()
    { return 1; }
#endif

  // codecvt<char, char, mbstate_t> required specialization
  template<>
    class codecvt<char, char, mbstate_t> 
    : public __codecvt_abstract_base<char, char, mbstate_t>
    {
    public:      
      // Types:
      typedef char 	intern_type;
      typedef char 	extern_type;
      typedef mbstate_t state_type;

      // Data Members:
      static locale::id id;

      explicit 
      codecvt(size_t __refs = 0);

    protected:
      virtual 
      ~codecvt();

      virtual result
      do_out(state_type& __state, const intern_type* __from, 
	     const intern_type* __from_end, const intern_type*& __from_next,
	     extern_type* __to, extern_type* __to_end,
	     extern_type*& __to_next) const;

      virtual result
      do_unshift(state_type& __state, extern_type* __to, 
		 extern_type* __to_end, extern_type*& __to_next) const;

      virtual result
      do_in(state_type& __state, const extern_type* __from, 
	    const extern_type* __from_end, const extern_type*& __from_next,
	    intern_type* __to, intern_type* __to_end, 
	    intern_type*& __to_next) const;

      virtual int 
      do_encoding() const throw();

      virtual bool 
      do_always_noconv() const throw();

      virtual int 
      do_length(const state_type&, const extern_type* __from, 
		const extern_type* __end, size_t __max) const;

      virtual int 
      do_max_length() const throw();
  };

#ifdef _GLIBCPP_USE_WCHAR_T
  // codecvt<wchar_t, char, mbstate_t> required specialization
  template<>
    class codecvt<wchar_t, char, mbstate_t> 
    : public __codecvt_abstract_base<wchar_t, char, mbstate_t>
    {
    public:
      // Types:
      typedef wchar_t 	intern_type;
      typedef char 	extern_type;
      typedef mbstate_t state_type;

      // Data Members:
      static locale::id id;

      explicit 
      codecvt(size_t __refs = 0);

    protected:
      virtual 
      ~codecvt();

      virtual result
      do_out(state_type& __state, const intern_type* __from, 
	     const intern_type* __from_end, const intern_type*& __from_next,
	     extern_type* __to, extern_type* __to_end,
	     extern_type*& __to_next) const;

      virtual result
      do_unshift(state_type& __state,
		 extern_type* __to, extern_type* __to_end,
		 extern_type*& __to_next) const;

      virtual result
      do_in(state_type& __state,
	     const extern_type* __from, const extern_type* __from_end,
	     const extern_type*& __from_next,
	     intern_type* __to, intern_type* __to_end,
	     intern_type*& __to_next) const;

      virtual 
      int do_encoding() const throw();

      virtual 
      bool do_always_noconv() const throw();

      virtual 
      int do_length(const state_type&, const extern_type* __from,
		    const extern_type* __end, size_t __max) const;

      virtual int 
      do_max_length() const throw();
    };
#endif //_GLIBCPP_USE_WCHAR_T

  // 22.2.1.6  Template class codecvt_byname
  template<typename _InternT, typename _ExternT, typename _StateT>
    class codecvt_byname : public codecvt<_InternT, _ExternT, _StateT>
    {
    public:
      explicit 
      codecvt_byname(const char*, size_t __refs = 0) 
      : codecvt<_InternT,_ExternT,_StateT> (__refs) { }
    protected:
      virtual 
      ~codecvt_byname() { }
    };

  template<>
    class codecvt_byname<char, char, mbstate_t>
    : public codecvt<char, char, mbstate_t>
    {
    public:
      explicit 
      codecvt_byname(const char*, size_t __refs = 0);

    protected:
      virtual 
      ~codecvt_byname();
    };
  
#ifdef _GLIBCPP_USE_WCHAR_T
  template<>
    class codecvt_byname<wchar_t, char, mbstate_t>
      : public codecvt<wchar_t, char, mbstate_t>
    {
    public:
      explicit 
      codecvt_byname(const char*, size_t __refs = 0);

    protected:
      virtual 
      ~codecvt_byname();
    };
#endif

} // namespace std

#endif // _CPP_BITS_CODECVT_H

// Local Variables:
// mode:c++
// End:

