#ifndef pcie_tadH
#define pcie_tadH

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

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#ifndef delay
    #ifdef __WIN32__
         #include <winsock2.h>
        #include <windows.h>
        #define delay Sleep
    #endif
#endif

#define CEILING(x,y) (((x) + (y) - 1) / (y))
#define N_CHANNELS_MODULE	32
#define N_MODULES			4
#define N_BYTES_BEAT		32
#define N_BYTES_SAMPLES		2

typedef union
{
	unsigned long word6_32;
	struct
	{
		unsigned long channel_id : 8;
		unsigned long reserverd : 22;
		unsigned long timestamp_present : 1;
		unsigned long last_block : 1;
	}bits;
} header_word6_t;

typedef union
{
	unsigned long word6_32;
	struct
	{
		unsigned short image_focal_law_index;
		unsigned short image_focal_law_total; // Si last_block=1 número de leyes focales válidas dentro del bloque
	}bits;
} header_word5_t;

typedef union
{
	unsigned long word6_32;
	struct
	{
		unsigned long block_beats : 16;
		unsigned long reserved : 16;
	}bits;
} header_word2_t;

typedef union
{
	unsigned long word3_32;
	struct
	{
		unsigned long block_focal_laws : 10;
		unsigned long reserved : 22;
	}bits;
} header_word3_t;

typedef union U_block_header
{
	unsigned long header32[8];
	struct
	{
		unsigned long magic_number; //"FUSE"
		unsigned long block_index;//Identificación de bloque, aumenta de uno en uno
		header_word2_t block_beats_word;//Número fijo: Muestras*Ncanales*2byte/muestra/número de bytes por "beat"(32)
		header_word3_t block_focal_laws_word;//Número fijo: Número de leyes focales que hay en cada bloque
		unsigned long image_index;//Número de imagen por la que va este bloque. Puede haber varias iágenes en el mismo bloque, por lo que aumentará en más de uno entre bloques o puede haber varios bloques por imagen por lo que puede no aumentar nada entre bloques consecutivos
		header_word5_t image_focal_law_word;//Número de la primera ley focal que se envía en este bloque
		header_word6_t ch_id_last;//El bit más alto identifica si es el último bloque de la adquisición y por tanto, no se deberían recibir más (sería otra adquisición que podría tener otros parámetros). Los 8 bits más bajos indican el canal virtual (ahora mismo en principio el 0). No debería cambiar entre bloques de la misma adquisición
		unsigned long tail;//De momento reservado
	} header_struct;
} T_block_header;

enum TThreadPriority {
	tpIdle,
	tpLowest,
	tpLower,
	tpNormal,
	tpHigher,
	tpHighest,
	tpTimeCritical
};

#define HEADER_MAGIC_NUMBER 0x45535546

#endif
