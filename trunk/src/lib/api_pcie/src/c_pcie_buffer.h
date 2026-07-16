#ifndef c_pcie_bufferH
#define c_pcie_bufferH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_tad.h"
#include "c_critical_section.h"

// _____________________________________________________________________________
// ------ D E F I N E S
// =============================================================================

#define CBufferScan_NameSize 100
#define N_BURST_BUFFER	15
#define FILE_LOG_PCI_BUFFER		"LOG_USER_BUFFER.TXT"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================


// _____________________________________________________________________________
// ------ CScanBuffer class
// _____________________________________________________________________________
// =============================================================================
class C_PCIBuffer
{
private:

   AnsiString name_;
   bool sorted_;
   unsigned long fl_size_;
   int n_fl_;
   int n_samples_;
   int n_ascan_;
   uint64 size_;
   uint64 capacity_;
   unsigned long data_size_;
   unsigned long image_size_;
   unsigned long block_size_;
   unsigned long n_images_;
   unsigned long n_images_blocks_;
   unsigned long n_blocks_;
   vector<unsigned short> pin_to_channel_;
   long idx_next_image_wr_;
   long idx_next_image_rd_;
   unsigned long long idx_next_image_rd_bak_;
   long idx_next_image_rd_locked_;
   //bool full_;
   //unsigned long count_image_wr_;
   //unsigned long count_image_rd_;
   unsigned long *buffer_current_image_index_;
   unsigned long *buffer_time_stamp_;
   short int *buffer_images_;
   FILE *f_wr_;
   FILE *f_rd_;
   bool paused_;
   CCriticalSection *mutex_;
   unsigned long long count_images_;
   unsigned long long count_images_available_;
   int moving_average_kernel_size_;
   vector<short> moving_average_image_;
   int last_image_read_;
   int data_logger_;

   short int *GetPtrBufferImage(unsigned long long idx_image);
   //int IncImagesCounter(void);
   //int DecImagesCounter(unsigned long long n_images);
   //int DecImagesAvailableCounter(unsigned long long n_images);

public:

	C_PCIBuffer(unsigned long n_blocks, unsigned long n_images_blocks, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned long data_size, bool sorted, int *error, AnsiString name="");
	~C_PCIBuffer();
	void Print(FILE *f);
	int Clear (void);
	int Resize(unsigned long n_blocks, unsigned long n_images_blocks, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned long data_size, bool sorted);
	int movingAverageSubstractFilter(short *ptr_data);
	int Add(vector<short int *> &focal_laws_pointers, unsigned long current_image_index, unsigned long timestamp);
	short int *GetPtrBufferNextImage(unsigned long *current_image_index, unsigned long *time_stamp, unsigned long timeout_ms);
	short int *GetPtrBufferNextBurst(unsigned long *current_image_index, unsigned long *time_stamp, unsigned long *n_images_buffer, unsigned long *n_images_availables, unsigned long timeout_ms);
	int FreePtrBufferImages(unsigned long long n_images);
	int GetBufferNextImage(short int *data, unsigned long size_data, unsigned long *current_image_index, unsigned long *time_stamp, int timeout_ms);
	unsigned long GetImagesNumber(void) {return n_images_;}
	unsigned long GetImageSize(void) {return image_size_;}
	uint64 GetBufferCapacity(void) { return capacity_; }
	//unsigned long long GetImagesCounter(void) { return count_images_; }
	unsigned long GetDataSize(void) { return data_size_; }
	uint64 GetSize(void) { return size_; }
	unsigned long GetImageCapacity(void) { return n_images_; }
	int SaveFileBuffer(char *file);
	void FileLogEnabled(bool data);
	void FileLog(FILE *f, short int *ptr, unsigned long long id_image_rd_locked, unsigned long long id_image, unsigned long long id_fl, unsigned long size);
	unsigned long long GetFreeImages(void);
	bool GetFreeBlocks(void);
	void RestoreNextImageRD(void) { idx_next_image_rd_ = idx_next_image_rd_bak_; }
	void BackupNextImageRD(void) { idx_next_image_rd_bak_ = idx_next_image_rd_; }
	void MovingAverage_KernelSize(int data) { moving_average_kernel_size_ = data;}
	bool IsFull(void);
	bool IsEmpty(void);
	bool IsFillingRatio(float data);
	int LastImageRead(void) { return IsEmpty() && last_image_read_; }
	void LastImageRead(int data) { last_image_read_ = data == 0 ? 0 : 1; }
	int GetImagesCounter(unsigned long *n_images_available);
	void DataLogger(int data) { data_logger_ = data; }
	int DataLogger(void) { return data_logger_; }
};

// _____________________________________________________________________________
// ------ D A T A   T Y P E S
// =============================================================================

// _____________________________________________________________________________
// ------ 'C_PCIBuffer' Access Functions
// _____________________________________________________________________________
// =============================================================================

C_PCIBuffer *CBufferScan_Resize(C_PCIBuffer *buffer_scan, unsigned long n_blocks, unsigned long n_images_blocks, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned long data_size, bool sorted, int *error, AnsiString name="");

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern C_PCIBuffer *gb_buffer_data;
extern unsigned long gb_buffer_n_images_block;
extern unsigned long gb_buffer_n_blocks;

#endif
