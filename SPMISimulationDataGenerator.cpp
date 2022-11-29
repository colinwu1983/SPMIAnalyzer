#include "SPMISimulationDataGenerator.h"


SPMISimulationDataGenerator::SPMISimulationDataGenerator()
{
}

SPMISimulationDataGenerator::~SPMISimulationDataGenerator()
{
}

void SPMISimulationDataGenerator::Initialize( U32 simulation_sample_rate, SPMIAnalyzerSettings* settings )
{
    // mSimulationSampleRateHz = simulation_sample_rate;
    // mSettings = settings;

    // mClockGenerator.Init( 400000, simulation_sample_rate );

    // mSda = mSPMISimulationChannels.Add( settings->mSdaChannel, mSimulationSampleRateHz, BIT_HIGH );
    // mScl = mSPMISimulationChannels.Add( settings->mSclChannel, mSimulationSampleRateHz, BIT_HIGH );

    // mSPMISimulationChannels.AdvanceAll( mClockGenerator.AdvanceByHalfPeriod( 10.0 ) ); // insert 10 bit-periods of idle

    // mValue = 0;
}

U32 SPMISimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate,
                                                        SimulationChannelDescriptor** simulation_channels )
{
    // U64 adjusted_largest_sample_requested =
    //     AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    // while( mScl->GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    // {
    //     mSPMISimulationChannels.AdvanceAll( mClockGenerator.AdvanceByHalfPeriod( 500 ) );


    //     if( rand() % 20 == 0 )
    //     {
    //         CreateStart();
    //         CreateSPMIByte( 0x24, SPMI_NAK );
    //         CreateStop();
    //         mSPMISimulationChannels.AdvanceAll( mClockGenerator.AdvanceByHalfPeriod( 80 ) );
    //     }


    //     CreateSPMITransaction( 0xA0, SPMI_WRITE, mValue++ + 12 );
    //     mSPMISimulationChannels.AdvanceAll( mClockGenerator.AdvanceByHalfPeriod( 80 ) );
    //     CreateSPMITransaction( 0xA0, SPMI_READ, mValue++ - 43 + ( rand() % 100 ) );
    //     mSPMISimulationChannels.AdvanceAll( mClockGenerator.AdvanceByHalfPeriod( 50 ) );
    //     CreateSPMITransaction( 0x24, SPMI_READ, mValue++ + ( rand() % 100 ) );

    //     mSPMISimulationChannels.AdvanceAll( mClockGenerator.AdvanceByHalfPeriod( 2000 ) ); // insert 20 bit-periods of idle

    //     CreateSPMITransaction( 0x24, SPMI_READ, mValue++ + 16 + ( rand() % 100 ) );

    //     mSPMISimulationChannels.AdvanceAll( mClockGenerator.AdvanceByHalfPeriod( 100 ) );
    // }

    // *simulation_channels = mSPMISimulationChannels.GetArray();
    // return mSPMISimulationChannels.GetCount();
    return largest_sample_requested - 1;
}
