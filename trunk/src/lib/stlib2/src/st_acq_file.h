#ifndef st_acq_fileH
#define st_acq_fileH

#define BINFILE_VERSION     "0000.0000.0000.0001"
#define XML_FILE_TMP		"tmp.xml"

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

using namespace std;

// ______________________________________________________________________
// ------ T I P O S   D E   D A T O S
// ======================================================================

typedef struct T_FilePositionGate {
	__int64 file_gate_amplitude;
	__int64 file_gate_position;
	__int64 file_gate_alarm;
	__int64 file_gate_start_sample;
} T_FilePositionGate;

typedef struct T_FilePositionEncoder {
	__int64 file_encoder_value;
	__int64 file_encoder_index;
} T_FilePositionEncoder;

typedef struct T_FilePositionData {
	__int64 file_raw_data;
	__int64 file_dsp_data;
	vector<T_FilePositionGate> file_sw_gate;
	vector<T_FilePositionGate> file_hw_gate;
	__int64 file_counter;
	vector<T_FilePositionEncoder> file_encoder;
	__int64 file_sw_timestamp;
	__int64 file_timestamp;
	__int64 file_acq_counter;
	__int64 file_trg_counter;
	__int64 file_trigger_error;
	__int64 file_memory_free;
	__int64 file_data_lost;
} T_FilePositionData;

typedef struct T_FileAcquisition {
	vector<T_FilePositionData> position_data;
	vector<char> config;
} T_FileAcquisition;

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern unsigned int gb_progress_file_acquisition;
extern char gb_msg_progress_file_acquisition[255];

extern unsigned int gb_progress_trigger_update_scan_data;
extern unsigned int gb_progress_auxiliar_update_scan_data;
extern char gb_msg_progress_update_scan_data[255];

#endif
