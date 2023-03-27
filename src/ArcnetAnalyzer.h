#ifndef ARCNET_ANALYZER_H
#define ARCNET_ANALYZER_H

#include <Analyzer.h>
#include <AnalyzerHelpers.h>
#include "ArcnetAnalyzerResults.h"
#include "ArcnetSimulationDataGenerator.h"


enum FrmFormat
{
	WAIT, BASIC, RECON 
};

enum BasicFrm
{
	BFN, ITT, FBE, PAC 
};

enum BasicSimbUnit
{
	BSN, SD, RSU, ISU, SOH, ENQ, ACK, NAK, EOT, NID, SID, DID, CP, SC, DATA, FCS
};

enum FrmFlag
{
	OK, ERROR
};

class ArcnetAnalyzerSettings;

class ANALYZER_EXPORT ArcnetAnalyzer : public Analyzer2
{
public:
	ArcnetAnalyzer();
	virtual ~ArcnetAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< ArcnetAnalyzerSettings > mSettings;
	std::auto_ptr< ArcnetAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	ArcnetSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //ARCNET_ANALYZER_H
