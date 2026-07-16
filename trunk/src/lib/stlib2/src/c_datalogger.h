#ifndef c_dataloggerH
#define c_dataloggerH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   T I P O S
// =============================================================================

// _____________________________________________________________________________
// ------ C_DataLogger class
// _____________________________________________________________________________
// =============================================================================
class C_DataLogger
{
private:

	bool enabled_;
	int config_;
	int pcie_user_buffer_;
	int pa_beamformer_;
	int tfm_beamformer_;

public:
   
	C_DataLogger();
  
	~C_DataLogger() {};

	int Update(void);

	bool Enabled(void) { return enabled_; }

	void Enabled(bool data) { enabled_ = data; }

	void Config(int data) { config_ = data; }

	int Config(void) { return config_; }

	void PABeamformer(int data) { pa_beamformer_ = data; }

	int PABeamformer(void) { return pa_beamformer_; }

	void TFMBeamformer(int data) { tfm_beamformer_ = data; }

	int TFMBeamformer(void) { return tfm_beamformer_; }
};

int STDL_Create(void);

int STDL_Delete(void);

int STDL_GetEnabled(void);

int STDL_SetEnabled(int data);

int STDL_GetConfig(void);

int STDL_SetConfig(int data);

int STDL_GetPABeamformer(void);

int STDL_SetPABeamformer(int data);

int STDL_GetTFMBeamformer(void);

int STDL_SetTFMBeamformer(int data);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern C_DataLogger *gb_datalogger;

#endif
