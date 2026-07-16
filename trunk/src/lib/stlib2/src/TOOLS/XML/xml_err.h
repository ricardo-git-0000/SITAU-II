#ifndef xml_errH
#define xml_errH

#define xml_offset -0

// OK, resultado de la operacion correcto, no se ha producido ningun error.
#define EXML_NONE                  0

#define EXML_FILE                 (xml_offset - 1)
#define charEXML_FILE             "Error de archivo, acceso al archivo incorrecto"

#define EXML_Parameter                (xml_offset - 2)
#define charEXML_Parameter            "Error de rango, la variable pasada como parámetro está fuera de rango"

#define EXML_NoMemory                (xml_offset - 3)
#define charEXML_NoMemory            "Error en la reerva de memoria"

#endif

