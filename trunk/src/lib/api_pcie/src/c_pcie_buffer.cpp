// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdlib.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "log.h"
#include "pcie_error_code.h"
#include "c_pcie_buffer.h"
#include "pcie_utils.h"
#include "fileformat.h"
#include "stlib2_wrapper.h"
#include "c_timeout.h"
#include "stlib2.h"
#include "c_critical_section.h"
#include "pcie_utils.h"
#include "pcie.h"
#include "pcie_thread_acq.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

C_PCIBuffer *gb_buffer_data = nullptr;
unsigned long gb_buffer_n_images_block = 0;
unsigned long gb_buffer_n_blocks = 0;

// _____________________________________________________________________________
// ------ C_PCIBuffer class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_PCIBuffer::C_PCIBuffer (unsigned long n_blocks, unsigned long n_images_blocks, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned long data_size, bool sorted, int *error, AnsiString name)
{
int result;

	LOG_Open(FILE_LOG_PCI_BUFFER, "PCIe User Buffer");
	count_images_ = 0;
	count_images_available_ = 0;
	paused_ = false;
	name_ = name;
	size_ = 0;
	image_size_ = 0;
	block_size_ = 0;
	data_size_ = 0;
	capacity_ = 0;
	n_images_ = 0;
	n_images_blocks_ = 0;
	n_blocks_ = 0;
	n_fl_ = 0;
	n_samples_ = 0;
	n_ascan_ = 0;
	fl_size_ = 0;
	idx_next_image_wr_ = -1;
	idx_next_image_rd_ = -1;
	idx_next_image_rd_bak_ = 0;
	idx_next_image_rd_locked_ = 0;// 0xFFFFFFFFFFFFFFFF;
	//full_ = false;
	buffer_current_image_index_ = nullptr;
	buffer_time_stamp_ = nullptr;
	buffer_images_ = nullptr;
	f_wr_ = nullptr;
	f_rd_ = nullptr;
	moving_average_kernel_size_ = 5;
	mutex_ = new CCriticalSection();
	data_logger_ = 0;

	Clear();
	if ((result = Resize (n_blocks, n_images_blocks, n_fl, n_ascan, n_samples, data_size, sorted)) < 0)
	{
		RLOG(result);
	}
	if (error != nullptr) *error = result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int C_PCIBuffer::DecImagesCounter(unsigned long long n_images)
//{
//	char str[255];
//
//	//::_InterlockedExchange((volatile unsigned __int64 *)&count_images_, (unsigned __int64)(count_images_-= n_images));
//
//	if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
//	if (count_images_ < n_images)
//	{
//		count_images_ = 0;
//		if (mutex_ != nullptr) mutex_->Release();
//		char str[255];
//		sprintf(str, "PCIe Buffer PULL (%llu): Image Counter(%llu), RD Pointer(%llu), WR Pointer(%llu)", n_images, count_images_, idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_wr_);
//		ELOG(str, EPCIE_USER);
//		return ELOG(charEPCIE_BUFFER_ADD_OVERFLOW, EPCIE_BUFFER_ADD_OVERFLOW);
//	}
//	count_images_-= n_images;
//	if (data_logger_ > 0)
//	{
//		sprintf(str, "Decrease Images:\t\t\t\tImages (%llu), Available Images (%llu), GET Pointer(%llu), RD Pointer(%llu), WR Pointer(%llu)", count_images_, count_images_available_, idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_rd_, idx_next_image_wr_);
//		LOG_Info(FILE_LOG_PCI_BUFFER, str);
//	}
//	if (mutex_ != nullptr) mutex_->Release();
//	return EPCIE_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int C_PCIBuffer::DecImagesAvailableCounter(unsigned long long n_images)
//{
//	char str[255];
//
//	if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
//	if (count_images_available_ < n_images)
//	{
//		count_images_ = 0;
//		if (mutex_ != nullptr) mutex_->Release();
//		char str[255];
//		sprintf(str, "PCIe Buffer PULL (%llu): Available Image Counter(%llu), RD Pointer(%llu), WR Pointer(%llu)", n_images, count_images_available_, idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_wr_);
//		ELOG(str, EPCIE_USER);
//		return ELOG(charEPCIE_BUFFER_ADD_OVERFLOW, EPCIE_BUFFER_ADD_OVERFLOW);
//	}
//	count_images_available_ -= n_images;
//	sprintf(str, "Decrease Available Images:\tImages (%llu), Available Images (%llu), GET Pointer(%llu), RD Pointer(%llu), WR Pointer(%llu)", count_images_, count_images_available_, idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_rd_, idx_next_image_wr_);
//	LOG_Info(FILE_LOG_PCI_BUFFER, str);
//	if (mutex_ != nullptr) mutex_->Release();
//	return EPCIE_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int C_PCIBuffer::IncImagesCounter(void)
//{
//	char str[255];
//
//	//::_InterlockedExchange((volatile unsigned __int64 *)&count_images_, (unsigned __int64)(count_images_+1));
//
//	if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
//	if (count_images_ >= n_images_)
//	{
//		if (mutex_ != nullptr) mutex_->Release();
//		//char str[255];
//		sprintf(str, "PCIe Buffer PUSH: Image Counter(%llu), RD Pointer(%llu), WR Pointer(%llu)", count_images_, idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_wr_);
//		ELOG(str, EPCIE_USER);
//		return ELOG(charEPCIE_BUFFER_ADD_OVERFLOW, EPCIE_BUFFER_ADD_OVERFLOW);
//	}
//	if (count_images_available_ >= n_images_)
//	{
//		if (mutex_ != nullptr) mutex_->Release();
//		//char str[255];
//		sprintf(str, "PCIe Buffer PUSH: Available Image Counter(%llu), RD Pointer(%llu), WR Pointer(%llu)", count_images_available_, idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_wr_);
//		ELOG(str, EPCIE_USER);
//		return ELOG(charEPCIE_BUFFER_ADD_OVERFLOW, EPCIE_BUFFER_ADD_OVERFLOW);
//	}
//	count_images_++;
//	count_images_available_++;
//
//	sprintf(str, "Increase Images:\t\t\t\tImages (%llu), Available Images (%llu), GET Pointer(%llu), RD Pointer(%llu), WR Pointer(%llu)", count_images_, count_images_available_, idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_rd_, idx_next_image_wr_);
//	LOG_Info(FILE_LOG_PCI_BUFFER, str);
//	if (mutex_ != nullptr) mutex_->Release();
//	return EPCIE_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::GetImagesCounter(unsigned long *n_images_available)
{
	int result = 0;

	if (n_images_available != nullptr) *n_images_available = 0;
	if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
	if (idx_next_image_wr_ >= 0)
	{
		if (idx_next_image_wr_ > idx_next_image_rd_locked_)
			result = idx_next_image_wr_ - idx_next_image_rd_locked_;
		else if (idx_next_image_wr_ < idx_next_image_rd_locked_)
			//result = n_images_ - idx_next_image_rd_locked_;
			result = idx_next_image_wr_ + n_images_ - idx_next_image_rd_locked_;

		if (n_images_available != nullptr)
		{
			if (idx_next_image_wr_ > idx_next_image_rd_locked_)
				*n_images_available = result;
			else 
				*n_images_available = idx_next_image_wr_ + n_images_ - idx_next_image_rd_locked_;
				//*n_images_available = n_images_ - idx_next_image_rd_locked_;
		}
	}
	if (mutex_ != nullptr) mutex_->Release();
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool C_PCIBuffer::IsFillingRatio(float data)
{
	bool result = false;

	result = GetImagesCounter(nullptr) < abs(n_images_ * data);
	//if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
	//if (idx_next_image_wr_ != PTR_VOID)
	//{
	//	result = count_images_ > count_images_ - abs(idx_next_image_wr_ - idx_next_image_rd_locked_) * data);
	//}
	////result = count_images_ >= Round64u(data * n_images_);
	//if (mutex_ != nullptr) mutex_->Release();
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool C_PCIBuffer::IsFull(void)
{
	//return full_;
bool result = false;
//unsigned long long next_image_wr;

	if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
	if (idx_next_image_wr_ >= 0)
	{
		//next_image_wr = idx_next_image_wr_ + 1;
		//if (next_image_wr >= n_images_) next_image_wr = 0;
    if (idx_next_image_wr_ > idx_next_image_rd_locked_)
    {
      result = idx_next_image_rd_locked_ <= 0 && idx_next_image_wr_ == n_images_ - 1;
    }
    else 
		  result = abs(idx_next_image_wr_ - idx_next_image_rd_locked_) == 1;
	}
	//result = (idx_next_image_rd_locked_ != 0xFFFFFFFFFFFFFFFF && idx_next_image_wr_ == idx_next_image_rd_locked_ - 1) || count_images_ >= n_images_ - 1;
	if (mutex_ != nullptr) mutex_->Release();
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool C_PCIBuffer::IsEmpty(void)
{
	bool result = true;

	if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
	if (idx_next_image_wr_ >= 0)
	{
		result = (idx_next_image_wr_ == idx_next_image_rd_);// || count_images_ == 0;
	}
	//result = (idx_next_image_wr_ == idx_next_image_rd_) || count_images_ == 0;
	if (mutex_ != nullptr) mutex_->Release();
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void C_PCIBuffer::FileLogEnabled(bool data)
{
	if (f_wr_ != nullptr) fclose(f_wr_);
	if (f_rd_ != nullptr) fclose(f_rd_);
	f_wr_ = nullptr;
	f_rd_ = nullptr;
	if (data)
	{
		f_wr_ = fopen("BufferWR.txt", "wt");
		f_rd_ = fopen("BufferRD.txt", "wt");
	}
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void C_PCIBuffer::Print(FILE *f)
{
  time_t t;
  char str[255];

   if (size_ > 0 && f != nullptr)
   {
       t = time(nullptr);
       strftime(str, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
       fprintf(f, "\n[%s]: PCIe User Buffer.-", str);
       fprintf(f, "\n-----------------------------------------------------------------");
       fprintf(f, "\n   N. Blocks:         %d", n_blocks_);
       fprintf(f, "\n   N. Image Blocks:   %d", n_images_blocks_);
    if (size_ > 1048576) fprintf(f, "\n   Size:          %.2f MBytes", (float)size_/1048576.);
    else if (size_ > 1024) fprintf(f, "\n   Size:          %.2f KBytes", (float)size_/1024.);
    else fprintf(f, "\n   Size:          %llu Bytes", size_);
    fprintf(f, "\n   Capacity:      %llu Data (%lu Bytes/Data)", capacity_, data_size_);
    fprintf(f, "\n   Data Images:   %lu Images (%lu Data/Image)", n_images_, image_size_);
    fprintf(f, "\n   N. Focal Laws: %lu", n_fl_);
    fprintf(f, "\n   N. A-Scans:    %lu", n_ascan_);
    fprintf(f, "\n   N. Samples:    %lu", n_samples_);
    fprintf(f, "\n-----------------------------------------------------------------");
   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_PCIBuffer::~C_PCIBuffer (void)
{
	if (buffer_images_ != nullptr)
	{
		free(buffer_images_);
		//ELOG("delete buffer_images_", EPCIE_USER);
	}
	if (buffer_current_image_index_ != nullptr)
	{
		free(buffer_current_image_index_);
		//ELOG("delete buffer_current_image_index_", EPCIE_USER);
	}
	if (buffer_time_stamp_ != nullptr)
	{
		free(buffer_time_stamp_);
		//ELOG("delete buffer_time_stamp_", EPCIE_USER);
	}
	buffer_images_ = nullptr;
	buffer_current_image_index_ = nullptr;
	buffer_time_stamp_ = nullptr;
	if (mutex_ != nullptr) delete mutex_;
	mutex_ = nullptr;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::Clear (void)
{
   idx_next_image_wr_ = -1;
   idx_next_image_rd_ = -1;
   idx_next_image_rd_bak_ = 0;
   idx_next_image_rd_locked_ = 0;// 0xFFFFFFFFFFFFFFFF;// this->n_images_ + 1;
   count_images_ = 0;
   last_image_read_ = 0;
   //full_ = false;
   return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::Resize (unsigned long n_blocks, unsigned long n_images_blocks, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned long data_size, bool sorted)
{
	FILE *f = nullptr;
	time_t t = time(nullptr);

    if (!n_blocks || !n_images_blocks || !n_fl || !n_ascan || !n_samples || data_size != sizeof(short)) return ELOG(charEPCIE_GeometrySize, EPCIE_GeometrySize); // Rechaza dimensiones nulas antes de reservar.
    const uint64 law_samples = static_cast<uint64>(n_ascan) * n_samples; // El producto se hace antes en 64 bits.
    if (law_samples > 0xFFFFFFFFULL / n_fl) return ELOG(charEPCIE_GeometrySize, EPCIE_GeometrySize); // image_size_ debe caber en unsigned long de Windows.
    const uint64 image_samples = law_samples * n_fl; // Producto ya acotado.
    if (image_samples > 0xFFFFFFFFULL / n_images_blocks || static_cast<uint64>(n_images_blocks) * n_blocks > 0xFFFFFFFFULL) return ELOG(charEPCIE_GeometrySize, EPCIE_GeometrySize); // Protege bloque e indices de imagen.
    if (image_samples * n_images_blocks > (static_cast<size_t>(-1) - 100ULL) / n_blocks / data_size || static_cast<uint64>(n_images_blocks) * n_blocks > static_cast<size_t>(-1) / sizeof(unsigned long)) return ELOG(charEPCIE_GeometrySize, EPCIE_GeometrySize); // Comprueba por division antes de multiplicar bytes, incluido el margen legacy y los indices.
	n_fl_ = n_fl;
	n_samples_ = n_samples;
	n_ascan_ = n_ascan;
	n_blocks_ = n_blocks;
	n_images_blocks_ = n_images_blocks;
	n_images_ = n_images_blocks * n_blocks;
	data_size_ = data_size;
	fl_size_ = n_ascan_ * n_samples_;
	image_size_ = fl_size_ * n_fl_;
	block_size_ = image_size_ * n_images_blocks_;
	capacity_ = (uint64)n_images_ * (uint64)image_size_;
	size_ = capacity_ * (uint64)data_size;
	sorted_ = sorted;
	//idx_next_image_wr_ = 0;
	//idx_next_image_rd_ = 0;
	//idx_next_image_rd_bak_ = 0;
	//idx_next_image_rd_locked_ = 0xFFFFFFFFFFFFFFFF;

	moving_average_image_.resize(fl_size_);

    short *image_buffer = static_cast<short *>(realloc(buffer_images_, static_cast<size_t>(size_ + 100))); // No pierde el puntero anterior si falla realloc.
    if (!image_buffer) return ELOG(charEPCIE_NO_MEMORY, EPCIE_NO_MEMORY); // El llamante detendra el arranque fallido.
    buffer_images_ = image_buffer; // Publica la reserva valida.
    unsigned long *index_buffer = static_cast<unsigned long *>(realloc(buffer_current_image_index_, static_cast<size_t>(n_images_) * sizeof(unsigned long))); // Multiplica en size_t.
    if (!index_buffer) return ELOG(charEPCIE_NO_MEMORY, EPCIE_NO_MEMORY); // Conserva la reserva anterior para su destruccion.
    buffer_current_image_index_ = index_buffer; // Actualiza indices tras reservar.
    unsigned long *timestamp_buffer = static_cast<unsigned long *>(realloc(buffer_time_stamp_, static_cast<size_t>(n_images_) * sizeof(unsigned long))); // Evita overflow intermedio de 32 bits.
    if (!timestamp_buffer) return ELOG(charEPCIE_NO_MEMORY, EPCIE_NO_MEMORY); // Conserva la propiedad de la memoria anterior.
    buffer_time_stamp_ = timestamp_buffer; // Completa las reservas de usuario.

	//if ((f = fopen("BufferCFG.txt", "wt")) != nullptr)
	//{
	//	t = time(nullptr);
	//	strftime(str, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
	//	fprintf(f, "[%s] PCIe User Buffer",str);
	//	fprintf(f, "\n-----------------------------------------------------------------\n");
	//	fprintf(f, "\rBlocks Number:         %d", n_blocks);
	//	fprintf(f, "\rImages Block Number:   %d", n_images_blocks);
	//	fprintf(f, "\rFocal Laws Number:     %d", n_fl);
	//	fprintf(f, "\rA-Scan Signals Number: %d", n_ascan);
	//	fprintf(f, "\rSamples Number:        %d", n_samples);
	//	fprintf(f, "\rCapacity:              %llu", size_);
	//	fflush(f);
	//	fclose(f);
	//}

	Clear();
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void C_PCIBuffer::FileLog(FILE *f, short int *ptr, unsigned long long id_image_rd_locked, unsigned long long id_image, unsigned long long id_fl, unsigned long size)
{
char str_char[255];
time_t t = time(nullptr);

	if (f != nullptr)
	{
		strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
		fprintf(f, "\r%s %llu %llu %llu %llu %d", str_char, id_image_rd_locked, id_image, id_fl, (unsigned long long)ptr, size);
		fflush(f);
	}
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool C_PCIBuffer::GetFreeBlocks(void)
{
	return GetFreeImages() >= n_images_blocks_;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long long C_PCIBuffer::GetFreeImages(void)
{
	return n_images_ - GetImagesCounter(nullptr);
	//unsigned long long result = 0;

	//if (idx_next_image_wr_ > idx_next_image_rd_locked_)
	//{
	//	result = n_images_ - (idx_next_image_wr_ - idx_next_image_rd_locked_ + 1);
	//}
	//else if (idx_next_image_wr_ < idx_next_image_rd_locked_)
	//{
	//	result = idx_next_image_rd_locked_ - idx_next_image_wr_ + 1;// -1;
	//}
	//else result = n_images_;// -1;
	//return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::movingAverageSubstractFilter(short *ptr_data)
{
	int k, ix1, ix2, length = (int)moving_average_image_.size();
	//Filtro de media móvil de tamaño "2L+1"

	for (int i = 0; i < length; i++)
	{
		//Calculo los índices a los datos de entrada sin salirme del vector
		ix1 = i - moving_average_kernel_size_;
		ix2 = i + moving_average_kernel_size_;
		if (ix1 < 0) ix1 = 0;
		if (ix2 > length - 1) ix2 = length - 1;
		//Acumulo sobre la muestra de salida recorriendo el tamaño de la ventana
		k = 0;
		for (int j = ix1; j <= ix2; j++)
		{
			moving_average_image_[i] = moving_average_image_[i] + ptr_data[j];
			k++;
		}
		moving_average_image_[i] = (ptr_data[i] - moving_average_image_[i] / k); //Promedio
	}
	memcpy(ptr_data, &moving_average_image_[0], length * 2);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::Add(vector<short int *> &focal_laws_pointers, unsigned long current_image_index, unsigned long timestamp)
{
	int i, result;
	unsigned long i_img = 0;
	short int *ptr_data = nullptr;
	char str[255];
	long idx_wr = idx_next_image_wr_;

	if (IsFull())
	{
		sprintf(str, "PCIe Buffer: RD Pointer(%llu), WR Pointer(%llu)", idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_wr_);
		ELOG(str, EPCIE_USER);
		return ELOG(charEPCIE_BUFFER_ADD_OVERFLOW, EPCIE_BUFFER_ADD_OVERFLOW);
	}

	if (idx_wr < 0) idx_wr = 0;
	if ((ptr_data = GetPtrBufferImage(idx_wr)) != nullptr)
	{
		for (i = i_img = 0; i < focal_laws_pointers.size(); i++, i_img += fl_size_)
		{
			if (i_img + fl_size_ > image_size_) return i;
			if (gb_driver_enabled)
			{
				if (gb_pin_to_channel.size() != n_ascan_) return ELOG(charEPCIE_ChannelMap, EPCIE_ChannelMap); // No permite que la ordenacion lea un mapa corto.
				if (sorted_)
				{
					if ((result = GetFocalLawSorted(focal_laws_pointers[i], fl_size_, ptr_data + i_img, n_ascan_, n_samples_, &gb_pin_to_channel[0])) < 0)
						return RLOG(result);
				}
				else
				{
					memcpy((short int *)(ptr_data + i_img), (short int *)focal_laws_pointers[i], fl_size_ * sizeof(unsigned short));
				}				
			}
			else
			{
				short int kk = (short int)focal_laws_pointers[i];
				for (unsigned long j = 0; j < fl_size_; j++)
					*(ptr_data + i_img + j) = kk; // Escribe la ley simulada en su posicion, no siempre sobre la primera.
			}
			if (f_wr_ != nullptr && i == 0) FileLog(f_wr_, (short int *)(ptr_data + i_img), idx_next_image_rd_locked_ == 0xFFFFFFFFFFFFFFFF ? -1 : idx_next_image_rd_locked_, idx_next_image_wr_, i, fl_size_ * sizeof(unsigned short));
		}

		if (buffer_current_image_index_ != nullptr)
			*(buffer_current_image_index_ + idx_wr) = current_image_index;

		if (buffer_time_stamp_ != nullptr)
		{
			if (timestamp != 0xFFFFFFFF)
				*(buffer_time_stamp_ + idx_wr) = timestamp;
			else
				*(buffer_time_stamp_ + idx_wr) = idx_wr;
		}

		if (fl_size_ * i != image_size_)
		{
			snprintf(gb_log_msg, gb_log_msg == nullptr ? 0 : SIZE_MSG_LOG, "Image size (%d) != Focal law size (%d) x Focal law number (%d)", image_size_, fl_size_, i);
			ELOG(gb_log_msg, EPCIE_USER);
		}

		if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
		
		if (idx_next_image_wr_ < 0) idx_next_image_wr_ = 0;
		if (data_logger_ > 0)
		{
			sprintf(str, "WR -->\tLOCKED(%d)\tRD(%d)\tWR(%d)", idx_next_image_rd_locked_, idx_next_image_rd_, idx_next_image_wr_);
			LOG_Info(FILE_LOG_PCI_BUFFER, str);
		}
		idx_next_image_wr_++;
		if (idx_next_image_wr_ >= n_images_) idx_next_image_wr_ = 0;
		if (mutex_ != nullptr) mutex_->Release();
	}
	else return ELOG(charEPCIE_BUFFER_NULL, EPCIE_BUFFER_NULL);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
short int *C_PCIBuffer::GetPtrBufferNextImage(unsigned long *current_image_index, unsigned long *time_stamp, unsigned long timeout_ms)
{
	short int *result = nullptr;
	C_Timeout *timeout = nullptr;
	long idx_rd = idx_next_image_rd_;
	
	if (current_image_index != nullptr) *current_image_index = -1;
	if (time_stamp != nullptr) *time_stamp = 0;
	//if (IsEmpty())
	//{
	//	//Sleep(50);
	//	return nullptr;
	//}
	try
	{
		if (timeout_ms > 0)
		{
			timeout = new C_Timeout(timeout_ms);
			timeout->Start();
		}
		do
		{
			if (!IsEmpty())
			{
				if (idx_rd < 0) idx_rd = 0;
				if (idx_rd != idx_next_image_wr_ && (result = GetPtrBufferImage(idx_rd)) != nullptr)
				{
					if (current_image_index != nullptr)
					{
						if (buffer_current_image_index_ != nullptr) *current_image_index = *(buffer_current_image_index_ + idx_rd);
						else *current_image_index = (unsigned long)idx_rd;
					}
					if (time_stamp != nullptr)
					{
						if (buffer_current_image_index_ != nullptr) *time_stamp = *(buffer_time_stamp_ + idx_rd);
					}

					if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
					if (data_logger_ > 0)
					{
						char str[255];
						sprintf(str, "RD -->\tLOCKED(%d)\tRD(%d)\tWR(%d)", idx_next_image_rd_locked_, idx_rd, idx_next_image_wr_);
						LOG_Info(FILE_LOG_PCI_BUFFER, str);
					}
					//idx_next_image_rd_ += n_images_blocks_;
					idx_next_image_rd_++;
					if (idx_next_image_rd_ >= n_images_) idx_next_image_rd_ = 0;
					if (mutex_ != nullptr) mutex_->Release();
					if (f_rd_ != nullptr) FileLog(f_rd_, result, idx_next_image_rd_locked_, idx_next_image_rd_, 0, fl_size_ * sizeof(unsigned short));
					return result;
				}
			}
			else Sleep(50);
		} while (timeout != nullptr && !timeout->Timeout());
		return result;
	}
	__finally
	{
		if (timeout != nullptr) delete timeout;
	}
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
short int *C_PCIBuffer::GetPtrBufferNextBurst(unsigned long *current_image_index, unsigned long *time_stamp, unsigned long *n_images_buffer, unsigned long *n_images_availables, unsigned long timeout_ms)
{
	short int *result = nullptr;
	C_Timeout *timeout = nullptr;
	long n_images, idx_rd = idx_next_image_rd_;
	bool fullBurst = false;

	if (current_image_index != nullptr) *current_image_index = -1;
	if (time_stamp != nullptr) *time_stamp = 0;
	if (n_images_availables != nullptr) *n_images_availables = 0;
	if (n_images_buffer != nullptr) *n_images_buffer = 0;
	//n_images = GetImagesCounter(n_images_availables);
	//if (n_images_buffer != nullptr) *n_images_buffer = n_images;
	//if (n_images <= 0 || timeout_ms > 0 && n_images < n_images_blocks_)
	//{
	//	//Sleep(50);
	//	return nullptr;
	//}
	try
	{
		if (timeout_ms > 0)
		{
			timeout = new C_Timeout(timeout_ms);
			timeout->Start();
		}
		do
		{
			n_images = GetImagesCounter(n_images_availables);
			//printf("\n%d",n_images);
			fullBurst = n_images >= n_images_blocks_;
			//if (!fullBurst)
			//{
				//n_images = GetImagesCounter(n_images_availables);
				//fullBurst = n_images >= n_images_blocks_;
			//}
			if (fullBurst)
			{
				if (idx_rd < 0) idx_rd = 0;
				if ((result = GetPtrBufferImage(idx_rd)) != nullptr)
				{
					if (current_image_index != nullptr)
					{
						if (buffer_current_image_index_ != nullptr) *current_image_index = *(buffer_current_image_index_ + idx_rd);
						else *current_image_index = (unsigned long)idx_rd;
					}
					if (time_stamp != nullptr)
					{
						if (buffer_time_stamp_ != nullptr) *time_stamp = *(buffer_time_stamp_ + idx_rd);
					}
					if (n_images_buffer != nullptr) *n_images_buffer = n_images;
					//if (n_images_availables != nullptr) *n_images_availables = n_images_blocks_;

					if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }

					if (idx_next_image_rd_ < 0) idx_next_image_rd_ = 0;
					if (data_logger_ > 0)
					{
						char str[255];
						sprintf(str, "RD -->\tLOCKED(%d)\tRD(%d)\tWR(%d)", idx_next_image_rd_locked_, idx_next_image_rd_, idx_next_image_wr_);
						LOG_Info(FILE_LOG_PCI_BUFFER, str);
					}
					idx_next_image_rd_ += n_images_blocks_;
					if (idx_next_image_rd_ >= n_images_) idx_next_image_rd_ = 0;
					if (mutex_ != nullptr) mutex_->Release();
					if (f_rd_ != nullptr) FileLog(f_rd_, result, idx_next_image_rd_locked_, idx_next_image_rd_, 0, fl_size_ * sizeof(unsigned short));
					return result;
				}
				else
				{
					ELOG(charEPCIE_PtrAccess_NULL, EPCIE_PtrAccess_NULL);
					return result;
				}
			}
			else if (ThrAcq_Error() < 0) return nullptr; 
			//else Sleep(150);

		} while (timeout != nullptr && !timeout->Timeout());
		return result;
	}
	__finally
	{
		if (timeout != nullptr) delete timeout;
	}
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::FreePtrBufferImages(unsigned long long n_images)
{
	int result;

	if (mutex_ != nullptr && !mutex_->Acquire()) { ELOG(charEPCIE_MUTEX_ACQUIRE, EPCIE_MUTEX_ACQUIRE); }
	if (idx_next_image_rd_locked_ != idx_next_image_rd_)
	{
		//if ((result = DecImagesCounter(n_images)) < 0) return RLOG(result);
		if (data_logger_ > 0)
		{
			char str[255];
			sprintf(str, "FREE -->\tLOCKED(%d)\tRD(%d)\tWR(%d)", idx_next_image_rd_locked_, idx_next_image_rd_, idx_next_image_wr_);
			LOG_Info(FILE_LOG_PCI_BUFFER, str);
		}
		idx_next_image_rd_locked_ += n_images;
		if (idx_next_image_rd_locked_ >= n_images_) idx_next_image_rd_locked_ = 0;
	}
	if (mutex_ != nullptr) mutex_->Release();

	//full_ = false;
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::GetBufferNextImage(short int *data, unsigned long size_data, unsigned long *current_image_index, unsigned long *time_stamp, int timeout_ms)
{
	short int *ptr_data = nullptr;
	int result;

	if (size_data < image_size_) return ELOG(charEPCIE_PARAMETER, EPCIE_PARAMETER);
	if (size_data > image_size_) size_data = image_size_;
	if ((ptr_data = GetPtrBufferNextImage(current_image_index, time_stamp, timeout_ms)) == nullptr) return EPCIE_NONE;
	//if (!sorted_)
	//{
	//	if ((result = GetImageSorted(ptr_data, image_size_, data, fl_size_, n_fl_, n_ascan_, n_samples_, gb_pin_to_channel.size() > 0 ? &gb_pin_to_channel[0] : nullptr)) < 0)
	//		return RLOG(result);
	//}
	//else 
	memcpy((short int *)data, (short int *)ptr_data, sizeof(short int) * size_data);
	//printf("\r\n RD ptr_data %X", (unsigned long long)ptr_data);
	FreePtrBufferImages(1);
	//printf("\r\n[RD] count_images_ = %d", count_images_);
	return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
short int *C_PCIBuffer::GetPtrBufferImage(unsigned long long idx_image)
{
	uint64 addr_buffer;

	if (buffer_images_ == nullptr || idx_image >= n_images_) return nullptr;
	addr_buffer = idx_image * image_size_;
	return buffer_images_ + addr_buffer;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_PCIBuffer::SaveFileBuffer(char *file)
{
	int result;
	TFileStream ^f;
	short int *ptr_img = nullptr;
	unsigned long i;

	try
	{
		if ((f = FS_CreateFileImageData(file)) == nullptr) return ELOG(charEPCIE_File, EPCIE_File);
		for (i = 0; i < n_images_; i++)
		{
			if ((ptr_img = GetPtrBufferImage(i)) != nullptr)
			{
				if ((result = FS_AddFileImageData(f, ptr_img, image_size_)) < 0) return RLOG(result);
			}
		}
	}
	__finally
	{
		if (f != nullptr) delete f;
	}
	return EPCIE_NONE;
}




// _____________________________________________________________________________
// ------ 'C_PCIBuffer' Access Functions
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_PCIBuffer *CBufferScan_Resize (C_PCIBuffer *buffer_scan, unsigned long n_blocks, unsigned long n_images_blocks, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned long data_size, bool sorted, int *error, AnsiString name)
{
int result;

   if (buffer_scan == nullptr)
      buffer_scan = new C_PCIBuffer (n_blocks, n_images_blocks, n_fl, n_ascan, n_samples, data_size, sorted, error, name);
   else
   {
      if ((result = buffer_scan->Resize (n_blocks, n_images_blocks, n_fl, n_ascan, n_samples, data_size, sorted)) < 0)
         {RLOG(result);}
      if (error != nullptr) *error = result;
   }
   return buffer_scan;
}








