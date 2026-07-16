#ifndef st_acq_file_v0H
#define st_acq_file_v0H

#define BINFILE_VERSION_v0     "0000.0000.0000.0000"
#define XML_FILE_TMP_v0		"tmp.xml"

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

using namespace std;

// ______________________________________________________________________
// ------ T I P O S   D E   D A T O S
// ======================================================================

typedef struct T_FilePositionGate_v0 {
	__int64 file_gate_amplitude;
	__int64 file_gate_position;
	__int64 file_gate_alarm;
	__int64 file_gate_start_sample;
} T_FilePositionGate_v0;

typedef struct T_FilePositionEncoder_v0 {
	__int64 file_encoder_value;
	__int64 file_encoder_index;
} T_FilePositionEncoder_v0;

typedef struct T_FilePositionData_v0 {
	vector<__int64> file_raw_data;
	__int64 file_dsp_data;
	vector<T_FilePositionGate_v0> file_sw_gate;
	vector<T_FilePositionGate_v0> file_hw_gate;
	__int64 file_counter;
	vector<T_FilePositionEncoder_v0> file_encoder;
	__int64 file_sw_timestamp;
	__int64 file_timestamp;
	__int64 file_acq_counter;
	__int64 file_trg_counter;
	__int64 file_trigger_error;
	__int64 file_memory_free;
	__int64 file_data_lost;
} T_FilePositionData_v0;

typedef struct T_FileAcquisition_v0 {
	vector<T_FilePositionData_v0> position_data;
	vector<char> config;
} T_FileAcquisition_v0;

int ST_LoadFileAcquisitionData_v0(fstream& f, T_FileAcquisition_v0 *file_data);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

#endif
