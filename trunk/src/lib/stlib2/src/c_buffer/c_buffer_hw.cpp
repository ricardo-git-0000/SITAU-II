// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include <cstdlib> // Declara std::free para el liberador del buffer.
#include <memory> // Libera reservas parciales si falla la construccion.
#include <new> // Traduce bad_alloc a un codigo de error de la biblioteca.
#include "c_buffer_hw.h"
#include "c_log_file.h"
#include "utils.h"
#include "buffer_error_code.h"
#include "buffer_mc_data.h" // Declara SIZE_BUFFER_HW para la construccion comun.
#include "c_acq_data.h"
#include "sitau_hw.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "c_virtual_channel.h"
#include "st_vch_get.h"
#include "st_vch_fl_pa.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

namespace { // Mantiene el liberador como detalle interno de este archivo.
struct HWBufferDeleter // Evita decltype(&free), que provoca C1001 en esta compilacion.
{ // Liberador sin punteros a funciones de la biblioteca CRT.
    void operator()(short* data) const noexcept { std::free(data); } // Empareja la reserva malloc con free.
}; // Fin del liberador del buffer.
} // Fin del espacio de nombres interno.

CBufferHW *gb_buffer_hw = NULL;
int ST_EnsureHWBuffer(void) // Punto comun de construccion para no dejar excepciones de memoria escapar a MATLAB.
{ // Solo publica un buffer completamente construido.
    if (gb_buffer_hw != NULL) return EBUF_NONE; // Conserva el buffer ya existente.
    try // Incluye la reserva del objeto y las reservas de su constructor.
    { // Comienza la construccion protegida.
        gb_buffer_hw = new CBufferHW(SIZE_BUFFER_HW); // Una excepcion deja el puntero global a NULL.
    } // Fin de la construccion.
    catch (const std::bad_alloc&) // Atiende especificamente el fallo de memoria recuperable.
    { // Devuelve el error existente en el catalogo.
        return ELOG(charEBUF_NoMemory, EBUF_NoMemory); // -703: fallo de reserva, sin exit ni aborto del proceso.
    } // Fin de la conversion de excepcion.
    return EBUF_NONE; // El llamante puede continuar con el buffer valido.
} // Fin de creacion controlada.
vector<short int> vresult_buf;
vector<long> gb_buffer_frame;
unsigned long gb_last_acq_counter = 0;
unsigned long gb_index_fl_acq = 0;

// _____________________________________________________________________________
// ------ CBufferHW class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferHW::CBufferHW (unsigned long size)
{
	if ((buffer = (short *)malloc (sizeof(short) * (size * 2))) == NULL)
	{
		throw std::bad_alloc(); // ST_EnsureHWBuffer captura el fallo sin terminar MATLAB.
	}
	std::unique_ptr<short, HWBufferDeleter> buffer_guard(buffer); // Protege malloc mientras se construye el objeto.
	std::unique_ptr<CCriticalSection> lock_guard(new CCriticalSection("CBuffer_HW (lock)")); // Libera el primer lock si falla el segundo.
	std::unique_ptr<CCriticalSection> image_lock_guard(new CCriticalSection("CBuffer_HW (img)")); // Protege el lock de imagen durante la inicializacion.
	plock = lock_guard.get(); // Permite inicializar los contadores bajo el lock habitual.
	plock_img_ = image_lock_guard.get(); // Mantiene la proteccion hasta completar el constructor.
	mc_wr_img_ = NULL;
	mc_rd_img_ = NULL;
	pa_wr_img_ = NULL;
	pa_rd_img_ = NULL;
    tfm_wr_img_ = NULL;
    tfm_rd_img_ = NULL;
    acqbuf_capacity_ = size;
   n_samples_ = 0;
	Clear_AcquisitionBuffer();

   log_circular_buffer_ = false;
   log_acq_counter_ = false;
   log_acq_size_ = false;
   log_acq_number_ = false;
   flog_circular_buffer_ = NULL;
	flog_acq_counter_ = NULL;
	flog_acq_counter_0_ = NULL;
	flog_acq_counter_1_ = NULL;
	flog_acq_size_ = NULL;
   flog_acq_number_ = NULL;
   logname_circular_buffer_ = "LogHWCircularBuffer.txt";
   logname_acq_counter_ = "LogAcqBufferCounter.txt";
   logname_acq_counter_0_ = "LogAcqBufferCounter_0.txt";
   logname_acq_counter_1_ = "LogAcqBufferCounter_1.txt";
   logname_acq_size_ = "LogAcqBufferSize.txt";
   logname_acq_number_ = "LogAcqBufferNumber.txt";
   gb_index_fl_acq = 0;
   buffer_guard.release(); // El destructor del objeto completo pasa a liberar el buffer.
   lock_guard.release(); // Transfiere el lock al objeto construido.
   image_lock_guard.release(); // Transfiere el lock de imagen al objeto construido.
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferHW::~CBufferHW (void)
{
	Clear_AcquisitionBuffer();
	if (buffer != NULL)
	{
		free(buffer);
		buffer = NULL;
	}
	if (plock != NULL)
	{
		delete plock;
		plock = NULL;
	}
	if (plock_img_ != NULL)
	{
		delete plock_img_;
		plock_img_ = NULL;
	}
   DBG_FileClose();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferHW::DelImg(TBufferImg* img)
{
    if (img->data != NULL) free(img->data);
    free(img);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferHW::DelBufferImg (TBufferImg *img)
{
TBufferImg *free_img, *index_img = img;

    while (index_img != NULL)
    {
        free_img = index_img;
        index_img = index_img->sig;
        DelImg(free_img);
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TBufferImg *CBufferHW::CreateAcqImg (unsigned long acq_size)
{
TBufferImg *result = NULL;

   if (buffer == NULL) 
       return NULL;
   if (acq_size == 0) 
       return NULL;
	if ((result = (TBufferImg*)malloc(sizeof(TBufferImg))) == NULL)
	{
		ELOG(charEBUF_NoMemory,EBUF_NoMemory);
		return NULL;
	}
	if ((result->data = (short int *)malloc(sizeof(short int) * acq_size * 2)) == NULL)
	{
		free(result);
      ELOG(charEBUF_NoMemory,EBUF_NoMemory);
		return NULL;
	}

	result->size = acq_size * 2;
	result->size32 = acq_size;
	result->sig = NULL;

	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::AddAcqImg (TBufferImg *img_new, unsigned long header)
{
unsigned long imgbuf_size = 0, imgbuf_num = 0;

    if (img_new == NULL) return EBUF_NONE;
    switch (header)
    {   
    case TRM_FP_HEADER_v0: 
        if (plock_img_ != NULL && !plock_img_->Acquire(CS_WR)) return ELOG(charEBUF_Acquire,EBUF_Acquire);
	    if (mc_wr_img_ == NULL)
	    {
		    mc_wr_img_ = img_new;
		    mc_rd_img_ = img_new;
	    }
	    else
	    {
		    mc_wr_img_->sig = img_new;
		    mc_wr_img_ = img_new;
	    }
   	    mc_imgbuf_num_++;
	    mc_imgbuf_size_ += img_new->size;
        imgbuf_size = mc_imgbuf_size_;
        imgbuf_num = mc_imgbuf_num_;
        if (plock_img_ != NULL) plock_img_->Release();
        break;

    case TRM_PA_HEADER_v0:    

        if (plock_img_ != NULL && !plock_img_->Acquire(CS_WR)) return ELOG(charEBUF_Acquire,EBUF_Acquire);
        if (ST_GetAcquisitionType() == ACQUISITION_TYPE_TFM)
        {
            if (tfm_wr_img_ == NULL)
            {
                tfm_wr_img_ = img_new;
                tfm_rd_img_ = img_new;
            }
            else
            {
                tfm_wr_img_->sig = img_new;
                tfm_wr_img_ = img_new;
            }
            tfm_imgbuf_num_++;
            tfm_imgbuf_size_ += img_new->size;
            imgbuf_size = tfm_imgbuf_size_;
            imgbuf_num = tfm_imgbuf_num_;
        }
        else
        {
            if (pa_wr_img_ == NULL)
            {
                pa_wr_img_ = img_new;
                pa_rd_img_ = img_new;
            }
            else
            {
                pa_wr_img_->sig = img_new;
                pa_wr_img_ = img_new;
            }
            pa_imgbuf_num_++;
            pa_imgbuf_size_ += img_new->size;
            imgbuf_size = pa_imgbuf_size_;
            imgbuf_num = pa_imgbuf_num_;
        }
        if (plock_img_ != NULL) plock_img_->Release();
        break;
    default: break;
    }

    if (log_acq_number_ && flog_acq_number_ != NULL) fprintf(flog_acq_number_,"%d ",imgbuf_num);
    if (log_acq_size_ && flog_acq_size_ != NULL) fprintf(flog_acq_size_,"%d ",imgbuf_size);
	return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TBufferImg *CBufferHW::MC_GetAcqImg (void)
{
TBufferImg *result = NULL;
bool unlocked = true;

	if (mc_rd_img_ != NULL)
	{
      if (plock_img_ != NULL) unlocked = plock_img_->Acquire(CS_WR);
      if (unlocked)
      {
		   result = mc_rd_img_;
		   mc_rd_img_ = mc_rd_img_->sig;
		   if (mc_rd_img_ == NULL) mc_wr_img_ = NULL;
      }
      if (plock_img_ != NULL) plock_img_->Release();
    }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TBufferImg *CBufferHW::PA_GetAcqImg (void)
{
TBufferImg *result = NULL;
bool unlocked = true;

    if (pa_rd_img_ != NULL)
	{
      if (plock_img_ != NULL) unlocked = plock_img_->Acquire(CS_WR);
      if (unlocked)
      {
		   result = pa_rd_img_;
		   pa_rd_img_ = pa_rd_img_->sig;
		   if (pa_rd_img_ == NULL) pa_wr_img_ = NULL;
      }
      if (plock_img_ != NULL) plock_img_->Release();
    }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TBufferImg* CBufferHW::TFM_GetAcqImg(void)
{
    TBufferImg* result = NULL;
    bool unlocked = true;

    if (tfm_rd_img_ != NULL)
    {
        if (plock_img_ != NULL) unlocked = plock_img_->Acquire(CS_WR);
        if (unlocked)
        {
            result = tfm_rd_img_;
            tfm_rd_img_ = tfm_rd_img_->sig;
            if (tfm_rd_img_ == NULL) tfm_wr_img_ = NULL;
        }
        if (plock_img_ != NULL) plock_img_->Release();
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CBufferHW::MC_GetAcqImgSize (void)
{
unsigned long result = 0;

	if (mc_rd_img_ != NULL) result = mc_rd_img_->size;
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CBufferHW::PA_GetAcqImgSize (void)
{
unsigned long result = 0;

	if (pa_rd_img_ != NULL) result = pa_rd_img_->size;
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CBufferHW::TFM_GetAcqImgSize(void)
{
    unsigned long result = 0;

    if (tfm_rd_img_ != NULL) result = tfm_rd_img_->size;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CBufferHW::Print(FILE *f)
{
unsigned long size;

   if (f == NULL) return 0;

   fprintf(f,"\n\n---> 'HW FP Acquisition Buffer'");
   size = acqbuf_capacity_ + mc_imgbuf_num_ * mc_imgbuf_size_ * sizeof(short int);
   if (acqbuf_capacity_ > 1048576)
      fprintf(f,"\n   Size:\t\t\t\t\t%.2f MBytes", (float)size/1048576.);
   else if (acqbuf_capacity_ > 1024)
      fprintf(f,"\n   Size:\t\t\t\t\t%.2f KBytes", (float)size/1024.);
   else
      fprintf(f,"\n   Size:\t\t\t\t\t%lu Bytes", size);
   fprintf(f,"\n   Acquisition Images:\t\t%lu", mc_imgbuf_num_);
   fprintf(f,"\nAcquisition Images size:\t\t%lu samples", mc_imgbuf_size_);

   fprintf(f,"\n\n---> 'HW PA Acquisition Buffer'");
   size = acqbuf_capacity_ + pa_imgbuf_num_ * pa_imgbuf_size_ * sizeof(short int);
   if (acqbuf_capacity_ > 1048576)
      fprintf(f,"\n   Size:\t\t\t\t\t%.2f MBytes", (float)size/1048576.);
   else if (acqbuf_capacity_ > 1024)
      fprintf(f,"\n   Size:\t\t\t\t\t%.2f KBytes", (float)size/1024.);
   else
      fprintf(f,"\n   Size:\t\t\t\t\t%lu Bytes", size);
   fprintf(f,"\n   Acquisition Images:\t\t%lu", pa_imgbuf_num_);
   fprintf(f,"\nAcquisition Images size:\t\t%lu samples", pa_imgbuf_size_);

   fprintf(f, "\n\n---> 'HW TFM Acquisition Buffer'");
   size = acqbuf_capacity_ + tfm_imgbuf_num_ * tfm_imgbuf_size_ * sizeof(short int);
   if (acqbuf_capacity_ > 1048576)
       fprintf(f, "\n   Size:\t\t\t\t\t%.2f MBytes", (float)size / 1048576.);
   else if (acqbuf_capacity_ > 1024)
       fprintf(f, "\n   Size:\t\t\t\t\t%.2f KBytes", (float)size / 1024.);
   else
       fprintf(f, "\n   Size:\t\t\t\t\t%lu Bytes", size);
   fprintf(f, "\n   Acquisition Images:\t\t%lu", tfm_imgbuf_num_);
   fprintf(f, "\nAcquisition Images size:\t\t%lu samples", tfm_imgbuf_size_);
   return size;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferHW::DBG_FileOpen(void)
{
	DBG_FileClose();
   if (log_circular_buffer_)
      flog_circular_buffer_ = fopen(logname_circular_buffer_.c_str(),"w");
   if (log_acq_counter_)
   {
      flog_acq_counter_ = fopen(logname_acq_counter_.c_str(),"w");
      flog_acq_counter_0_ = fopen(logname_acq_counter_0_.c_str(),"w");
      flog_acq_counter_1_ = fopen(logname_acq_counter_1_.c_str(),"w");
   }
   if (log_acq_size_)
      flog_acq_size_ = fopen(logname_acq_size_.c_str(),"w");
   if (log_acq_number_)
      flog_acq_number_ = fopen(logname_acq_number_.c_str(),"w");
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferHW::DBG_FileClose(void)
{
	if (flog_circular_buffer_ != NULL) fclose(flog_circular_buffer_);
	if (flog_acq_counter_ != NULL) fclose(flog_acq_counter_);
	if (flog_acq_counter_0_ != NULL) fclose(flog_acq_counter_0_);
	if (flog_acq_counter_1_ != NULL) fclose(flog_acq_counter_1_);
	if (flog_acq_size_ != NULL) fclose(flog_acq_size_);
	if (flog_acq_number_ != NULL) fclose(flog_acq_number_);
   flog_circular_buffer_ = NULL;
	flog_acq_counter_ = NULL;
	flog_acq_counter_0_ = NULL;
	flog_acq_counter_1_ = NULL;
	flog_acq_size_ = NULL;
	flog_acq_number_ = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::Clear_HWBuffer (void)
{
	wr_ = rd_ = acqbuf_size_ = 0;
   gb_last_acq_counter = 0;
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::Clear_AcquisitionBuffer (void)
{
   if (plock_img_ != NULL && !plock_img_->Acquire(CS_WR)) return ELOG(charEBUF_Acquire,EBUF_Acquire);
   Clear_HWBuffer();
   acquired_images_ = 0;
	if (mc_rd_img_ != NULL) DelBufferImg(mc_rd_img_);
	mc_imgbuf_num_ = mc_imgbuf_size_ = 0;
	mc_wr_img_ = NULL;
	mc_rd_img_ = NULL;
	if (pa_rd_img_ != NULL) DelBufferImg(pa_rd_img_);
	pa_imgbuf_num_ = pa_imgbuf_size_ = 0;
	pa_wr_img_ = NULL;
	pa_rd_img_ = NULL;
    if (tfm_rd_img_ != NULL) DelBufferImg(tfm_rd_img_);
    tfm_imgbuf_num_ = tfm_imgbuf_size_ = 0;
    tfm_wr_img_ = NULL;
    tfm_rd_img_ = NULL;
    if (plock_img_ != NULL) plock_img_->Release();
    gb_last_acq_counter = 0;
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//unsigned long CBufferHW::NumDatos (void)  {return imgbuf_size_;}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::Add (short *data, unsigned int ndat)
{
unsigned int blq1, blq2;
unsigned long acqbuf_size = acqbuf_size_;

	if (data != NULL && ndat > 0)
	{
      if (plock != NULL && !plock->Acquire(CS_TRY)) return ELOG(charEBUF_Acquire,EBUF_Acquire);
      if (acqbuf_size_ + ndat > acqbuf_capacity_) 
         ReadSwap_AllBuffer();

      if (wr_ >= rd_)
      {
         if (wr_ + ndat >= acqbuf_capacity_)
         {
            blq1 = acqbuf_capacity_ - wr_;
            memcpy(&buffer[wr_], data, blq1*sizeof(short int));
            wr_ = 0;
            blq2 = ndat - blq1;
            if (rd_ < blq2)
            {
               Clear_HWBuffer();
               if (plock != NULL) plock->Release();
               return ELOG(charEBUF_Overflow,EBUF_Overflow);
            }
            else
            {
               memcpy(&buffer[wr_], data+blq1, sizeof(short int)*blq2);
               wr_+=blq2;
            }
         }
         else
         {
            memcpy(&buffer[wr_], data, sizeof(short int)*ndat);
            wr_+=ndat;
         }
      }
      else
      {
         if (rd_ - wr_ < ndat)
         {
            Clear_HWBuffer();
            if (plock != NULL) plock->Release();
            return ELOG(charEBUF_Overflow,EBUF_Overflow);
         }
         else
         {
            memcpy(&buffer[wr_], data, ndat*2);
            wr_+=ndat;
         }
      }
      acqbuf_size_ += ndat;
      acqbuf_size = acqbuf_size_;

      if (plock != NULL) plock->Release();
	}

	if (log_circular_buffer_ && flog_circular_buffer_ != NULL)
      fprintf(flog_circular_buffer_,"%d ",acqbuf_size);

   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::FP_AcquisitionNumber (void)
{
   if (mc_rd_img_ != NULL) return mc_imgbuf_num_;
   else return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::PA_AcquisitionNumber (void)
{
   if (pa_rd_img_ != NULL) return pa_imgbuf_num_;
   else return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::TFM_AcquisitionNumber(void)
{
    if (tfm_rd_img_ != NULL) return tfm_imgbuf_num_;
    else return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CBufferHW::FP_AcquisitionData (void)
{
   return (mc_rd_img_ != NULL && mc_imgbuf_num_ > 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CBufferHW::PA_AcquisitionData (void)
{
   return (pa_rd_img_ != NULL && pa_imgbuf_num_ > 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CBufferHW::TFM_AcquisitionData(void)
{
    return (tfm_rd_img_ != NULL && tfm_imgbuf_num_ > 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::MC_Read(unsigned long *n_data, unsigned long *header)
{
TBufferImg *ptr = NULL;
unsigned long imgbuf_size, imgbuf_num;
int i;


    if (n_data != NULL) *n_data = 0;
    if ((ptr = MC_GetAcqImg()) != NULL)
    {
        if (ptr->data != NULL && ptr->size > 1)
   	    {
            if (vresult_buf.size() < ptr->size) vresult_buf.resize(ptr->size);
            for (i=0; i<ptr->size && i<(unsigned long)vresult_buf.size(); i++) vresult_buf[i] = ptr->data[i];

            if (plock_img_ != NULL && !plock_img_->Acquire(CS_RD)) return ELOG(charEBUF_Acquire,EBUF_Acquire);
            mc_imgbuf_size_ -= ptr->size;
            mc_imgbuf_num_--;
            imgbuf_size = mc_imgbuf_size_;
            imgbuf_num = mc_imgbuf_num_;
            if (plock_img_ != NULL) plock_img_->Release();
            if (n_data != NULL) *n_data = ptr->size;
            if (header != NULL) *header = ((uint16)ptr->data[1] << 16) | (uint16)ptr->data[0];
   
   		    if (log_acq_number_ && flog_acq_number_ != NULL) fprintf(flog_acq_number_,"%d ",imgbuf_num);
		    if (log_acq_size_ && flog_acq_size_ != NULL) fprintf(flog_acq_size_,"%d ",imgbuf_size);
        }
        DelImg(ptr);
    }
	return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::PA_Read (unsigned long *n_data, unsigned long *header)
{
TBufferImg *ptr = NULL;
unsigned long imgbuf_size, imgbuf_num;
int i;

    if (n_data != NULL) *n_data = 0;
	if ((ptr = PA_GetAcqImg()) != NULL)
    {

        if (ptr->data != NULL && ptr->size > 1)
   	    {
            if (vresult_buf.size() < ptr->size) vresult_buf.resize(ptr->size);
            for (i=0; i<ptr->size && i< (unsigned long)vresult_buf.size(); i++) vresult_buf[i] = ptr->data[i];

            if (plock_img_ != NULL && !plock_img_->Acquire(CS_RD)) return ELOG(charEBUF_Acquire,EBUF_Acquire);
            pa_imgbuf_size_ -= ptr->size;
            pa_imgbuf_num_--;
            imgbuf_size = pa_imgbuf_size_;
            imgbuf_num = pa_imgbuf_num_;
            if (plock_img_ != NULL) plock_img_->Release();
            if (n_data != NULL) *n_data = ptr->size;
            if (header != NULL) *header = ((uint16)ptr->data[1] << 16) | (uint16)ptr->data[0];
   
   		    if (log_acq_number_ && flog_acq_number_ != NULL) fprintf(flog_acq_number_,"%d ",imgbuf_num);
		    if (log_acq_size_ && flog_acq_size_ != NULL) fprintf(flog_acq_size_,"%d ",imgbuf_size);
        }
        DelImg(ptr);
	}
	return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::TFM_Read(unsigned long* n_data, unsigned long* header)
{
    TBufferImg* ptr = NULL;
    unsigned long imgbuf_size, imgbuf_num;
    int i;

    if (n_data != NULL) *n_data = 0;
    if ((ptr = TFM_GetAcqImg()) != NULL)
    {
        if (ptr->data != NULL && ptr->size > 1)
        {
            if (vresult_buf.size() < ptr->size) vresult_buf.resize(ptr->size);
            for (i = 0; i < ptr->size && i < (unsigned long)vresult_buf.size(); i++) vresult_buf[i] = ptr->data[i];

            if (plock_img_ != NULL && !plock_img_->Acquire(CS_RD)) return ELOG(charEBUF_Acquire, EBUF_Acquire);
            tfm_imgbuf_size_ -= ptr->size;
            tfm_imgbuf_num_--;
            imgbuf_size = tfm_imgbuf_size_;
            imgbuf_num = tfm_imgbuf_num_;
            if (plock_img_ != NULL) plock_img_->Release();
            if (n_data != NULL) *n_data = ptr->size;
            if (header != NULL) *header = ((uint16)ptr->data[1] << 16) | (uint16)ptr->data[0];

            if (log_acq_number_ && flog_acq_number_ != NULL) fprintf(flog_acq_number_, "%d ", imgbuf_num);
            if (log_acq_size_ && flog_acq_size_ != NULL) fprintf(flog_acq_size_, "%d ", imgbuf_size);
        }
        DelImg(ptr);
    }
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::ReadSwap (void)
{
unsigned int blq1, blq2;
int result = 0;
TBufferImg *img_new = NULL;
bool fin, flag_img_new = false;
unsigned long header, acq_size = 0;
short int *ptr_short = NULL;

    if (acqbuf_size_ < 2) return 0;
    header = ((uint16)buffer[rd_+1] << 16) | (uint16)buffer[rd_];

    switch (header)
    {   
    case TRM_FP_HEADER_v0:
    case TRM_PA_HEADER_v0: break;
    default:
        if (plock != NULL && !plock->Acquire(CS_NonBlocking)) return 0;
        fin = false;
        while(!fin)
        {
            header = ((uint16)buffer[rd_+1] << 16) | (uint16)buffer[rd_]; 
            if (header != (uint32)TRM_FP_HEADER_v0 && header != (uint32)TRM_PA_HEADER_v0)
            {
                rd_+=2;
                acqbuf_size_-=2;
            }
            else fin = true;
            if (acqbuf_size_ <= 0 || rd_ >= acqbuf_capacity_)
            {
                fin = true;
                Clear_HWBuffer();
            }
        }
        if (plock != NULL) plock->Release();
        break;
    }

    ptr_short = (short int*)&acq_size;
    if (rd_ + 4 < acqbuf_capacity_)
    {
        ptr_short[0] = buffer[rd_+4];
        if (rd_ + 6 < acqbuf_capacity_) ptr_short[1] = buffer[rd_+5];
        else ptr_short[1] = buffer[0];
    }
    else
    {
        ptr_short[0] = buffer[4];
        ptr_short[1] = buffer[5];
    }
    if (acq_size * 2 <= acqbuf_size_) 
	{
        if (plock != NULL && !plock->Acquire(CS_NonBlocking)) return 0;
        if ((img_new = CreateAcqImg(acq_size)) != NULL)
        {
            if (rd_ + img_new->size >= acqbuf_capacity_)
            {
                blq1 = acqbuf_capacity_ - rd_;
                if (blq1 > 0) memcpy(img_new->data, &buffer[rd_], sizeof(short int) * blq1);

                rd_ = 0;
                blq2 = img_new->size - blq1;
                if (blq2 > 0) memcpy(img_new->data + blq1, &buffer[rd_], sizeof(short int) * blq2);
                rd_ += blq2;
            }
            else
            {
                memcpy(img_new->data, &buffer[rd_], sizeof(short int) * img_new->size);
                rd_ += img_new->size;
            }

            if ((result = AddAcqImg(img_new, header)) < 0)
            {
                plock->Release();
                return RLOG(result);
            }
            acqbuf_size_ -= img_new->size;
            if (plock != NULL) plock->Release();
            result = 1;

            gb_index_fl_acq++;

            if (log_circular_buffer_ && flog_circular_buffer_ != NULL) fprintf(flog_circular_buffer_, "%d ", acqbuf_size_);
        }
	}
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::ReadSwap_AllBuffer (void)
{
   while (ReadSwap() == 1);
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::ImageSize (vector<unsigned long> &acq_size)
{
   image_size_.assign(acq_size.begin(), acq_size.end());
   image_size_alignment_.assign(acq_size.begin(), acq_size.end());
   Clear_AcquisitionBuffer();
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::PA_DecoDataBuffer (int *id_vch_read)
{
int n_bases, result, i, i_block;
uint16 j, id_vch, n_ascan, n_samples;
unsigned long n_acq_data, n_frame_data, n_parallel_beamformed_lines, data_u32, n_data, header;
int32 data_low, data_high, ibuf;
float hw_gate_reduction = 1;
unsigned char data_u8;
TFP_VCH_ConfigData *cfg = NULL;

   if (id_vch_read != NULL) *id_vch_read = -1;

   if ((result = PA_Read(&n_data, &header)) < 0) return RLOG(result);
   if (n_data == 0) return 0;

   if ((n_bases = STHW_GetHWBASENumber()) <= 0) return ELOG(charEBUF_BASE_Number, EBUF_BASE_Number);

   id_vch = 0;
   ibuf = 0;

   // HEADER
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   if ((uint32)data_u32 != (uint32)TRM_PA_HEADER_v0)
      return ELOG(charEBUF_Frame,EBUF_Frame);

   // ID DEL CANAL VIRTUAL
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   id_vch = (uint16)data_u32;
   if (id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charEBUF_Frame,EBUF_Frame);
   cfg = &gb_virtual_channel[id_vch]->cfg_;

   // NUMERO TOTAL DE DATOS DE LA TRAMA
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   n_frame_data = (data_high << 16) | data_low;
   if (n_frame_data == 0 || n_frame_data > TRM_MAX_ACQ_SIZE)
      return ELOG(charEBUF_Frame,EBUF_Frame);

   gb_virtual_channel[id_vch]->acq_data_.n_frame_data = n_frame_data;

   // NUMERO DE LINEAS CONFORMADAS EN PARALELO POR CADA CONFORMADOR
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;

   n_parallel_beamformed_lines = data_u32;
   gb_virtual_channel[id_vch]->acq_data_.n_parallel_beamformed_lines = n_parallel_beamformed_lines;

   //n_ascan = (unsigned short)cfg->pa_focal_law.beamformer.size();
   switch (cfg->acquisition_type)
   {
   case ACQUISITION_TYPE_PA: n_ascan = (int)cfg->pa_emission_focal_law.size(); break;
   case ACQUISITION_TYPE_TFM: n_ascan = (int)cfg->tfm_emission_focal_law.size(); break;
   default: return ELOG(charEBUF_Frame, EBUF_Frame); break;
   }
   n_samples = gb_virtual_channel[id_vch]->cfg_.acq.n_beamformed_samples;// GET_AScanDataNumber();
   n_acq_data = n_ascan * n_samples;

   // NUMERO DE SEÑALES A-SCAN (Lineas de la imagen PA)
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   if (n_ascan != (uint16)data_u32)
      return ELOG(charEBUF_Frame,EBUF_Frame);
   gb_virtual_channel[id_vch]->acq_data_.n_ascan = n_ascan;

   // NUMERO DE DATOS DE CADA A-SCAN (Línea)
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   gb_virtual_channel[id_vch]->acq_data_.n_samples = (uint16)data_u32;

   // NUMERO DE BLOQUES QUE FORMAN LA IMAGEN [N_BLOQUES]
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   gb_virtual_channel[id_vch]->acq_data_.n_blocks_32b = (uint16)vresult_buf[ibuf];
   ibuf++;

   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   gb_virtual_channel[id_vch]->acq_data_.n_blocks = (uint16)vresult_buf[ibuf];
   ibuf++;

   // NUMERO DE SEÑALES A-SCAN (LINEAS) DE CADA BLOQUE
   gb_virtual_channel[id_vch]->acq_data_.n_lines_block.clear();
   for (i=i_block=0; i<gb_virtual_channel[id_vch]->acq_data_.n_blocks_32b * 2; i++, ibuf++)
   {
      for (j=0; j<2; j++, i_block++)
      {
         data_u8 = (unsigned char)(vresult_buf[ibuf] >> j*8);
         if (i_block < gb_virtual_channel[id_vch]->acq_data_.n_blocks)
            gb_virtual_channel[id_vch]->acq_data_.n_lines_block.push_back(data_u8);
      }
   }

   // CONTADOR DE IMAGENES PA
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   gb_virtual_channel[id_vch]->acq_data_.acq_counter = (data_high << 16) | data_low;
   gb_last_acq_counter = gb_virtual_channel[id_vch]->acq_data_.acq_counter;

   if (log_acq_counter_)
   {
      if (flog_acq_counter_ != NULL)
         fprintf(flog_acq_counter_,"%lu ", gb_last_acq_counter);
      if (id_vch == 0 && flog_acq_counter_0_ != NULL)
         fprintf(flog_acq_counter_0_,"%lu ", gb_last_acq_counter);
      else if (id_vch == 1 && flog_acq_counter_1_ != NULL)
         fprintf(flog_acq_counter_1_,"%lu ", gb_last_acq_counter);
   }

   // OFFSET TRIGGER ENCODER
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;

   // DATOS DE ADQUISICION
   if (n_acq_data != gb_virtual_channel[id_vch]->acq_data_.data.size()) gb_virtual_channel[id_vch]->acq_data_.data.resize(n_acq_data);

   // Image Focal Law
   if (ibuf + n_acq_data > n_data)
   {
       return ELOG(charEBUF_Frame, EBUF_Frame);
   }
   memcpy (&gb_virtual_channel[id_vch]->acq_data_.data[0], &vresult_buf[ibuf], sizeof(short int) * n_acq_data);
   ibuf += n_acq_data;

   // FOOTER
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   if (data_u32 != (uint32)TRM_FOOTER)
      return ELOG(charEBUF_Frame,EBUF_Frame);

   if (id_vch_read != NULL) *id_vch_read = id_vch;
      
   if (ibuf != n_data) 
      {ELOG(charEBUF_DataInconsistency,EBUF_DataInconsistency);}

   return ibuf;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::TFM_DecoDataBuffer(int* id_vch_read)
{
    int n_bases, result, i, i_block;
    uint16 j, id_vch, n_ascan, n_samples;
    unsigned long n_acq_data, n_frame_data, n_parallel_beamformed_lines, data_u32, n_data, header;
    int32 data_low, data_high, ibuf;
    float hw_gate_reduction = 1;
    unsigned char data_u8;
    TFP_VCH_ConfigData* cfg = NULL;

    if (id_vch_read != NULL) *id_vch_read = -1;

    if ((result = TFM_Read(&n_data, &header)) < 0) return RLOG(result);
    if (n_data == 0) return 0;

    if ((n_bases = STHW_GetHWBASENumber()) <= 0) return ELOG(charEBUF_BASE_Number, EBUF_BASE_Number);

    id_vch = 0;
    ibuf = 0;

    // HEADER
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    data_u32 = (data_high << 16) | data_low;
    if ((uint32)data_u32 != (uint32)TRM_PA_HEADER_v0)
        return ELOG(charEBUF_Frame, EBUF_Frame);

    // ID DEL CANAL VIRTUAL
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    data_u32 = (data_high << 16) | data_low;
    id_vch = (uint16)data_u32;
    if (id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charEBUF_Frame, EBUF_Frame);
    cfg = &gb_virtual_channel[id_vch]->cfg_;

    // NUMERO TOTAL DE DATOS DE LA TRAMA
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    n_frame_data = (data_high << 16) | data_low;
    if (n_frame_data == 0 || n_frame_data > TRM_MAX_ACQ_SIZE)
        return ELOG(charEBUF_Frame, EBUF_Frame);

    gb_virtual_channel[id_vch]->acq_data_.n_frame_data = n_frame_data;

    // NUMERO DE LINEAS CONFORMADAS EN PARALELO POR CADA CONFORMADOR
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    data_u32 = (data_high << 16) | data_low;

    n_parallel_beamformed_lines = data_u32;
    gb_virtual_channel[id_vch]->acq_data_.n_parallel_beamformed_lines = n_parallel_beamformed_lines;

    switch (cfg->acquisition_type)
    {
    case ACQUISITION_TYPE_PA: n_ascan = (int)cfg->pa_emission_focal_law.size(); break;
    case ACQUISITION_TYPE_TFM: n_ascan = (int)cfg->tfm_backward.beamformer.size(); break;
    default: return ELOG(charEBUF_Frame, EBUF_Frame); break;
    }
    n_samples = gb_virtual_channel[id_vch]->cfg_.acq.n_beamformed_samples;
    n_acq_data = n_ascan * n_samples;

    // NUMERO DE SEÑALES A-SCAN (Lineas de la imagen PA)
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    data_u32 = (data_high << 16) | data_low;
    if (n_ascan != (uint16)data_u32)
        return ELOG(charEBUF_Frame, EBUF_Frame);
    gb_virtual_channel[id_vch]->acq_data_.n_ascan = n_ascan;

    // NUMERO DE DATOS DE CADA A-SCAN (Línea)
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    data_u32 = (data_high << 16) | data_low;
    gb_virtual_channel[id_vch]->acq_data_.n_samples = (uint16)data_u32;

    // NUMERO DE BLOQUES QUE FORMAN LA IMAGEN [N_BLOQUES]
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    gb_virtual_channel[id_vch]->acq_data_.n_blocks_32b = (uint16)vresult_buf[ibuf];
    ibuf++;

    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    gb_virtual_channel[id_vch]->acq_data_.n_blocks = (uint16)vresult_buf[ibuf];
    ibuf++;

    // NUMERO DE SEÑALES A-SCAN (LINEAS) DE CADA BLOQUE
    gb_virtual_channel[id_vch]->acq_data_.n_lines_block.clear();
    for (i = i_block = 0; i < gb_virtual_channel[id_vch]->acq_data_.n_blocks_32b * 2; i++, ibuf++)
    {
        for (j = 0; j < 2; j++, i_block++)
        {
            data_u8 = (unsigned char)(vresult_buf[ibuf] >> j * 8);
            if (i_block < gb_virtual_channel[id_vch]->acq_data_.n_blocks)
                gb_virtual_channel[id_vch]->acq_data_.n_lines_block.push_back(data_u8);
        }
    }

    // CONTADOR DE IMAGENES PA
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    gb_virtual_channel[id_vch]->acq_data_.acq_counter = (data_high << 16) | data_low;
    gb_last_acq_counter = gb_virtual_channel[id_vch]->acq_data_.acq_counter;

    if (log_acq_counter_)
    {
        if (flog_acq_counter_ != NULL)
            fprintf(flog_acq_counter_, "%lu ", gb_last_acq_counter);
        if (id_vch == 0 && flog_acq_counter_0_ != NULL)
            fprintf(flog_acq_counter_0_, "%lu ", gb_last_acq_counter);
        else if (id_vch == 1 && flog_acq_counter_1_ != NULL)
            fprintf(flog_acq_counter_1_, "%lu ", gb_last_acq_counter);
    }

    // OFFSET TRIGGER ENCODER
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;

    // DATOS DE ADQUISICION
    if (n_acq_data != gb_virtual_channel[id_vch]->acq_data_.data.size()) gb_virtual_channel[id_vch]->acq_data_.data.resize(n_acq_data);

    // Image Focal Law
    if (ibuf + n_acq_data > n_data)
    {
        return ELOG(charEBUF_Frame, EBUF_Frame);
    }
    memcpy(&gb_virtual_channel[id_vch]->acq_data_.data[0], &vresult_buf[ibuf], sizeof(short int) * n_acq_data);
    ibuf += n_acq_data;

    // FOOTER
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_low = (uint16)vresult_buf[ibuf];
    ibuf++;
    if (ibuf >= n_data) return ELOG(charEBUF_Frame, EBUF_Frame);
    data_high = (uint16)vresult_buf[ibuf];
    ibuf++;
    data_u32 = (data_high << 16) | data_low;
    if (data_u32 != (uint32)TRM_FOOTER)
        return ELOG(charEBUF_Frame, EBUF_Frame);

    if (id_vch_read != NULL) *id_vch_read = id_vch;

    if (ibuf != n_data)
    {
        ELOG(charEBUF_DataInconsistency, EBUF_DataInconsistency);
    }
    return ibuf;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferHW::MC_DecoDataBuffer(int *id_vch_read, int *id_fl_read)
{
int n_bases, result;
uint16 id_vch, id_fl, n_ascan, n_samples;
unsigned long n_acq_data, n_frame_data, data_u32, n_data = 0, header;
int32 data_low, data_high, ibuf;
float hw_gate_reduction = 1;

   if (id_vch_read != NULL) *id_vch_read = -1;
   if (id_fl_read != NULL) *id_fl_read = -1;

   if ((result = MC_Read(&n_data, &header)) < 0) return RLOG(result);
   if (n_data == 0) return 0;

   if ((n_bases = STHW_GetHWBASENumber()) <= 0) return ELOG(charEBUF_BASE_Number, EBUF_BASE_Number);

   id_vch = 0;
   ibuf = 0;

   // HEADER
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   if (data_u32 != (uint32)TRM_FP_HEADER_v0)
      return ELOG(charEBUF_Frame,EBUF_Frame);

   // ID DEL CANAL VIRTUAL
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   id_vch = (uint16)data_u32;
   if (id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charEBUF_Frame,EBUF_Frame);

   // NUMERO DE DATOS DE LA ACQUISICION
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   n_frame_data = (data_high << 16) | data_low;
   if (n_frame_data == 0 || n_frame_data > TRM_MAX_ACQ_SIZE)
      return ELOG(charEBUF_Frame,EBUF_Frame);

   gb_virtual_channel[id_vch]->acq_data_.n_frame_data = n_frame_data;

   // ESCRIBE EL ID DE LA LEY FOCAL
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   id_fl = (uint16)data_u32;
   gb_virtual_channel[id_vch]->acq_data_.id_fl = id_fl;

   n_ascan = gb_virtual_channel[id_vch]->GET_MC_FocalLaw_AScanNumber(id_fl);
   n_samples = gb_virtual_channel[id_vch]->GET_FP_AScanDataNumber();
   n_acq_data = n_ascan * n_samples;

   // NUMERO DE SEÑALES A-SCAN (Canales)
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   if (n_ascan != (uint16)data_u32)
      return ELOG(charEBUF_Frame,EBUF_Frame);
   gb_virtual_channel[id_vch]->acq_data_.n_ascan = n_ascan;

   // NUMERO DE DATOS DE CADA ASCAN (Canal)
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   gb_virtual_channel[id_vch]->acq_data_.n_samples = (uint16)data_u32;

   // CONTADOR DE IMAGENES
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   gb_virtual_channel[id_vch]->acq_data_.acq_counter = (data_high << 16) | data_low;
   gb_last_acq_counter = gb_virtual_channel[id_vch]->acq_data_.acq_counter;

   if (log_acq_counter_)
   {
      if (flog_acq_counter_ != NULL)
         fprintf(flog_acq_counter_,"%lu ", gb_last_acq_counter);
      if (id_vch == 0 && flog_acq_counter_0_ != NULL)
         fprintf(flog_acq_counter_0_,"%lu ", gb_last_acq_counter);
      else if (id_vch == 1 && flog_acq_counter_1_ != NULL)
         fprintf(flog_acq_counter_1_,"%lu ", gb_last_acq_counter);
   }

   // OFFSET TRIGGER ENCODER
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;

   // DATOS DE ADQUISICION
   if (n_acq_data != gb_virtual_channel[id_vch]->acq_data_.data.size()) gb_virtual_channel[id_vch]->acq_data_.data.resize(n_acq_data);

   // Image Focal Law
   if (ibuf + n_acq_data > n_data) 
       return ELOG(charEBUF_Frame,EBUF_Frame);
   memcpy (&gb_virtual_channel[id_vch]->acq_data_.data[0], &vresult_buf[ibuf], sizeof(short int) * n_acq_data);
   ibuf += n_acq_data;

   // FOOTER
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_low = (uint16)vresult_buf[ibuf];
   ibuf++;
   if (ibuf >= n_data) return ELOG(charEBUF_Frame,EBUF_Frame);
   data_high = (uint16)vresult_buf[ibuf];
   ibuf++;
   data_u32 = (data_high << 16) | data_low;
   if (data_u32 != (uint32)TRM_FOOTER)
      return ELOG(charEBUF_Frame,EBUF_Frame);

   if (id_vch_read != NULL) *id_vch_read = id_vch;
   if (id_fl_read != NULL) *id_fl_read = id_fl;
      
   if (ibuf != n_data) 
      {ELOG(charEBUF_DataInconsistency,EBUF_DataInconsistency);}

   return ibuf;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferHW::Release (void)
{
   if (plock != NULL) plock->Release();
}

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S
// =============================================================================




