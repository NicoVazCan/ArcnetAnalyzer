#ifndef ARCNET_ANALYZER_H
#define ARCNET_ANALYZER_H

#include <Analyzer.h>
#include <AnalyzerHelpers.h>
#include "ArcnetAnalyzerResults.h"
#include "ArcnetSimulationDataGenerator.h"

#define SD 1
#define RSU 2
#define ISU 3

class ArcnetAnalyzerSettings;

enum State
{
	WAIT, PACKET, RECONF 
};

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

	State mState;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //ARCNET_ANALYZER_H
