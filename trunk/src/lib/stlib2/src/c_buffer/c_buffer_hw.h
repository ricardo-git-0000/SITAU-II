//---------------------------------------------------------------------------

#ifndef fp_c_buffer_hwH
#define fp_c_buffer_hwH

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


#define TRM_FP_HEADER_v0 	   0x0000CEB0
#define TRM_PA_HEADER_v0 	   0x0001CEB0

#define TRM_FOOTER            0x0000C0C0
#define TRM_SIZE_BSCAN_HEADER 3
#define TRM_SIZE_ASCAN_HEADER 0
#define TRM_N_GATES           4
#define TRM_N_ENCODERS        4
#define TRM_MAX_N_SAMPLES     20000
#define TRM_MAX_ACQ_SIZE      5100000
#define TRM_MAX_N_LINES       256

typedef struct TBufferImg
{
   TBufferImg *sig;
   unsigned long size;
   short int *data;
   unsigned long size32;
} TBufferImg;

class CBufferHW
{
private:

   unsigned long acquired_images_;
   unsigned long acqbuf_size_;
   unsigned long wr_;
   unsigned long rd_;
   unsigned long acqbuf_capacity_;
   unsigned long n_samples_;
   vector<unsigned long> image_size_;
   vector<unsigned long> image_size_alignment_;

   short int *buffer;
   CCriticalSection *plock;

   TBufferImg *mc_wr_img_;
   TBufferImg *mc_rd_img_;
   unsigned long mc_imgbuf_size_;
   unsigned long mc_imgbuf_num_;

   TBufferImg *pa_wr_img_;
   TBufferImg *pa_rd_img_;
   unsigned long pa_imgbuf_size_;
   unsigned long pa_imgbuf_num_;

   TBufferImg* tfm_wr_img_;
   TBufferImg* tfm_rd_img_;
   unsigned long tfm_imgbuf_size_;
   unsigned long tfm_imgbuf_num_;

   CCriticalSection *plock_img_;

   bool log_circular_buffer_;
   bool log_acq_counter_;
   bool log_acq_size_;
   bool log_acq_number_;
   FILE *flog_circular_buffer_;
	FILE *flog_acq_counter_;
	FILE *flog_acq_counter_0_;
	FILE *flog_acq_counter_1_;
	FILE *flog_acq_size_;
   FILE *flog_acq_number_;
   AnsiString logname_circular_buffer_;
   AnsiString logname_acq_counter_;
   AnsiString logname_acq_counter_0_;
   AnsiString logname_acq_counter_1_;
   AnsiString logname_acq_size_;
   AnsiString logname_acq_number_;

public:

   CBufferHW (unsigned long size);
   ~CBufferHW();
   unsigned long Print(FILE *f);
   int Add (short *trama, unsigned int ndat);
   int Clear_HWBuffer (void);
   int Clear_AcquisitionBuffer (void);
   int MC_Read(unsigned long *n_data, unsigned long *header);
   int PA_Read (unsigned long *n_data, unsigned long *header);
   int TFM_Read(unsigned long* n_data, unsigned long* header);
   int FP_AcquisitionNumber (void);
   int PA_AcquisitionNumber (void);
   int TFM_AcquisitionNumber(void);
   bool FP_AcquisitionData (void);
   bool PA_AcquisitionData (void);
   bool TFM_AcquisitionData(void);
   int ReadSwap (void);
   int ReadSwap_AllBuffer (void);
   unsigned long FP_NumDatos (void) {return mc_imgbuf_size_;}
   unsigned long PA_NumDatos (void) {return pa_imgbuf_size_;}
   unsigned long TFM_NumDatos(void) { return tfm_imgbuf_size_; }
   int ImageSize (vector<unsigned long> &acq_size);
   void DBG_FileOpen (void);
   void DBG_FileClose (void);
   void DelImg(TBufferImg* img);
   void DelBufferImg (TBufferImg *img);
   TBufferImg *CreateAcqImg (unsigned long acq_size);
   int AddAcqImg (TBufferImg *img_new, unsigned long header);
   TBufferImg *MC_GetAcqImg (void);
   TBufferImg *PA_GetAcqImg (void);
   TBufferImg* TFM_GetAcqImg(void);
   unsigned long MC_GetAcqImgSize (void);
   unsigned long PA_GetAcqImgSize (void);
   unsigned long TFM_GetAcqImgSize(void);
   int MC_DecoDataBuffer(int *id_vch_read, int *id_fl_read);
   int PA_DecoDataBuffer (int *id_vch_read);
   int TFM_DecoDataBuffer(int* id_vch_read);
   unsigned long FP_AcquisitionDataSize (void) {return mc_imgbuf_size_ * 2;}   // Bytes
   unsigned long PA_AcquisitionDataSize (void) {return pa_imgbuf_size_ * 2;}   // Bytes
   unsigned long TFM_AcquisitionDataSize(void) { return tfm_imgbuf_size_ * 2; }   // Bytes

   unsigned long AcquisitionBufferSize (void) {return acqbuf_size_ * 2;} // Bytes

   unsigned long AcquiredImages(void) { return acquired_images_; } // Bytes
   void IncreaseAcquiredImages(void) { acquired_images_++; } // Bytes
   unsigned long Capacity(void) {return acqbuf_capacity_;}
   unsigned long GetMemoryUsed_BufferDifraScope(void) {return acqbuf_size_;}
   unsigned long FP_GetMemoryUsed_BufferAcquisitions(void) {return mc_imgbuf_size_;}
   unsigned long MC_BufferAcquisitionsNumber(void) {return mc_imgbuf_num_;}
   unsigned long PA_GetMemoryUsed_BufferAcquisitions(void) {return pa_imgbuf_size_;}
   unsigned long PA_BufferAcquisitionsNumber(void) {return pa_imgbuf_num_;}
   unsigned long TFM_GetMemoryUsed_BufferAcquisitions(void) { return tfm_imgbuf_size_; }
   unsigned long TFM_BufferAcquisitionsNumber(void) { return tfm_imgbuf_num_; }

   void SelectLogData_HWCircularBuffer (bool data) {log_circular_buffer_ = data;}
   void SelectLogData_AcqBufferCounter (bool data) {log_acq_counter_ = data;}
   void SelectLogData_AcqBufferSize (bool data) {log_acq_size_ = data;}
   void SelectLogData_AcqBufferNumber (bool data) {log_acq_number_ = data;}
   bool SelectLogData_HWCircularBuffer (void) {return log_circular_buffer_;}
   bool SelectLogData_AcqBufferCounter (void) {return log_acq_counter_;}
   bool SelectLogData_AcqBufferSize (void) {return log_acq_size_;}
   bool SelectLogData_AcqBufferNumber (void) {return log_acq_number_;}
   void Release (void);
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern CBufferHW *gb_buffer_hw;
int ST_EnsureHWBuffer(void); // Devuelve -703 si no se puede construir el buffer, sin cerrar el proceso anfitrion.
extern vector<short int> vresult_buf;
extern vector<long> gb_buffer_frame;

#endif
