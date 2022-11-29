#ifndef SPMI_ANALYZER_SETTINGS
#define SPMI_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

enum SPMIDirection
{
    SPMI_READ,
    SPMI_WRITE
};
enum SPMIResponse
{
    SPMI_ACK,
    SPMI_NAK
};

enum AddressDisplay
{
    NO_DIRECTION_7,
    NO_DIRECTION_8,
    YES_DIRECTION_8
};

class SPMIAnalyzerSettings : public AnalyzerSettings
{
  public:
    SPMIAnalyzerSettings();
    virtual ~SPMIAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();

    void UpdateInterfacesFromSettings();

    Channel mSdaChannel;
    Channel mSclChannel;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mSdaChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mSclChannelInterface;
};

#endif // SPMI_ANALYZER_SETTINGS
