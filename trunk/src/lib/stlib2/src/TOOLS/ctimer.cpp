// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "ctimer.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ CTimer class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CTimer::CTimer()
{
LARGE_INTEGER frequency;

   if (!QueryPerformanceFrequency(&frequency))
      throw ELOG("CTimer Error: Hardware does not support a high-resolution performance counter",-1);
   stop_ = true;
   elapsed_time_ = 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CTimer::Start(void)
{
   QueryPerformanceCounter(&start_);
   start_clk_ = clock();
//   time(&start_time_);
   stop_ = false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double CTimer::ElapsedTime_ms(void)
{
LARGE_INTEGER frequency;
double result, result_clk;//, result_time;

   end_clk_ = clock();
//   time(&end_time_);
   QueryPerformanceCounter(&end_);
   if (!QueryPerformanceFrequency(&frequency))
      throw ELOG("CTimer Error: Hardware does not support a high-resolution performance counter",-1);
   result = double(end_.QuadPart - start_.QuadPart) * (double)1000. / (double)frequency.QuadPart;
   result_clk = (double)(end_clk_ - start_clk_) * (double)1000. / (double)CLK_TCK;
//   result_time = (double)difftime(end_time_, start_time_) * (double)1000.;
//   result = result_query;
   if (result_clk < result)
      result = result_clk;
//   if (result_time > 0 && result_time < result)
//      result = result_time;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CTimer::Stop(void)
{
   elapsed_time_ = ElapsedTime_ms();
//LARGE_INTEGER frequency;
//
//   QueryPerformanceCounter(&end_);
//   if (!QueryPerformanceFrequency(&frequency))
//      throw ELOG("CTimer Error: Hardware does not support a high-resolution performance counter",-1);
//   elapsed_time_ = double(end_.QuadPart - start_.QuadPart) * (double)1000000000. / (double)frequency.QuadPart;
   stop_ = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double CTimer::ElapsedTime_ns(void)
{
//LARGE_INTEGER frequency;

   if (stop_) return elapsed_time_ * (double)1000000.;
   else return ElapsedTime_ms() * (double)1000000.;
//   {
//      QueryPerformanceCounter(&end_);
//      if (!QueryPerformanceFrequency(&frequency))
//         throw ELOG("CTimer Error: Hardware does not support a high-resolution performance counter",-1);
//      return double(end_.QuadPart - start_.QuadPart) * (double)1000000000. / (double)frequency.QuadPart;
//   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double CTimer::ElapsedTime_us(void)
{
//LARGE_INTEGER frequency;

   if (stop_) return elapsed_time_ * (double)1000.;
   else return ElapsedTime_ms() * (double)1000.;
//   {
//      QueryPerformanceCounter(&end_);
//      if (!QueryPerformanceFrequency(&frequency))
//         throw ELOG("CTimer Error: Hardware does not support a high-resolution performance counter",-1);
//      return double(end_.QuadPart - start_.QuadPart) * (double)1000000. / (double)frequency.QuadPart;
//   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//double CTimer::ElapsedTime_ms(void)
//{
//LARGE_INTEGER frequency;
//
//   if (stop_) return elapsed_time_ / 1000000.;
//   else
//   {
//      QueryPerformanceCounter(&end_);
//      if (!QueryPerformanceFrequency(&frequency))
//         throw ELOG("CTimer Error: Hardware does not support a high-resolution performance counter",-1);
//      return double(end_.QuadPart - start_.QuadPart) * (double)1000. / (double)frequency.QuadPart;
//   }
//}


// _____________________________________________________________________________
// ------ CTimeout class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CTimeout::CTimeout()
   : CTimer()
{
   timeout_ = 1;
   unit_ = MILISECONDS;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CTimeout::CTimeout(double timeout, int unit)
   : CTimer()
{
   if (timeout <= 0)
      throw ELOG("CTimeout Error: timeout value is out of range",-1);
   if (unit < 0 || unit > 2)
      throw ELOG("CTimeout Error: unit value is out of range",-1);
   timeout_ = timeout;
   unit_ = unit;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CTimeout::Start(void)
{
   CTimer::Start();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CTimeout::Start(double timeout, int unit)
{
   if (timeout <= 0)
      return ELOG("CTimeout Error: timeout value is out of range",-1);
   if (unit < 0 || unit > 2)
      return ELOG("CTimeout Error: unit value is out of range",-1);
   timeout_ = timeout;
   unit_ = unit;
   CTimer::Start();
   return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CTimeout::Timeout(void)
{
bool result = false;

   switch(unit_)
   {
   case NANOSECONDS:
      if (ElapsedTime_ns() >= timeout_)
         result = true;
      break;
   case MICROSECONDS:
      if (ElapsedTime_us() >= timeout_)
         result = true;
      break;
   case MILISECONDS:
      if (ElapsedTime_ms() >= timeout_)
         result = true;
      break;
   }
   return result;
}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Delay (double time, int unit)
{
CTimeout *timeout = new CTimeout(time, unit);
//bool fin = false;
//int i;

   timeout->Start();
//   for (i=0; !fin; i++)
//   {
//      fin = timeout->Timeout();
//   }
   while (!timeout->Timeout());
   delete timeout;
}

