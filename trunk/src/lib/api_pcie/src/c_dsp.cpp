// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_dsp.h"
#include "log.h"
#include "pcie_error_code.h"
#include "armadillo"
#include "typedef_mat.hpp"

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ======================================================================

// _____________________________________________________________________________
// ------ CDSP class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CDSP::CDSP()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CDSP::~CDSP()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CDSP::FIRFilter(short *data, int length)
{
	fvec arma_in1, arma_in2, arma_out;
	float* ptr_signal = NULL, *ptr_out = NULL;
	short int* ptr_filtered_signal = NULL;
	unsigned int i_ascan, i_result, i_out, arma_size_out = 0, offset = 0;

	arma_in2 = fvec(fir_coefficients, n_fir_coefficients, false, true);

	for (i_ascan = 0; i_ascan < n_ascan; i_ascan++)
	{
		ptr_signal = signal + (i_ascan * n_samples);
		ptr_filtered_signal = filtered_signal + (i_ascan * n_samples);
		arma_in1 = fvec(ptr_signal, n_samples, false, true);

		arma_out = round(conv(arma_in1, arma_in2) * fir_factor);
		arma_size_out = arma_out.size();

		ptr_out = arma_out.memptr();

		if (arma_size_out > n_samples)
		{
			if (arma_size_out - n_samples >= n_fir_coefficients / 2)
				offset = n_fir_coefficients / 2;
		}
		else n_samples = arma_size_out;

		memcpy(data, &dsp_image_[0], length * 2);
	}
	return EPCIE_NONE;
}


