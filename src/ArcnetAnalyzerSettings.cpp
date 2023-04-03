#include "ArcnetAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


ArcnetAnalyzerSettings::ArcnetAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mBitRate(4999200),
	mInverted(false)
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Arcnet", "Standard Arcnet Analyzer" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/S)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->SetMax(6000000);
	mBitRateInterface->SetMin(1);
	mBitRateInterface->SetInteger(mBitRate);

	mInvertedInterface.reset( new AnalyzerSettingInterfaceBool() );
	mInvertedInterface->SetTitleAndTooltip( "Signal inverted",  "Specify if signal is inverted." );
	mInvertedInterface->SetValue(mInverted);

	AddInterface( mInputChannelInterface.get() );
	AddInterface( mBitRateInterface.get() );
	AddInterface( mInvertedInterface.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "Serial", false );
}

ArcnetAnalyzerSettings::~ArcnetAnalyzerSettings()
{
}

bool ArcnetAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mBitRate = mBitRateInterface->GetInteger();
	mInverted = mInvertedInterface->GetValue();

	ClearChannels();
	AddChannel( mInputChannel, "Arcnet Analyzer", true );

	return true;
}

void ArcnetAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mBitRateInterface->SetInteger( mBitRate );
	mInvertedInterface->SetValue( mInverted );
}

void ArcnetAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
	text_archive >> mBitRate;
	text_archive >> mInverted;

	ClearChannels();
	AddChannel( mInputChannel, "Arcnet Analyzer", true );

	UpdateInterfacesFromSettings();
}

const char* ArcnetAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mBitRate;
	text_archive << mInverted;

	return SetReturnString( text_archive.GetString() );
}
