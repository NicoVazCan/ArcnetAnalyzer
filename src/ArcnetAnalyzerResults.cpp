#include "ArcnetAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "ArcnetAnalyzer.h"
#include "ArcnetAnalyzerSettings.h"
#include <iostream>
#include <fstream>

ArcnetAnalyzerResults::ArcnetAnalyzerResults( ArcnetAnalyzer* analyzer, ArcnetAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

ArcnetAnalyzerResults::~ArcnetAnalyzerResults()
{
}

void ArcnetAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );
	char number_str[128];

	switch (frame.mFlags)
	{
	case OK:
		switch (frame.mType)
		{
		case SD:
			AddResultString("SD");
			break;

		case RSU:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("RSU:", number_str);
			break;

		case ISU:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("ISU:", number_str);
			break;

		case SOH:
			AddResultString("SOH");
			break;

		case ENQ:
			AddResultString("ENQ");
			break;

		case ACK:
			AddResultString("ACK");
			break;

		case NAK:
			AddResultString("NAK");
			break;

		case EOT:
			AddResultString("EOT");
			break;

		case NID:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("NID:", number_str);
			break;

		case SID:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("SID:", number_str);
			break;

		case DID:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("DID:", number_str);
			break;

		case CP:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("CP:", number_str);
			break;

		case SC:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("SC:", number_str);
			break;

		case DATA:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("DATA:", number_str);
			break;

		case FCS:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString("FCS:", number_str);
			break;
		}
		break;

	case ERROR:
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
		AddResultString("ERROR:", number_str);
		break;
	
	default:
		AddResultString("Unrecognised");
	}
}

void ArcnetAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();
	U64 num_frames = GetNumFrames();

	file_stream << "Time [s], SC, DATA, ..., DATA";

	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		char number_str[128];

		switch(frame.mFlags)
        {
        case OK:
			switch(frame.mType)
			{
            case SC:
                AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );
                file_stream << std::endl;
                file_stream << time_str;
                file_stream.put( ',' );
                AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
                file_stream << number_str;
                break;

			case DATA:
                file_stream.put( ',' );
                AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
                file_stream << number_str;
                break;

            default:
                break;
			}
			break;

        default:
            break;
        }

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void ArcnetAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	Frame frame = GetFrame( frame_index );
	ClearTabularText();

	char number_str[128];
	AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	AddTabularText( number_str );
#endif
}

void ArcnetAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	//not supported

}

void ArcnetAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	//not supported
}