#include "ArcnetAnalyzer.h"
#include "ArcnetAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#define FRM_SD_LEN	6
#define FRM_RSU_LEN	9
#define FRM_ISU_LEN	11
#define RECONF_FRMS	765


ArcnetAnalyzer::ArcnetAnalyzer()
:	Analyzer2(),  
	mSettings( new ArcnetAnalyzerSettings() ),
	mSimulationInitilized( false ),
	mState(State::WAIT)
{
	SetAnalyzerSettings( mSettings.get() );
}

ArcnetAnalyzer::~ArcnetAnalyzer()
{
	KillThread();
}

void ArcnetAnalyzer::SetupResults()
{
	mResults.reset( new ArcnetAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void ArcnetAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData(mSettings->mInputChannel);
	
	ClockGenerator clock;
	clock.Init(mSettings->mBitRate*2, mSampleRateHz);

	auto H = mSettings->mInverted? BIT_LOW: BIT_HIGH;
	auto L = mSettings->mInverted? BIT_HIGH: BIT_LOW;

	while (true)
	{
		if (mSerial->GetBitState() == L)
		{
			mSerial->AdvanceToNextEdge();
			mSerial->Advance(clock.AdvanceByHalfPeriod());
		}

		bool endedFrame = false;
		bool sendFrame = false;
		U64 data = 0;
		U8 type = 0;
		U64 starting_sample = mSerial->GetSampleNumber();
		bool reconf = false;


		for (int f = 0; !endedFrame; ++f)
		{
			switch (mState)
			{
			case State::WAIT:

				if (mSerial->GetBitState() == H)
				{
					if (f == FRM_SD_LEN-1)
					{
						type = SD;
						sendFrame = true;
						endedFrame = true;

						mState = State::PACKET;
					}
				}
				else endedFrame = true;
				break;

			case State::PACKET:
				if (f <= 2)
				{
					if (reconf && f == 0 && mSerial->GetBitState() == BIT_LOW)
					{
						type = RSU;
						sendFrame = true;
						endedFrame = true;

						mState = State::RECONF;
					}
					else if (not(
						mSerial->GetBitState() == H && f <= 1 ||
						mSerial->GetBitState() == L && f == 2
					))
					{
						endedFrame = true;

						mState = State::WAIT;
					}

				}
				else
				{
					if (mSerial->GetBitState() == H)
						data |= 1 << (f-3);

					if (f == FRM_ISU_LEN-1)
					{
						type = ISU;
						sendFrame = true;
						endedFrame = true;

						reconf = data == 255;
					}
				}
				break;

			case State::RECONF:
				if (f == FRM_RSU_LEN-1)
				{
					if (mSerial->GetBitState() == L)
					{
						type = RSU;
						sendFrame = true;
						endedFrame = true;
					}
					else
					{
						endedFrame = false;

						mState = State::WAIT;
					}
				}
				else if (mSerial->GetBitState() == L)
				{
					endedFrame = true;

					mState = State::WAIT;
				}
				break;
			}
			mResults->AddMarker( mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
			mSerial->Advance(clock.AdvanceByHalfPeriod(4));
		}

		if (sendFrame)
		{
			Frame frame;
			frame.mData1 = data;
			frame.mData2 = 0;
			frame.mType = type;
			frame.mFlags = 0;
			frame.mStartingSampleInclusive = starting_sample;
			frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

			mResults->AddFrame(frame);
			mResults->CommitResults();
			ReportProgress(frame.mEndingSampleInclusive);
		}
	}
}

bool ArcnetAnalyzer::NeedsRerun()
{
	return false;
}

U32 ArcnetAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 ArcnetAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* ArcnetAnalyzer::GetAnalyzerName() const
{
	return "Arcnet Analyzer";
}

const char* GetAnalyzerName()
{
	return "Arcnet Analyzer";
}

Analyzer* CreateAnalyzer()
{
	return new ArcnetAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}