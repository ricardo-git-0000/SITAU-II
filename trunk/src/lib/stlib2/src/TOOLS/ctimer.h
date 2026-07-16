#ifndef ctimerH
#define ctimerH

#if defined(_MSC_VER)
   //#define WIN32_LEAN_AND_MEAN
   #define NOMINMAX
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

//#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define NANOSECONDS     0
#define MICROSECONDS    1
#define MILISECONDS     2

// _____________________________________________________________________________
// ------ CTimer class
// _____________________________________________________________________________
// =============================================================================
class CTimer
{
private:
   LARGE_INTEGER start_;
   LARGE_INTEGER end_;
   clock_t start_clk_;
   clock_t end_clk_;
   double multiplier_;
   bool stop_;
   double elapsed_time_;

public:
	CTimer();
	~CTimer() {};
   void Start (void);
   void Stop (void);
   double ElapsedTime_ns(void);
   double ElapsedTime_us(void);
   double ElapsedTime_ms(void);
};

// _____________________________________________________________________________
// ------ CTimeout class
// _____________________________________________________________________________
// =============================================================================
class CTimeout : public CTimer
{
private:
   double timeout_;
   int unit_;

public:
	CTimeout::CTimeout();
   CTimeout(double timeout, int unit);
   void Start(void);
   int Start(double timeout, int unit);
	~CTimeout() {};
   bool Timeout (void);
};

void Delay (double time, int unit= MILISECONDS);

#endif
