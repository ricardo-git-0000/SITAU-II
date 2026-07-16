//---------------------------------------------------------------------------

#ifndef c_image_handleH
#define c_image_handleH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fuse.h"
#include "pcie_tad.h"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ CImageHandle class Methods
// _____________________________________________________________________________
// =============================================================================

class CImageHandle
{
private:

	fuse *fuse_pcie_drv_;
	unsigned long current_block_read_;
	unsigned long current_block_fred_;
	unsigned long current_image_read_;
	unsigned long current_image_fred_;
	unsigned long current_images_unfred_;
	unsigned short current_block_focal_law_not_read_;
	unsigned short current_block_focal_law_not_fred_;
	unsigned short num_valid_focal_laws_last_block_;
	vector<T_block_header *> blocks_fifo_;
	unsigned long fifo_read_index_;
	unsigned long fifo_fred_index_;
	unsigned long fifo_size_;
	bool first_image_read_;
	bool last_image_read_;
	bool last_block_read_;
	bool first_image_fred_;
	bool last_image_fred_;
	unsigned long last_image_index_;
	unsigned long last_block_index_;
	unsigned short focal_laws_per_image_;
	unsigned short focal_laws_per_block_;
	unsigned long num_shorts_per_focal_law_;
	bool paused_;
	bool request_continue_;

	int calc_used_fifo_blocks(void);
	int calc_free_fifo_blocks(void);

public:

   int focal_law_mode_;

   CImageHandle(fuse *fuse_pcie_drv, int fifo_pointers, int focal_laws_per_image, int focal_laws_per_block, int frame_length);
   ~CImageHandle();
   void image_manager_init(fuse *p_fuse_pcie_drv, int fifo_pointers, int p_focal_laws_per_image, int p_focal_laws_per_block, int frame_length);
   int FreeOneImage_Pointers(unsigned long *image_index_fred);
   int GetOneImage_Pointers(short int **focal_laws_pointers, unsigned long *p_current_image_index, T_block_header **last_block_header, bool paused=false);
   bool LastImageRead(void) { return last_image_read_; }
   bool LastBlockRead(void) { return last_block_read_; }
   void ResetRequestContinue(void) { request_continue_ = paused_ = false; }
   bool RequestContinue(void) { return request_continue_; }
   bool Paused(void) { return paused_; }

};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern CImageHandle *gb_image_handle;

#endif
