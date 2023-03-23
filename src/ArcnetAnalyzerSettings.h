#ifndef ARCNET_ANALYZER_SETTINGS
#define ARCNET_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class ArcnetAnalyzerSettings : public AnalyzerSettings
{
public:
	ArcnetAnalyzerSettings();
	virtual ~ArcnetAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;
	bool mInverted;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool >		mInvertedInterface;
};

#endif //ARCNET_ANALYZER_SETTINGS
