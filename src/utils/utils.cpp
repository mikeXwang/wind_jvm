#include "utils/utils.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <locale>
#include "runtime/oop.hpp"
#include "wind_jvm.hpp"
#include "native/java_lang_string.hpp"

// from Stack Overflow
std::string wstring_to_utf8 (const std::wstring& str)			// 这东西有时候不准。像是我的 JVM_GetBooleanAttributes0 这个函数，wstring 文件路径转为 char* 经常变成 "x"。。。都不知道为啥....
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

// from Stack Overflow
std::wstring utf8_to_wstring (const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// from Stack Overflow
std::string narrow( const std::wstring& str )
{
	std::ostringstream stm ;
	const std::ctype<char>& ctfacet = std::use_facet< std::ctype<char> >( stm.getloc() ) ;
	for( size_t i=0 ; i<str.size() ; ++i )
		stm << ctfacet.narrow( str[i], 0 ) ;
	return stm.str() ;
}

// toString fast execute
wstring toString(InstanceOop *oop, vm_thread *thread)		// for debugging
{
	auto real_klass = std::static_pointer_cast<InstanceKlass>(oop->get_klass());
	auto toString = real_klass->search_vtable(L"toString:()Ljava/lang/String;");	// don't use `find_in_this_klass()..."
	assert(toString != nullptr);
	InstanceOop *str = (InstanceOop *)thread->add_frame_and_execute(toString, {oop});
	return java_lang_string::stringOop_to_wstring(str);
}


int parse_field_descriptor(const std::wstring & descriptor)
{
	if(descriptor.size() == 1) {
		switch(descriptor[0]) {
			case L'B':	// byte
			case L'C':	// char
			case L'F':	// float
			case L'I':	// int
			case L'S':	// short
			case L'Z':	// boolean
//				return 4;					// 可耻地全部设成 8 字节了......
			case L'D':	// double
			case L'J':	// long
				return 8;
			default:
				std::wcerr << "can't get here in <utils.cpp>::parse_field_descriptor!" << std::endl;
				assert(false);
				return -1;
		}
	} else {
		return 8;	// [I or Ljava.lang.String.  Array && Reference is pointer in x64.
	}
}
