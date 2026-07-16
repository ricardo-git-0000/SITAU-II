#ifndef fl_pa_uiH
#define fl_pa_uiH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <vector>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "fl_pa.h"

using namespace std;

class CFL_PAUI : public CFL_PA
{
public:

	int *update_scan_;
    vector<unsigned short> pin_to_channel_;
    bool pin_to_channel_assignment_;

   CFL_PAUI (unsigned short n_elements, float pitch, float felement_dist, float frequency, float radius);

   ~CFL_PAUI() {}

   int SetConfig (unsigned short n_elements, float pitch, float frequency, float felement_dist, float radius);

   int GetElementNumber (void) {return n_elements_;}

   float GetPitch (void) {return pitch_;}

   float GetRadius (void) {return radius_;}

   float GetFirstElementDist (void) {return felement_dist_;}

   int PACoordinates (void);

   int CONFIG_Copy (CFL_PAUI *pa_probe);

   void PinToChannel(vector<unsigned short> &data) { if (data.size() > 0) pin_to_channel_.assign(data.begin(), data.end()); }

   int PinToChannelSize(void) { return (int)pin_to_channel_.size(); }

   int FirstElementCoordinates(float* x, float* z);
   int LastElementCoordinates(float* x, float* z);

private:
   
    float pitch_;
    float felement_dist_;
    float radius_;
    int n_elements_;
};

#endif

