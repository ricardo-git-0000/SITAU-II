#ifndef fl_paH
#define fl_paH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

using namespace std;

class CFL_PA
{
public:
   // Creates the componente
   // Crea el componente
	CFL_PA (int n_elements, float frequency);

   // Deletes the component
   // Elimina el componente
   ~CFL_PA() {}
   
   // Set transducer elements number
   // Configura el numero de elmentos del transductor phased array
   void SetElementNumber (int n_element);

   // Get transducer elements number
   // Devuelve el numero de elmentos del transductor phased array
   int GetElementNumber (void) {return n_elements_;}

   float GetFrequency (void) {return frequency_;}

   vector<float> x_;
   vector<float> z_;

   float x_center_;
   float z_center_;

protected:

	int n_elements_;
	float frequency_;

private:

};
#endif

