#ifndef c_timeoutH
#define c_timeoutH

#define NOMINMAX

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <windows.h>
#include <time.h>

// _____________________________________________________________________________
// ------ C_Timeout class
// _____________________________________________________________________________
// =============================================================================
class C_Timeout
{
private:
   double timeout_;
   clock_t start_clk_;

public:
   C_Timeout(double timeout) { timeout_ = timeout <= 0 ? 1 : timeout; };
   ~C_Timeout() {};
   void Start(void) { start_clk_ = clock(); };
   bool Timeout(void) { return (((double)(clock() - start_clk_) * (double)1000. / (double)CLK_TCK) >= timeout_); };
};

#endif
