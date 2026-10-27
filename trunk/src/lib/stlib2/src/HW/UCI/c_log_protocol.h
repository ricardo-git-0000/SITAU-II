//---------------------------------------------------------------------------
#ifndef c_log_protocolH
#define c_log_protocolH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "protocol_uci.h"

class CLOG_PROTOCOL
{
public:

   CLOG_PROTOCOL();
	~CLOG_PROTOCOL();
	void LastMessage(unsigned char data);
	void LastMessage(TFP_MSG_SITAUConfig *data);
	void LastMessage(unsigned short data);
	void LastMessage(TMSG_TriggerSource* data);
	void LastMessage(TMSG_StatusConfig* data);
	void LastMessage(TMSG_Config* data);
	void LastMessage(TMSG_FIR* data);
	void LastMessage(TMSG_EmissionFocalLaws* data);
	void LastMessage(TMSG_EmissionFocalLawsExtended* data);
	void LastMessage(TMSG_Beamformer* data);
	void LastMessage(TMSG_BeamformerExtended* data);
	void LastMessage(TMSG_TGC* data);
	void LastMessage(TFP_MSG_PASystemConfig* data);
	void Print(char* filename);

private:

	bool sent_MSG_PulseAmplitude;
	bool sent_MSG_ACQUIRE;
	bool sent_MSG_SITAUConfig;
	bool sent_MSG_StatusConfig;
	bool sent_MSG_TriggerSource;
	bool sent_MSG_VCH_Config;
	bool sent_MSG_VCH_FIR;
	bool sent_MSG_VCH_EmissionFocallaws;
	bool sent_MSG_VCH_EmissionFocallawsExtended;
	bool sent_MSG_VirtualChannel_Beamformer;
	bool sent_MSG_VirtualChannel_BeamformerExtended;
	bool sent_MSG_VirtualChannel_TGC;
	bool sent_MSG_PASystemConfig;
	bool sent_MSG_TERMINAL_LOG;

	unsigned char last_MSG_PulseAmplitude;
	unsigned short last_MSG_ACQUIRE;
	TFP_MSG_SITAUConfig last_MSG_SITAUConfig;
	TMSG_StatusConfig last_MSG_StatusConfig;
	TMSG_TriggerSource last_MSG_TriggerSource;
	TMSG_Config last_MSG_VCH_Config;
	TMSG_FIR last_MSG_VCH_FIR;
	TMSG_EmissionFocalLaws last_MSG_VCH_EmissionFocallaws;
	TMSG_EmissionFocalLawsExtended last_MSG_VCH_EmissionFocallawsExtended;
	TMSG_Beamformer last_MSG_VirtualChannel_Beamformer;
	TMSG_BeamformerExtended last_MSG_VirtualChannel_BeamformerExtended;
	TMSG_TGC last_MSG_VirtualChannel_TGC;
	TFP_MSG_PASystemConfig last_MSG_PASystemConfig;
	unsigned short last_MSG_TERMINAL_LOG;
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern CLOG_PROTOCOL* log_protocol;

#endif
