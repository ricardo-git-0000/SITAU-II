#ifndef fl_specimen_uiH
#define fl_specimen_uiH

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

#include "fl_specimen.h"

using namespace std;

class CFL_SpecimenUI : public CFL_Specimen
{
public:

	int *update_scan_;

   CFL_SpecimenUI (float velocity);

   ~CFL_SpecimenUI() {}

	int SetVelocity (float data);

	float GetVelocity (void) {return velocity_;}

   int CONFIG_Copy (CFL_SpecimenUI *specimen);

private:

};

#endif



