// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_utils.h"
#include "log.h"
#include "pcie_tad.h"
#include "pcie_error_code.h"

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string ChangeFileExtension (string file_path, string extension)
{
int i, idx_dot;
string result = "";
bool flag;

   for (flag=false, i= (int)file_path.length()-1; i>0 && !flag; i--)
   {
      if (file_path[i] == '.') {flag = true; i++;}
      else if (file_path[i] == '\\') i = 0;
   }
   idx_dot = i;
   if (idx_dot > 0)
   {
      for (i=1; i<idx_dot && i< file_path.length(); i++)
         result.insert(result.length(), i, file_path[i]);
      result += extension;
   }
   else result = file_path + extension;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string ToUpper (string data)
{
int i;
string result = data;

	for (i=0; i<data.size(); i++)
	   result[i] = toupper(result[i]);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float dB2Linear (float db)
{
   return (float)pow((float)10., db/(float)20.);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float Linear2dB (float linear)
{
   return (float)20. * (float)log10(linear);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned int Log2 (unsigned int data)
{
unsigned int result = 0;

   while (data >>= 1) result++;
   //result++;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int Round32(double data)
{
	int result, sgn = 1;

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5)
		result = (int)((data + 1) * sgn);
	else
		result = (int)(data * sgn);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int Round32 (float data)
{
int result, sgn = 1;

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5 )
		result = (int)((data + 1) * sgn);
	else
		result = (int)(data * sgn);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long Round32u (float data)
{
	if (data < 0) return 0;
	if (data - (int)data > 0.5 )
		return (unsigned long)(data + 1);
	else
		return (unsigned long)data;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long long Round64u(float data)
{
	if (data < 0) return 0;
	if (data - (int)data > 0.5)
		return (unsigned long long)(data + 1);
	else
		return (unsigned long long)data;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
short int Round16 (float data)
{
int data_int, sgn = 1;
short int result;
int min_value = numeric_limits<short int>::min();
int max_value = numeric_limits<short int>::max();

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5 )
		data_int = (int)((data + 1) * sgn);
	else
		data_int = (int)(data * sgn);
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (short int)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short Round16u (float data)
{
int data_int;
unsigned short result;
int max_value = numeric_limits<unsigned short>::max();

	if (data < 0) return 0;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1);
	else
		data_int = (int)data;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned short)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short Round16u_d (double data)
{
int data_int;
unsigned short result;
int max_value = numeric_limits<unsigned short>::max();

	if (data < 0) return 0;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1);
	else
		data_int = (int)data;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned short)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned char Round8u (double data)
{
int data_int, sgn = 1;
unsigned char result;
unsigned char min_value = numeric_limits<unsigned char>::min();
unsigned char max_value = numeric_limits<unsigned char>::max();

	if (data < 0) sgn = -1;
	data *= sgn;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1) * sgn;
	else
		data_int = (int)data * sgn;
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned char)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned char Round8u (float data)
{
int data_int, sgn = 1;
unsigned char result;
unsigned char min_value = numeric_limits<unsigned char>::min();
unsigned char max_value = numeric_limits<unsigned char>::max();

	if (data < 0) sgn = -1;
	data *= sgn;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1) * sgn;
	else
		data_int = (int)data * sgn;
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned char)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
char Round8 (double data)
{
int data_int, sgn = 1;
char result;
char min_value = numeric_limits<char>::min();
char max_value = numeric_limits<char>::max();

	if (data < 0) sgn = -1;
	data *= sgn;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1) * sgn;
	else
		data_int = (int)data * sgn;
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (char)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int RoundUp32 (float f)
{
	if (f-(int)f > 0)
		return (int)(f+1);
	else
		return (int)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short RoundUp16u (float f)
{
	if (f-(int)f > 0)
		return (unsigned short)(f+1);
	else
		return (unsigned short)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int RoundDown (float f)
{
	if (f < 0)
	{
		if (f-(int)f > 0)
			return (int)f;
		else
			return (int)(f-1);
	}
	else
		return (int)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int GetImageSorted(short int *src_data, unsigned long n_data, short int *data, unsigned long size_fl, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned short *pin_to_channel)
{
	int result = EPCIE_NONE;
	unsigned long i_fl, i_data;

	if (src_data == nullptr) return ELOG(charEPCIE_PARAMETER, EPCIE_PARAMETER);
	if (data == nullptr) return ELOG(charEPCIE_PARAMETER, EPCIE_PARAMETER);
	for (i_data = i_fl = 0; i_fl < n_fl && i_data < n_data; i_fl++, i_data += size_fl)
	{
		if ((result = GetFocalLawSorted(src_data + i_data, size_fl, data + i_data, n_ascan, n_samples, pin_to_channel)) < 0) return RLOG(result);
	}
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int GetFocalLawSorted(short int *ptr_focal_law, unsigned long size_fl, short int *data, unsigned long n_ascan, unsigned long n_samples, unsigned short *pin_to_channel)
{
	int result = EPCIE_NONE;
	unsigned long i_data, i_fl_data, i_ascan, i_ascan_assign, i_sample, i_line_block;

	if (ptr_focal_law == nullptr) return ELOG(charEPCIE_PARAMETER, EPCIE_PARAMETER);
	if (data == nullptr) return ELOG(charEPCIE_PARAMETER, EPCIE_PARAMETER);
	for (i_data = i_ascan = 0; i_ascan < n_ascan; i_ascan++)
	{
		if (pin_to_channel != nullptr) i_ascan_assign = *(pin_to_channel + i_ascan);
		else i_ascan_assign = i_ascan;
		i_ascan_assign = i_ascan_assign < n_ascan ? i_ascan_assign : n_ascan - 1;

		for (i_sample = 0; i_sample < n_samples; i_sample++, i_data++)
		{
			i_line_block = i_ascan_assign / N_CHANNELS_MODULE;
			if (n_ascan > N_CHANNELS_MODULE) i_fl_data = i_sample * N_CHANNELS_MODULE + i_ascan_assign + i_line_block * N_CHANNELS_MODULE * (n_samples - 1);
			else i_fl_data = i_sample * n_ascan + i_ascan_assign;
			if (i_fl_data >= size_fl) i_fl_data = size_fl - 1;
			*(data + i_data) = *((short int*)ptr_focal_law + i_fl_data);
		}
	}
	return result;
}
