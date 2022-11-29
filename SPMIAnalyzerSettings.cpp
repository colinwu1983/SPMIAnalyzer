#include "SPMIAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <cstring>

SPMIAnalyzerSettings::SPMIAnalyzerSettings() : mSdaChannel( UNDEFINED_CHANNEL ), mSclChannel( UNDEFINED_CHANNEL )
{
    mSdaChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mSdaChannelInterface->SetTitleAndTooltip( "SDA", "Serial Data Line" );
    mSdaChannelInterface->SetChannel( mSdaChannel );

    mSclChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mSclChannelInterface->SetTitleAndTooltip( "SCL", "Serial Clock Line" );
    mSclChannelInterface->SetChannel( mSclChannel );

    AddInterface( mSdaChannelInterface.get() );
    AddInterface( mSclChannelInterface.get() );

    // AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mSdaChannel, "SDA", false );
    AddChannel( mSclChannel, "SCL", false );
}

SPMIAnalyzerSettings::~SPMIAnalyzerSettings()
{
}

bool SPMIAnalyzerSettings::SetSettingsFromInterfaces()
{
    if( mSdaChannelInterface->GetChannel() == mSclChannelInterface->GetChannel() )
    {
        SetErrorText( "SDA and SCL can't be assigned to the same input." );
        return false;
    }

    mSdaChannel = mSdaChannelInterface->GetChannel();
    mSclChannel = mSclChannelInterface->GetChannel();

    ClearChannels();
    AddChannel( mSdaChannel, "SDA", true );
    AddChannel( mSclChannel, "SCL", true );

    return true;
}

void SPMIAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    const char* name_string; // the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if( strcmp( name_string, "SaleaeSPMIAnalyzer" ) != 0 )
        AnalyzerHelpers::Assert( "SaleaeSPMIAnalyzer: Provided with a settings string that doesn't belong to us;" );

    text_archive >> mSdaChannel;
    text_archive >> mSclChannel;

    ClearChannels();
    AddChannel( mSdaChannel, "SDA", true );
    AddChannel( mSclChannel, "SCL", true );

    UpdateInterfacesFromSettings();
}

const char* SPMIAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "SaleaeSPMIAnalyzer";
    text_archive << mSdaChannel;
    text_archive << mSclChannel;

    return SetReturnString( text_archive.GetString() );
}

void SPMIAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mSdaChannelInterface->SetChannel( mSdaChannel );
    mSclChannelInterface->SetChannel( mSclChannel );
}
