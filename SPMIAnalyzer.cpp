#include "SPMIAnalyzer.h"
#include "SPMIAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

SPMIAnalyzer::SPMIAnalyzer() : Analyzer2(), mSettings( new SPMIAnalyzerSettings() ), mSimulationInitilized( false )
{
    SetAnalyzerSettings( mSettings.get() );
}

SPMIAnalyzer::~SPMIAnalyzer()
{
    KillThread();
}

void SPMIAnalyzer::SetupResults()
{
    mResults.reset( new SPMIAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mSdaChannel );
}

void SPMIAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();
    mNeedAddress = true;

    mSda = GetAnalyzerChannelData( mSettings->mSdaChannel );
    mScl = GetAnalyzerChannelData( mSettings->mSclChannel );


    for( ;; ) {
        AdvanceToStartBit();
        GetFrame();
        CheckIfThreadShouldExit();
    }
}

U8 SPMIAnalyzer::GetData(U8 len) {
    U8 value = 0;
    for (U8 i = 0; i < len; i++) {
        mScl->AdvanceToNextEdge(); //fall edge
        mArrowLocations.push_back( mScl->GetSampleNumber());
        mSda->AdvanceToAbsPosition(mScl->GetSampleNumber());
        BitState bit_state = mSda->GetBitState();
        value += ((bit_state == BIT_LOW) ? 0:1) << (len - i - 1);
        mScl->AdvanceToNextEdge(); //rise edge
    }
    return value;
}

U64 SPMIAnalyzer::GetFrame(void) 
{
    mArrowLocations.clear();
    SPMI_FRAME s;

    mScl->AdvanceToNextEdge(); //rise edge
    if( mScl->GetBitState() == BIT_LOW ) return 0; //oops NG, not rise edge
    U64 starting_sample = mScl->GetSampleNumber();
    s.map.slaveAddr = GetData(SPMI_LEN_SLAVE_ADDR);
    s.map.cmd = GetData(SPMI_LEN_CMD);
    s.map.p_cmd = GetData(SPMI_LEN_PARITY);
    s.map.isWrite = true;
    U8 BC = 0;
    // Extended Register Write Long 
    if ((s.map.cmd >= spmi_ext_reg_write_long_l) && 
        (s.map.cmd <= spmi_ext_reg_write_long_h)) {
        s.map.isWrite = true;
        BC = s.map.cmd & 0x7;
    }
    // Extended Register Read Long 
    else if ((s.map.cmd >= spmi_ext_reg_read_long_l) && 
        (s.map.cmd <= spmi_ext_reg_read_long_h)) {
        s.map.isWrite = false;
        BC = s.map.cmd & 0x7;
    }
    // WIP, currently only support extended register read/write
    else {
        return s.data;
    }

    s.map.extregAddrH = GetData(SPMI_LEN_REG_ADDR);
    s.map.p_extregAddrH = GetData(SPMI_LEN_PARITY);
    s.map.extregAddrL = GetData(SPMI_LEN_REG_ADDR);
    s.map.p_extregAddrL = GetData(SPMI_LEN_PARITY);

    if (!s.map.isWrite) {
        s.map.park = GetData(SPMI_LEN_PARITY);
    }
    s.map.Data0 = GetData(SPMI_LEN_DATA);
    s.map.p_Data0 = GetData(SPMI_LEN_PARITY);

    Frame frame;
    frame.mData1 = s.data;
    if (BC) {
        frame.mType = SPMIMultiData;
        for (U8 i=0; i < BC; i++) {
            U8 val = 0;
            U8 p_val = 0;
            val = GetData(SPMI_LEN_DATA);
            p_val = GetData(SPMI_LEN_PARITY);
            frame.mData2 += val << i * 8;
        }
    }
    else {
        frame.mType = SPMISingleData;
    }
    frame.mStartingSampleInclusive = starting_sample;
    U64 ending_sample = mScl->GetSampleNumber();
    frame.mEndingSampleInclusive = ending_sample;

    mResults->AddFrame( frame );

    //to be improved
    U32 count = mArrowLocations.size();
    for( U32 i = 0; i < count; i++ ) {
        mResults->AddMarker( mArrowLocations[ i ], AnalyzerResults::DownArrow, mSettings->mSclChannel );
    }
    mResults->CommitResults();

    return s.data;

}


void SPMIAnalyzer::AdvanceToStartBit()
{
    for( ;; )
    {
        mSda->AdvanceToNextEdge();

        if( mSda->GetBitState() == BIT_HIGH ) {
            mScl->AdvanceToAbsPosition( mSda->GetSampleNumber() );
            if( mScl->GetBitState() == BIT_LOW ) {
                mSda->AdvanceToNextEdge();
                if (mScl->AdvanceToAbsPosition(mSda->GetSampleNumber()) == 0) {
                    if ( mScl->GetBitState() == BIT_LOW ) {
                        mResults->AddMarker( mSda->GetSampleNumber(), AnalyzerResults::Start, mSettings->mSdaChannel ); 
                        mResults->CommitResults();
                        break;
                    }
                }
            }
        }
    }
}

bool SPMIAnalyzer::NeedsRerun()
{
    return false;
}

U32 SPMIAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                         SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitilized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SPMIAnalyzer::GetMinimumSampleRateHz()
{
    return 2000000;
}

const char* SPMIAnalyzer::GetAnalyzerName() const
{
    return "SPMIcwu";
}

const char* GetAnalyzerName()
{
    return "SPMIcwu";
}

Analyzer* CreateAnalyzer()
{
    return new SPMIAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}
