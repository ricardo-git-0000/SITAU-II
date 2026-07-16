// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_image_handle.h"
#include "log.h"
#include "pcie_utils.h"
#include "pcie_tad.h"
#include "pcie_error_code.h"
#include "api_pcie.h"
#include "stlib2.h"
#include "pcie.h"
#include "pcie_thread_ge.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

CImageHandle *gb_image_handle = nullptr;

// _____________________________________________________________________________
// ------ CImageHandle class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
CImageHandle::CImageHandle(fuse *fuse_pcie_drv, int fifo_pointers, int focal_laws_per_image, int focal_laws_per_block, int frame_length)
{
	image_manager_init(fuse_pcie_drv, fifo_pointers, focal_laws_per_image, focal_laws_per_block, frame_length);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
CImageHandle::~CImageHandle(void)
{
	blocks_fifo_.clear();
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
void CImageHandle::image_manager_init(fuse *p_fuse_pcie_drv, int fifo_pointers, int p_focal_laws_per_image, int p_focal_laws_per_block, int frame_length)
{
	current_block_read_ = 0;
	current_block_fred_ = 0;
	current_image_read_ = 0;
	current_image_fred_ = 0;
	current_block_focal_law_not_read_ = 0;
	current_block_focal_law_not_fred_ = p_focal_laws_per_block;
	num_valid_focal_laws_last_block_ = p_focal_laws_per_block;

	fifo_read_index_ = 0;
	fifo_fred_index_ = 0;
	current_images_unfred_ = 0;
	fifo_size_ = fifo_pointers;
	blocks_fifo_.resize(fifo_pointers);
	fuse_pcie_drv_ = p_fuse_pcie_drv;
	first_image_read_ = false;
	last_image_read_ = false;
	last_block_read_ = false;
	first_image_fred_ = false;
	last_image_fred_ = false;
	last_image_index_ = 0;
	last_block_index_ = 0;
	focal_laws_per_image_ = p_focal_laws_per_image;
	focal_laws_per_block_ = p_focal_laws_per_block;
	num_shorts_per_focal_law_ = frame_length * N_CHANNELS_MODULE * N_MODULES;
	paused_ = false;
	request_continue_ = false;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int CImageHandle::FreeOneImage_Pointers(unsigned long *image_index_fred)
{
	int test_block_header = 1;
	int focal_laws_to_free;

	if (!current_images_unfred_)
		return 0;

	focal_laws_to_free = focal_laws_per_image_;

	while (focal_laws_to_free)
	{
		if (current_block_focal_law_not_fred_ >= focal_laws_to_free)
		{
			current_block_focal_law_not_fred_ -= focal_laws_to_free;
			focal_laws_to_free = 0;
			//Detección de última imagen. Si estamos liberando la última imagen, es un caso especial porque hay que liberar el buffer completo.
			if ((last_image_read_ == true) && (current_image_fred_ == last_image_index_))
			{
				fuse_pcie_drv_->free_dma_buffer(blocks_fifo_[fifo_fred_index_]); //Liberamos el buffer!
				last_image_fred_ = true;
				
				if ((last_block_index_ != current_block_fred_) || (current_images_unfred_ != 1))
				{
					snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "Image manager Error: Inconsistent internal data (last_block_index_ != current_block_fred_): current_images_unfred = %d - current_image_fred = %d - current_image_read = %d - current_block_read = %d - current_block_fred = %d - fifo_fred_index = %d - fifo_read_index_ = %d - focal_laws_to_free = %d - last_image_index_ %d - last_block_index_ %d",
						current_images_unfred_, current_image_fred_, current_image_read_, current_block_read_, current_block_fred_, fifo_fred_index_, fifo_read_index_, focal_laws_to_free, last_image_index_, last_block_index_);
					printf("\r\n%s", gb_log_msg);
					return -1;
				}
				current_images_unfred_ = 0;
				int kk = gb_drv_handle->dma_buffer_leakage();
				if (kk > 0)
				{ ELOG(charEPCIE_BUFFER_LEAKAGE, EPCIE_BUFFER_LEAKAGE); }
			}
		}
		else
		{
			focal_laws_to_free -= current_block_focal_law_not_fred_;
			current_block_focal_law_not_fred_ = 0;
			fuse_pcie_drv_->free_dma_buffer(blocks_fifo_[fifo_fred_index_]); //Liberamos el buffer!

			if (calc_used_fifo_blocks())
			{
				if ((fifo_fred_index_ + 1) >= fifo_size_)
					fifo_fred_index_ = 0;
				else
					fifo_fred_index_++;

				current_block_focal_law_not_fred_ = focal_laws_per_block_;
				current_block_fred_++;
			}
			else
			{
				snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "Image manager Error: Inconsistent internal data: current_images_unfred = %d - current_image_fred = %d - current_image_read = %d - current_block_read = %d - current_block_fred = %d - fifo_fred_index = %d - fifo_read_index_ = %d - focal_laws_to_free = %d - last_image_index_ %d - last_block_index_ %d",
					current_images_unfred_, current_image_fred_, current_image_read_, current_block_read_, current_block_fred_, fifo_fred_index_, fifo_read_index_, focal_laws_to_free, last_image_index_,last_block_index_);
				//printf("\nImage manager Error: Inconsistent internal data:\n");
				//printf("current_images_unfred = %d\n", current_images_unfred_);
				//printf("current_image_fred = %d\n", current_image_fred_);
				//printf("current_image_read = %d\n", current_image_read_);
				//printf("current_block_read = %d\n", current_block_read_);
				//printf("current_block_fred = %d\n", current_block_fred_);
				//printf("fifo_fred_index = %d\n", fifo_fred_index_);
				//printf("fifo_read_index_ = %d\n", fifo_read_index_);
				//printf("focal_laws_to_free = %d\n", focal_laws_to_free);
				printf("\r\n%s",gb_log_msg);
				return -1;
			}
		}


	}

	current_images_unfred_--;
	if (image_index_fred != NULL)
		*image_index_fred = current_image_fred_;

	current_image_fred_++;
	return 1;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int CImageHandle::GetOneImage_Pointers(short int **p_focal_laws_pointers, unsigned long *p_current_image_index, T_block_header **p_last_block_header, bool paused)
{
	int result, test_block_header = 1;
	int num_blocks_to_read;
	int focal_law_sent_index = 0;
	int free_fifo_blocks, free_pcie_blocks;
	short int* block_data_start_ptr;
	unsigned short index_block_focal_law;
	unsigned long next_image_read_index, next_block_read_index;

	if (focal_laws_per_image_ <= current_block_focal_law_not_read_)
		num_blocks_to_read = 0;
	else
		num_blocks_to_read = CEILING((focal_laws_per_image_ - current_block_focal_law_not_read_), focal_laws_per_block_);

	if ((free_pcie_blocks = PCIe_GetFreeBlocksNumber()) < 0) return 0;
  //printf("\nfree_pcie_blocks = %d", free_pcie_blocks);
  if (free_pcie_blocks < gb_pcie_buffer_n_blocks_buf * 0.3)
	{
		if (!paused && !paused_)
		{
			ThrGE_Pause(GE_SOURCE_ACQ_BUFFER_DRV);
			paused_ = true;
		}
	}
	else if (paused_ && (free_pcie_blocks > gb_pcie_buffer_n_blocks_buf * 0.7))
	{
		request_continue_ = true;
		//ST_Continue();
		paused_ = false;
	}

	if ((result = gb_drv_handle->wait_for_dma_buffer(num_blocks_to_read, TIMEOUT_DMA_BLOCKS_MS)) < 0) 
		return RLOG(result);
	if (result < num_blocks_to_read) return 0;

	free_fifo_blocks = calc_free_fifo_blocks();
	if (free_fifo_blocks < num_blocks_to_read) 
		return ELOG("Need more memory in pointers FIFO", EPCIE_USER);

	if (!first_image_read_)
	{
		next_image_read_index = 0;
		next_block_read_index = 0;
	}
	else
	{
		next_image_read_index = current_image_read_ + 1;
		next_block_read_index = current_block_read_ + 1;
	}

	while (focal_law_sent_index < focal_laws_per_image_)
	{
		if (current_block_focal_law_not_read_)
		{
			if (last_block_read_)
			{
				index_block_focal_law = num_valid_focal_laws_last_block_ - current_block_focal_law_not_read_;
			}
			else
				index_block_focal_law = focal_laws_per_block_ - current_block_focal_law_not_read_;

			block_data_start_ptr = (short int *)&((blocks_fifo_[fifo_read_index_])[1]);
			for (index_block_focal_law; current_block_focal_law_not_read_ > 0 && (index_block_focal_law < focal_laws_per_block_) && (focal_law_sent_index < focal_laws_per_image_); current_block_focal_law_not_read_--, index_block_focal_law++, focal_law_sent_index++)
				p_focal_laws_pointers[focal_law_sent_index] = &block_data_start_ptr[num_shorts_per_focal_law_ * index_block_focal_law];
		}
		else
		{
			unsigned int next_fifo_read_index;
			int n_blocks_read, result;

			if (!first_image_read_)
				next_fifo_read_index = 0;
			else
			{
				if ((fifo_read_index_ + 1) >= fifo_size_)
					next_fifo_read_index = 0;
				else
					next_fifo_read_index = fifo_read_index_ + 1;
			}

			if ((result = PCIe_GetPtrBlocks(&n_blocks_read, (void**) &(blocks_fifo_[next_fifo_read_index]), 1)) < 0)
			{
				snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "Error PCIe_GetPtrBlocks returns error %d", result);
				return ELOG(gb_log_msg, result);
			}
			if (n_blocks_read > 1)
			{
				snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "Error! %d block read, asked for only 1!", n_blocks_read);
				return ELOG(gb_log_msg, result);
			}
			if (n_blocks_read == 0)
			{
				printf("\r\nError!No había buffers cuando se nos prometío que habría!");
				ELOG("Error! No había buffers cuando se nos prometío que habría!", EPCIE_USER);
				return 0;
			}
			if (test_block_header)
			{
				if ((result = PCIe_CheckPCIeHeader_fast((void *)blocks_fifo_[next_fifo_read_index])) < 0)
				{
					//PCIe_InitBoard();
					return RLOG(result);
				}
				if (blocks_fifo_[next_fifo_read_index]->header_struct.image_index != next_image_read_index)
				{
					snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "Image manager: block image index = %d != image manager index = %d", blocks_fifo_[next_fifo_read_index]->header_struct.image_index, next_image_read_index);
					ELOG(gb_log_msg, result);
				}
				if (blocks_fifo_[next_fifo_read_index]->header_struct.image_focal_law_word.bits.image_focal_law_index != focal_law_sent_index)
				{
					snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "Image manager: focal law image index = %d != image manager focallaw index = %d", blocks_fifo_[next_fifo_read_index]->header_struct.image_focal_law_word.bits.image_focal_law_index, focal_law_sent_index);
					ELOG(gb_log_msg, result);
				}
			}
			/*printf("HBI = %3d HII = %3d MII = %3d HFI = %2d MFI = %2d\n",
				blocks_fifo_[next_fifo_read_index]->header_struct.block_index,
				blocks_fifo_[next_fifo_read_index]->header_struct.image_index,
				next_image_read_index,
				blocks_fifo_[next_fifo_read_index]->header_struct.image_focal_law_word.bits.image_focal_law_index,
				focal_law_sent_index);
				*/

			first_image_read_ = true;
			fifo_read_index_ = next_fifo_read_index;
			if (blocks_fifo_[next_fifo_read_index]->header_struct.ch_id_last.bits.last_block)
			{
				num_valid_focal_laws_last_block_ = blocks_fifo_[next_fifo_read_index]->header_struct.image_focal_law_word.bits.image_focal_law_total;
				current_block_focal_law_not_read_ = num_valid_focal_laws_last_block_;
				last_block_read_ = true;
				last_block_index_ = blocks_fifo_[next_fifo_read_index]->header_struct.block_index;// next_block_read_index;
			}
			else
				current_block_focal_law_not_read_ = focal_laws_per_block_;

			current_block_read_ = next_block_read_index;
		}
	}
	current_image_read_ = next_image_read_index;

	if (p_current_image_index != NULL)
		*p_current_image_index = next_image_read_index;

	if (p_last_block_header != NULL)
		*p_last_block_header = blocks_fifo_[fifo_read_index_];

	if ((current_block_focal_law_not_read_ == 0) && (last_block_read_ == true))
	{
		last_image_read_ = true;
		last_image_index_ = current_image_read_;
	}
		

	//Debería ser atomica o protegida
	current_images_unfred_++;

	return 1;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int CImageHandle::calc_used_fifo_blocks(void)
{
	int used_fifo_blocks;
	if (fifo_read_index_ >= fifo_fred_index_) used_fifo_blocks = fifo_fred_index_ - fifo_read_index_;
	else used_fifo_blocks = fifo_read_index_ + 1 + fifo_size_ - fifo_fred_index_;
	return used_fifo_blocks;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int CImageHandle::calc_free_fifo_blocks(void)
{
	int free_fifo_blocks;
	if (fifo_read_index_ >= fifo_fred_index_)	free_fifo_blocks = (fifo_size_ - fifo_read_index_) + fifo_fred_index_;
	else free_fifo_blocks = fifo_fred_index_ - fifo_read_index_;
	return free_fifo_blocks;
}
