#ifndef fp_c_acq_dataH
#define fp_c_acq_dataH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_critical_section.h"


// _____________________________________________________________________________
// ------ CAcqData class
// _____________________________________________________________________________
// =============================================================================
class CAcqData
{
public:
   int id_fl;
   int n_parallel_beamformed_lines;
   vector<int16> data;
   uint32 acq_counter;
   uint32 n_frame_data;
   uint32 n_samples;
   uint16 n_ascan;
   uint16 n_blocks;
   uint16 n_blocks_32b;
   vector<uint8> n_lines_block;

   CAcqData();
   ~CAcqData() {}
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

#endif
