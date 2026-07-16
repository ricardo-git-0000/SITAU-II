#ifndef string_defineH
#define string_defineH

#include <ExtCtrls.hpp>

#ifndef _DELPHI_STRING_UNICODE 
    #define UnicodeString AnsiString
    #define _Lit(str) str
#else
    #define _Lit(str) L##str
#endif

#endif