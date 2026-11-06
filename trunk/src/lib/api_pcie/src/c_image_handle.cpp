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
CImageHandle::CImageHandle(fuse *fuse_pcie_drv, int fifo_pointers, const T_PCIeGeometry &geometry) // Recibe la geometria validada de la adquisicion.
{
	image_manager_init(fuse_pcie_drv, fifo_pointers, geometry); // Conserva exactamente las dimensiones de DMA.
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
void CImageHandle::image_manager_init(fuse *p_fuse_pcie_drv, int fifo_pointers, const T_PCIeGeometry &geometry) // Recibe la geometria validada de la adquisicion.
{
	current_block_read_ = 0;
	current_block_fred_ = 0;
	current_image_read_ = 0;
	current_image_fred_ = 0;
	current_block_focal_law_not_read_ = 0;
	current_block_focal_law_not_fred_ = geometry.focal_laws_per_block; // Reinicia liberacion por bloque.
	num_valid_focal_laws_last_block_ = geometry.focal_laws_per_block; // El ultimo bloque puede reducirlo tras validar su cabecera.

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
	geometry_ = geometry; // Copia el contrato sin consultar globals durante la lectura.
	focal_laws_per_image_ = geometry.focal_laws_per_image; // Conserva las leyes por imagen.
	focal_laws_per_block_ = geometry.focal_laws_per_block; // Conserva las leyes por bloque.
	num_shorts_per_focal_law_ = static_cast<unsigned long>(geometry.shorts_per_focal_law); // Usa 128 o 256 canales efectivos.
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
				// El decremento comun libera la ultima imagen sin provocar underflow unsigned.
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

			if (calc_used_fifo_blocks() > 1) // Tras liberar el bloque actual debe existir otro retenido.
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

	if (last_image_read_) return 0; // No espera otro bloque despues del final de adquisicion.
	if (!p_focal_laws_pointers || !fuse_pcie_drv_) return ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); // Exige destino y fuente validos.
	if (fuse_pcie_drv_->dma_get_sw_err(0)) return ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); // Propaga una lectura DMA fallida en vez de esperar indefinidamente.
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
            { // Comprueba la ley completa antes de exponer su puntero.
                size_t offset_shorts = 0; // Desplazamiento relativo al payload del bloque.
                const unsigned int valid_laws = last_block_read_ ? num_valid_focal_laws_last_block_ : focal_laws_per_block_; // Excluye relleno del ultimo bloque.
                if ((result = PCIe_LawOffset(geometry_, static_cast<size_t>(geometry_.block_size_bytes), valid_laws, index_block_focal_law, offset_shorts)) < 0) return RLOG(result); // Rechaza desplazamientos fuera de capacidad.
                p_focal_laws_pointers[focal_law_sent_index] = block_data_start_ptr + offset_shorts; // Publica solamente una ley completa y comprobada.
            } // Fin de entrega de esta ley.
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
            T_block_header *received_header = blocks_fifo_[next_fifo_read_index]; // La lectura DMA garantiza el tamano fisico completo.
            const unsigned int valid_laws = received_header->header_struct.ch_id_last.bits.last_block ? received_header->header_struct.image_focal_law_word.bits.image_focal_law_total : focal_laws_per_block_; // Leyes utiles del bloque recibido.
            size_t checked_offset = 0; // Comprueba que incluso la ultima ley cabe en la reserva.
            unsigned int received_real_laws = 0; // Conserva intacta la cabecera con unidades de cuatro modulos.
            const int header_result = PCIe_DecodeBlockLaws(received_header->header_struct.block_focal_laws_word.bits.block_focal_laws, geometry_.transport_units_per_law, received_real_laws); // Traduce solo el contador nominal del bloque.
            if (header_result < 0 || received_real_laws != static_cast<unsigned int>(focal_laws_per_block_) || received_header->header_struct.magic_number != HEADER_MAGIC_NUMBER || received_header->header_struct.block_beats_word.bits.block_beats + 1 != geometry_.beats_per_module || received_header->header_struct.ch_id_last.bits.channel_id != 0 || received_header->header_struct.block_index != next_block_read_index || received_header->header_struct.image_index != next_image_read_index || received_header->header_struct.image_focal_law_word.bits.image_focal_law_index != focal_law_sent_index || (!received_header->header_struct.ch_id_last.bits.last_block && received_header->header_struct.image_focal_law_word.bits.image_focal_law_total != focal_laws_per_image_) || (received_header->header_struct.ch_id_last.bits.last_block && (valid_laws + focal_law_sent_index) % focal_laws_per_image_ != 0) || PCIe_LawOffset(geometry_, static_cast<size_t>(geometry_.block_size_bytes), valid_laws, valid_laws ? valid_laws - 1 : 0, checked_offset) < 0) // Comprueba formato, secuencia y ultimo bloque sin depender del modo de diagnostico.
            { // El bloque rechazado todavia no pertenece a la FIFO del gestor.
                fuse_pcie_drv_->free_dma_buffer(received_header); // Devuelve la memoria al productor.
                blocks_fifo_[next_fifo_read_index] = nullptr; // No conserva un puntero rechazado.
                return ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); // Detiene la adquisicion con una causa traducible.
            } // Fin de validacion de bloque.
			if (test_block_header)
			{
				if ((result = PCIe_CheckPCIeHeader_fast((void *)blocks_fifo_[next_fifo_read_index])) < 0)
				{
					fuse_pcie_drv_->free_dma_buffer(received_header); // Devuelve tambien los bloques rechazados por el verificador historico.
					blocks_fifo_[next_fifo_read_index] = nullptr; // Evita retener una referencia libre.
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

			current_block_read_ = next_block_read_index++; // Avanza por cada bloque, incluso si una imagen necesita varios.
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
	used_fifo_blocks = (!first_image_read_ || last_image_fred_) ? 0 : static_cast<int>(current_block_read_ - current_block_fred_ + 1); // Cuenta tambien el bloque retenido hasta liberar su ultima ley.
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
	free_fifo_blocks = static_cast<int>(fifo_size_) - calc_used_fifo_blocks(); // Distingue FIFO llena y vacia cuando coinciden los indices.
	return free_fifo_blocks;
}
