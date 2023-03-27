#include "ArcnetAnalyzer.h"
#include "ArcnetAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#define FRM_SD_LEN	6
#define FRM_RSU_LEN	9
#define FRM_ISU_LEN	11
#define RECON_FRMS	765

#define FRM_SOH_D	0x01
#define FRM_ENQ_D	0x85
#define FRM_ACK_D	0x86
#define FRM_NAK_D	0x15
#define FRM_EOT_D	0x04


ArcnetAnalyzer::ArcnetAnalyzer()
:	Analyzer2(),  
	mSettings( new ArcnetAnalyzerSettings() ),
	mSimulationInitilized( false )
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
	FrmFormat frmFormat = WAIT;
	BasicFrm basicFrm = BFN;
	BasicSimbUnit basicSimbUnit = BSN;
	FrmFlag flag = OK;
	int f = 0, df = 0, cp = 0, fcs = 0;

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
		U64 data = 0;
		U64 starting_sample = mSerial->GetSampleNumber();
		bool reconf = false;


		// Extract Basic Frames

		for (int b = 0; !endedFrame; ++b)
		{
			switch (frmFormat)
			{
			case WAIT:
				if (mSerial->GetBitState() == H)
				{
					if (b == FRM_SD_LEN-1)
					{
						flag = OK;
						basicSimbUnit = SD;
						endedFrame = true;

						frmFormat = BASIC;
					}
				}
				else
				{
					basicFrm = BFN;
					basicSimbUnit = BSN;
					flag = ERROR;
					f = 0;
					endedFrame = true;
				}
				break;

			case BASIC:
				if (b <= 2)
				{
					if (reconf && b == 0 && mSerial->GetBitState() == BIT_LOW)
					{
						flag = OK;
						basicSimbUnit = RSU;
						endedFrame = true;

						frmFormat = RECON;
					}
					else if (not(
						mSerial->GetBitState() == H && b <= 1 ||
						mSerial->GetBitState() == L && b == 2
					))
					{
						basicFrm = BFN;
						basicSimbUnit = BSN;
						flag = ERROR;
						f = 0;
						endedFrame = true;

						frmFormat = WAIT;
					}

				}
				else
				{
					if (mSerial->GetBitState() == H)
						data |= 1 << (b-3);

					if (b == FRM_ISU_LEN-1)
					{
						flag = OK;
						basicSimbUnit = ISU;
						endedFrame = true;

						reconf = data == 255;
					}
				}
				break;

			case RECON:
				if (b == FRM_RSU_LEN-1)
				{
					if (mSerial->GetBitState() == L)
					{
						flag = OK;
						basicSimbUnit = RSU;
						endedFrame = true;
					}
					else
					{
						basicFrm = BFN;
						basicSimbUnit = BSN;
						flag = ERROR;
						f = 0;
						endedFrame = true;

						frmFormat = WAIT;
					}
				}
				else if (mSerial->GetBitState() == L)
				{
					basicFrm = BFN;
					basicSimbUnit = BSN;
					flag = ERROR;
					f = 0;
					endedFrame = true;

					frmFormat = WAIT;
				}
				break;
			}
			mResults->AddMarker( mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
			mSerial->Advance(clock.AdvanceByHalfPeriod(4));
		}


		// Decode ISU Frames

		if (flag == OK)
		{
			switch (basicFrm)
			{
			case ITT:
				if (basicSimbUnit == ISU)
				{
					switch (f)
					{
					case 2:
						basicSimbUnit = NID;
						flag = OK;
						f++;
						break;

					case 3:
						basicFrm = BFN;
						basicSimbUnit = NID;
						flag = OK;
						f = 0;

						frmFormat = WAIT;
						break;

					default:
						basicFrm = BFN;
						basicSimbUnit = BSN;
						flag = ERROR;
						f = 0;

						frmFormat = WAIT;
					}
				}
				else
				{
					basicFrm = BFN;
					basicSimbUnit = BSN;
					flag = ERROR;
					f = 0;

					frmFormat = WAIT;
				}
				break;

			case FBE:
				if (basicSimbUnit == ISU)
				{
					switch (f)
					{
					case 2:
						basicSimbUnit = DID;
						flag = OK;
						f++;
						break;

					case 3:
						basicSimbUnit = DID;
						flag = OK;
						f = 0;

						frmFormat = WAIT;
						break;

					default:
						basicFrm = BFN;
						basicSimbUnit = BSN;
						flag = ERROR;
						f = 0;

						frmFormat = WAIT;
					}
				}
				else
				{
					basicFrm = BFN;
					basicSimbUnit = BSN;
					flag = ERROR;
					f = 0;

					frmFormat = WAIT;
				}
				break;

			case PAC:
				if (basicSimbUnit == ISU)
				{
					switch (f)
					{
					case 2:
						basicSimbUnit = SID;
						flag = OK;
						f++;
						break;

					case 3:
						basicSimbUnit = DID;
						flag = OK;
						f++;
						break;

					case 4:
						basicSimbUnit = DID;
						flag = OK;
						f++;
						break;

					case 5:
						if (0x03 <= data || 0xFF >= data)
						{
							cp = data;
							basicSimbUnit = CP;
							flag = OK;
							f++;
						}
						else
						{
							basicFrm = BFN;
							basicSimbUnit = BSN;
							flag = ERROR;
							f = 0;

							frmFormat = WAIT;
						}
						break;

					case 6:
						basicSimbUnit = SC;
						flag = OK;
						f++;
						break;

					default:
						if ((f-6)*11 <= cp)
						{
							basicSimbUnit = DATA;
							flag = OK;
							f++;
						}
						else
						{
							basicSimbUnit = FCS;
							flag = OK;

							if (fcs)
							{
								basicFrm = BFN;
								f = 0;
								fcs = 0;

								frmFormat = WAIT;
							}
							else
							{
								f++;
								fcs++;
							}
						}
						
					}
				}
				else
				{
					basicFrm = BFN;
					basicSimbUnit = BSN;
					flag = ERROR;
					f = 0;

					frmFormat = WAIT;
				}
				break;

			default:
				switch (basicSimbUnit)
				{
				case SD:
					if (f == 0)
					{
						flag = OK;
						f++;
					}
					else
					{
						basicFrm = BFN;
						basicSimbUnit = BSN;
						flag = ERROR;
						f = 0;

						frmFormat = WAIT;
					}
					break;

				case ISU:
					switch (data)
					{
					case FRM_EOT_D:
						if (f == 1)
						{
							basicFrm = ITT;
							basicSimbUnit = EOT;
							flag = OK;
							f++;
						}
						else
						{
							basicFrm = BFN;
							basicSimbUnit = BSN;
							flag = ERROR;
							f = 0;

							frmFormat = WAIT;
						}
						break;

					case FRM_ENQ_D:
						if (f == 1)
						{
							basicFrm = FBE;
							basicSimbUnit = ENQ;
							flag = OK;
							f++;
						}
						else
						{
							basicFrm = BFN;
							basicSimbUnit = BSN;
							flag = ERROR;
							f = 0;

							frmFormat = WAIT;
						}
						break;

					case FRM_ACK_D:
						if (f == 1)
						{
							basicSimbUnit = ACK;
							flag = OK;
							f = 0;

							frmFormat = WAIT;
						}
						else
						{
							basicFrm = BFN;
							basicSimbUnit = BSN;
							flag = ERROR;
							f = 0;

							frmFormat = WAIT;
						}
						break;

					case FRM_NAK_D:
						if (f == 1)
						{
							basicSimbUnit = NAK;
							flag = OK;
							f = 0;

							frmFormat = WAIT;
						}
						else
						{
							basicFrm = BFN;
							basicSimbUnit = BSN;
							flag = ERROR;
							f = 0;

							frmFormat = WAIT;
						}
						break;

					case FRM_SOH_D:
						if (f == 1)
						{
							basicFrm = PAC;
							basicSimbUnit = SOH;
							flag = OK;
							f++;
						}
						else
						{
							basicFrm = BFN;
							basicSimbUnit = BSN;
							flag = ERROR;
							f = 0;

							frmFormat = WAIT;
						}
						break;

					default:
						basicFrm = BFN;
						basicSimbUnit = BSN;
						flag = ERROR;
						f = 0;

						frmFormat = WAIT;
					}
					break;

				default:
					basicFrm = BFN;
					basicSimbUnit = BSN;
					flag = ERROR;
					f = 0;

					frmFormat = WAIT;
				}
			}
		}


		// Send Frame

		Frame frame;
		frame.mData1 = data;
		frame.mData2 = 0;
		frame.mType = basicSimbUnit;
		frame.mFlags = flag;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

		mResults->AddFrame(frame);
		mResults->CommitResults();
		ReportProgress(frame.mEndingSampleInclusive);


		CheckIfThreadShouldExit();
	}
}

bool ArcnetAnalyzer::NeedsRerun()
{
	return false;
}

U32 ArcnetAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	/*
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}*/

	return NULL;//mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 ArcnetAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 8;
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