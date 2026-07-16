#ifndef fl_errH
#define fl_errH

#define fl_error_code_start  -1000

#define EFL_NONE               	   0

#define EFL_PARAMETER        	      (fl_error_code_start  - 1)
#define charEFL_PARAMETER    	      "Error: Parameter value error"

#define EFL_FILE        	 	      (fl_error_code_start  - 2)
#define charEFL_FILE		     	      "Error: File access error"

#define EFL_PA_Z					      (fl_error_code_start  - 3)
#define charEFL_PA_Z    		      "ERROR: Invalid PA transducer Z position"

#define EFL_PA_X					      (fl_error_code_start  - 4)
#define charEFL_PA_X    		      "ERROR: Invalid PA transducer X position"

#define EFL_PA_NULL				      (fl_error_code_start  - 5)
#define charEFL_PA_NULL    	      "ERROR: The 'PA' transducer has not been created"

#define EFL_WEDGE_NULL			      (fl_error_code_start  - 6)
#define charEFL_WEDGE_NULL          "ERROR: The 'Wedge' has not been created"

#define EFL_SPECIMEN_NULL		      (fl_error_code_start  - 7)
#define charEFL_SPECIMEN_NULL       "ERROR: The 'Specimen' has not been created"

#define EFL_SCAN_NULL			      (fl_error_code_start  - 8)
#define charEFL_SCAN_NULL           "ERROR: The 'Scan' has not been created"

#define EFL_DDF_APERTURE_NULL			(fl_error_code_start  - 9)
#define charEFL_DDF_APERTURE_NULL   "ERROR: The 'Reception Aperture' has not been created"

#define EFL_NO_MEM			         (fl_error_code_start  - 10)
#define charEFL_NO_MEM              "ERROR: Not enough dinamic memory space"

#define EFL_NO_SCAN			         (fl_error_code_start  - 11)
#define charEFL_NO_SCAN             "ERROR: Scan not found in the list of scan assigned to the component"

#define EFL_ANGLE_OVF			      (fl_error_code_start  - 12)
#define charEFL_ANGLE_OVF           "ERROR: Incident angle is greater than or equal 90 degrees"

#define EFL_XYZ_SIZE			         (fl_error_code_start  - 13)
#define charEFL_XYZ_SIZE            "ERROR: X, Y and Z size coordinates is different"

#define EFL_IANGLE_CRITICAL			(fl_error_code_start  - 14)
#define charEFL_IANGLE_CRITICAL     "ERROR: The incidence angle is greater than 90º"

#define EFL_APERTURE_SIZE			   (fl_error_code_start  - 15)
#define charEFL_APERTURE_SIZE       "ERROR Size aperture"

#define EFL_FOCAL_LAW_SIZE			   (fl_error_code_start  - 16)
#define charEFL_FOCAL_LAW_SIZE      "ERROR emission an reception focal law number are different"

#define EFL_PA_COORD_SIZE			   (fl_error_code_start  - 17)
#define charEFL_PA_COORD_SIZE       "ERROR PA coordinates size and element number are different"

#define EFL_NO_WEDGE_SURF			   (fl_error_code_start  - 18)
#define charEFL_NO_WEDGE_SURF       "ERROR Calculating surface limits without wedge"

#define EFL_SURFACE			         (fl_error_code_start  - 19)
#define charEFL_SURFACE             "ERROR Surface type"

#define EFL_VIRTUAL_ARRAY_SIZE		(fl_error_code_start  - 20)
#define charEFL_VIRTUAL_ARRAY_SIZE  "ERROR virtual aperture size and real aperture size are differenc"

#define EFL_DDF_NFOCUS			      (fl_error_code_start  - 21)
#define charEFL_DDF_NFOCUS          "ERROR en el número de focos"

#define EFL_X_AP_SIZE			      (fl_error_code_start  - 22)
#define charEFL_X_AP_SIZE           "ERROR: The aperture size is null or negative distance"

#define EFL_PQ			               (fl_error_code_start  - 23)
#define charEFL_PQ                  "DDF ERROR: Error en la codificación de los parámetros P y Q"

#define EFL_ACQ_RANGE			      (fl_error_code_start  - 24)
#define charEFL_ACQ_RANGE           "DDF ERROR: Error en la rango de la adquisición"

#define EFL_SQRT_NEG			      (fl_error_code_start  - 25)
#define charEFL_SQRT_NEG           "ERROR: Raiz de un número negativo."

#define EFL_PtrAccess_NULL			      (fl_error_code_start  - 26)
#define charEFL_PtrAccess_NULL           "ERROR: Acceso a puntero NULL."

#define EFL_PAElement			      (fl_error_code_start  - 27)
#define charEFL_PAElement           "ERROR: The element doesn't exist."

#define EFL_IFAZ_ERR        (fl_error_code_start  - 28)
#define charEFL_IFAZ_ERR    "ERROR: Interface commponent not exists"

#define EFL_DataInconsistency      (fl_error_code_start  - 29)
#define charEFL_DataInconsistency 	"Error, inconsistencia entre los datos"

#define EFL_ASIN      (fl_error_code_start  - 30)
#define charEFL_ASIN 	"Error, Cálculo de un arco cuyo seno es mayor que 1."

#define EFL_SurfacePoints_Izqd      (fl_error_code_start  - 31)
#define charEFL_SurfacePoints_Izqd  	"Error, El número de puntos de la superficie es insuficiente por la izquierda"

#define EFL_SurfacePoints_Drch      (fl_error_code_start  - 32)
#define charEFL_SurfacePoints_Drch  	"Error, El número de puntos de la superficie es insuficiente por la derecha"

#define EFL_SAD_NoGate      (fl_error_code_start  - 33)
#define charEFL_SAD_NoGate 	"Surface Auto Detect Error, Error en la definición en la puerta de busqueda."

#define EFL_NoSurface      (fl_error_code_start  - 34)
#define charEFL_NoSurface 	"No se ha deficido la superficie."

#define EFL_SCAN_LINES_NUMBER          (fl_error_code_start  - 35)
#define charEFL_SCAN_LINES_NUMBER 	   "Error, el número de lineas total de la adquisición excede el número máximo"


#endif

