#ifndef tadH
#define tadH

#include <string>

typedef std::string AnsiString;
typedef System::IO::FileStream TFileStream;

typedef unsigned long         uint32;
typedef unsigned short        uint16;
typedef unsigned char         uint8;

typedef unsigned __int64      uint64;
typedef __int64               int64;
typedef signed long           int32;
typedef signed short          int16;
typedef signed char           int8;

typedef unsigned char		   BYTE;
typedef unsigned short 		   WORD;
typedef unsigned long		   LWORD;

#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
   typedef unsigned char  BYTE;	//  8 bits
#endif // !_BYTE_DEFINED

#ifndef _WORD_DEFINED
#define _WORD_DEFINED
   typedef unsigned short WORD;	// 16 bits
#endif // !_WORD_DEFINED

#ifndef _LWORD_DEFINED
#define _LWORD_DEFINED
   typedef unsigned long  LWORD;	// 32 bits
#endif

//typedef unsigned int u32;
//typedef unsigned short u16;
//typedef unsigned char u8;

#ifndef delay
    #ifdef __WIN32__
         #include <winsock2.h>
        #include <windows.h>
        #define delay Sleep
    #endif
#endif

#endif
