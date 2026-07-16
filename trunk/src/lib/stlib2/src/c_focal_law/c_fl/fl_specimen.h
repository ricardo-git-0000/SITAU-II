#ifndef fl_specimenH
#define fl_specimenH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

using namespace std;

class CFL_Specimen
{
public:
   // Creates the component
   // Crea el componente
   CFL_Specimen (float velocity);

   // Deletes the component
   // Elimina el componente
	~CFL_Specimen() {}

   // Sets the sound velocity inside material
   // Configura la velocidad de propagación del sonido en el interior del
   // material.
	int SetVelocity (float data);

   // Gets the sound velocity inside material
   // Devuelve la velocidad de propagación del sonido en el interior del
   // material.
   float GetVelocity (void) {return velocity_;}

protected:
	// (m/s) Sound velocity inside material
	// (m/s) Velocidad del sonido en el interior del material
   float velocity_;

private:
};

#endif


