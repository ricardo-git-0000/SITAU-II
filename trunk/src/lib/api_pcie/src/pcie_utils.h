#ifndef pcie_utilsH
#define pcie_utilsH

#include <vector>

using namespace std;

string ChangeFileExtension (string file_path, string extension);
string ToUpper (string str);
float dB2Linear (float db);
float Linear2dB (float linear);
unsigned int Log2 (unsigned int data);
int Round32(double data);
int Round32 (float data);
unsigned long Round32u (float data);
unsigned long long Round64u(float data);
short int Round16 (float data);
unsigned short Round16u (float data);
unsigned short Round16u_d (double data);
unsigned char Round8u (double data);
unsigned char Round8u (float data);
char Round8 (double data);
int RoundUp32 (float f);
unsigned short RoundUp16u (float f);
int RoundDown (float f);
int GetFocalLawSorted(short int *ptr_focal_law, unsigned long size_fl, short int *data, unsigned long n_ascan, unsigned long n_samples, unsigned short *pin_to_channel);
int GetImageSorted(short int *src_data, unsigned long n_data, short int *data, unsigned long size_fl, unsigned long n_fl, unsigned long n_ascan, unsigned long n_samples, unsigned short *pin_to_channel);

#endif
