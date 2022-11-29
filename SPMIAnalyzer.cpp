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


    for( ;; )
    {
        AdvanceToStartBit();
        // mScl->AdvanceToNextEdge(); // now scl is low.
        U64 SlaveAddr = GetFrame(4, SPMIslaveaddr);
#if 0
        U64 cmd = GetFrame(9, SPMIcmd);
        
        //
        U8 BC = 0;
        //Extended Register Write Long 
        if (((cmd >> 1) >= spmi_ext_reg_write_long_l) && 
            ((cmd >> 1) <= spmi_ext_reg_write_long_h)) {
            BC = cmd & 0xf;
            GetFrame(18, SPMIregaddr);
            GetFrame(9 + BC * 8, SPMIregwritedata);
        }
        //Extended Register Read Long 
        else if (((cmd >> 1) >= spmi_ext_reg_read_long_l) && 
            ((cmd >> 1) <= spmi_ext_reg_read_long_h)) {
            GetFrame(18, SPMIregaddr);
            GetFrame(10, SPMIregreaddata);
        }
        else {// WIP
            GetFrame(9, SPMIregaddr);
            GetFrame(9 + BC * 8, SPMIregwritedata);
        }
#endif        
        CheckIfThreadShouldExit();
    }
}

U64 SPMIAnalyzer::GetFrame(U8 frameLength, SPMIFrameType frameType)
{
    mArrowLocations.clear();
    U8 local_Length, curbit;
    U64 value;
    U32 i = 0;
    // DataBuilder byte;
    // byte.Reset( &value, AnalyzerEnums::MsbFirst, frameLength );
    U64 starting_sample = mScl->GetSampleNumber();
    U64 ex_sample;


    mScl->AdvanceToNextEdge(); //rise edge
    if( mScl->GetBitState() == BIT_LOW ) return 0; //oops NG, not rise edge

    // 4bit slaveAddr
    local_Length = SPMI_SLAVE_ADDR_LEN;
    curbit = SPMI_SLAVE_ADDR_LEN;
    for(i = 0; i < local_Length; i++ ) {
        ex_sample = mScl->GetSampleNumber();
        mScl->AdvanceToNextEdge(); //rise edge
        mArrowLocations.push_back( mScl->GetSampleNumber());
        mSda->AdvanceToAbsPosition(mScl->GetSampleNumber());
        BitState bit_state = mSda->GetBitState();
        value += ((bit_state == BIT_LOW) ? 0:1) << (curbit - i - 1);
        // byte.AddBit(bit_state);
        mScl->AdvanceToNextEdge(); //fall edge
    }
    U8 slaveAddr = value;

    // 9bit cmd + Parity
    local_Length = SPMI_CMD_LEN;
    curbit += SPMI_CMD_LEN;
    for(i = 0; i < local_Length; i++ ) {
        ex_sample = mScl->GetSampleNumber();
        mScl->AdvanceToNextEdge(); //rise edge
        mArrowLocations.push_back( mScl->GetSampleNumber());
        mSda->AdvanceToAbsPosition(mScl->GetSampleNumber());
        BitState bit_state = mSda->GetBitState();
        value += ((bit_state == BIT_LOW) ? 0:1) << (curbit - i - 1);
        mScl->AdvanceToNextEdge(); //fall edge
    }
    U8 cmd = value >> (SPMI_SLAVE_ADDR_LEN + 1);

    U8 BC = 0;
    // Extended Register Write Long 
    if ((cmd >= spmi_ext_reg_write_long_l) && 
        (cmd <= spmi_ext_reg_write_long_h)) {
        BC = cmd & 0xf;
    }
    // Extended Register Read Long 
    else if ((cmd >= spmi_ext_reg_read_long_l) && 
        (cmd <= spmi_ext_reg_read_long_h)) {
        BC = cmd & 0xf;
    }
    // WIP, currently only support extended register read/write
    else {
        return value;
    }

    local_Length = SPMI_REG_ADDR_LEN;
    curbit += SPMI_REG_ADDR_LEN;
    for(i = 0; i < local_Length; i++ ) {
        ex_sample = mScl->GetSampleNumber();
        mScl->AdvanceToNextEdge(); //rise edge
        mArrowLocations.push_back( mScl->GetSampleNumber());
        mSda->AdvanceToAbsPosition(mScl->GetSampleNumber());
        BitState bit_state = mSda->GetBitState();
        value += ((bit_state == BIT_LOW) ? 0:1) << (curbit - i - 1);
        mScl->AdvanceToNextEdge(); //fall edge
    }
    U32 regAddr = value >> (SPMI_SLAVE_ADDR_LEN + SPMI_CMD_LEN + 1);

#if 0
    local_Length = SPMI_DATA_LEN * (BC + 1) + 1;
    curbit += SPMI_DATA_LEN * (BC + 1) + 1;
    for(; i < local_Length; i++ ) {
        ex_sample = mScl->GetSampleNumber();
        mScl->AdvanceToNextEdge(); //rise edge
        mArrowLocations.push_back( mScl->GetSampleNumber());
        mSda->AdvanceToAbsPosition(mScl->GetSampleNumber());
        BitState bit_state = mSda->GetBitState();
        value += ((bit_state == BIT_LOW) ? 0:1) << (curbit - i - 1);
        mScl->AdvanceToNextEdge(); //fall edge
    }
    U32 data = value >> (SPMI_SLAVE_ADDR_LEN + SPMI_CMD_LEN + SPMI_REG_ADDR_LEN + 1);
#endif
    U64 ending_sample = mScl->GetSampleNumber();;
    // if (frameType == SPMIslaveaddr) //w/o Parity
    //     ending_sample = mScl->GetSampleNumber();
    // else //with Parity
    //     ending_sample = ex_sample;

    Frame frame;
    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = ending_sample;
    frame.mData1 = value;
    frame.mType = frameType;
    mResults->AddFrame( frame );

    U32 count = mArrowLocations.size();
    for( U32 i = 0; i < count; i++ ) {
        mResults->AddMarker( mArrowLocations[ i ], AnalyzerResults::DownArrow, mSettings->mSclChannel );
    }
    mResults->CommitResults();

    return value;

}

/*
U64 SPMIAnalyzer::GetFrameold(U8 frameLength, SPMIFrameType frameType)
{
    mArrowLocations.clear();
    U64 value;
    DataBuilder byte;
    byte.Reset( &value, AnalyzerEnums::MsbFirst, frameLength );
    U64 starting_sample = mScl->GetSampleNumber();
    U64 ex_sample;
    if ((frameType == SPMIslaveaddr))
        mScl->AdvanceToNextEdge(); //rise edge
    if( mScl->GetBitState() == BIT_LOW ) return 0; //oops NG, not rise edge
    // if(( mScl->GetBitState() == BIT_LOW ) && (frameType == SPMIslaveaddr)) return; //oops NG, not rise edge
    for( U32 i = 0; i < frameLength; i++ ) {
        ex_sample = mScl->GetSampleNumber();
        mScl->AdvanceToNextEdge(); //rise edge
        mArrowLocations.push_back( mScl->GetSampleNumber());
        mSda->AdvanceToAbsPosition(mScl->GetSampleNumber());
        BitState bit_state = mSda->GetBitState();
        value += ((bit_state == BIT_LOW) ? 0:1) << (frameLength - i - 1);
        byte.AddBit(bit_state);
        mScl->AdvanceToNextEdge(); //fall edge

    }
    U64 ending_sample;
    if (frameType == SPMIslaveaddr) //w/o Parity
        ending_sample = mScl->GetSampleNumber();
    else //with Parity
        ending_sample = ex_sample;

    Frame frame;
    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = ending_sample;
    frame.mData1 = value;
    frame.mType = frameType;
    mResults->AddFrame( frame );

    U32 count = mArrowLocations.size();
    for( U32 i = 0; i < count; i++ ) {
        mResults->AddMarker( mArrowLocations[ i ], AnalyzerResults::DownArrow, mSettings->mSclChannel );
    }
    mResults->CommitResults();

    return value;

}
*/


void SPMIAnalyzer::GetFrameTest()
{
    mArrowLocations.clear();
    U64 value;
    DataBuilder byte;
    U8 curbit = 8;
    byte.Reset( &value, AnalyzerEnums::MsbFirst, curbit );
    U64 starting_sample = mScl->GetSampleNumber();

    mScl->AdvanceToNextEdge(); //rise edge
    if( mScl->GetBitState() == BIT_LOW ) return; //oops NG, not rise edge
    for( U32 i = 0; i < curbit; i++ ) {
        mScl->AdvanceToNextEdge(); //rise edge
        mArrowLocations.push_back( mScl->GetSampleNumber() );
        mSda->AdvanceToAbsPosition(mScl->GetSampleNumber());
        BitState bit_state = mSda->GetBitState();
        value += ((bit_state == BIT_LOW) ? 0:1) << (curbit - i - 1);
        byte.AddBit(bit_state);
        mScl->AdvanceToNextEdge(); //fall edge

    }
    U64 ending_sample = mScl->GetSampleNumber();
    Frame frame;
    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = ending_sample;
    frame.mData1 = value;
    frame.mType = SPMIcwutest;
    mResults->AddFrame( frame );

    U32 count = mArrowLocations.size();
    for( U32 i = 0; i < count; i++ ) {
        mResults->AddMarker( mArrowLocations[ i ], AnalyzerResults::DownArrow, mSettings->mSclChannel );
    }
    mResults->CommitResults();

}



void SPMIAnalyzer::GetByte()
{
    mArrowLocations.clear();
    U64 value;
    DataBuilder byte;
    byte.Reset( &value, AnalyzerEnums::MsbFirst, 8 );
    U64 starting_sample = 0;
    U64 potential_ending_sample = 0;

    for( U32 i = 0; i < 8; i++ )
    {
        BitState bit_state;
        U64 scl_rising_edge;
        bool result = GetBitPartOne( bit_state, scl_rising_edge, potential_ending_sample );
        result &= GetBitPartTwo();
        if( result == true )
        {
            mArrowLocations.push_back( scl_rising_edge );
            byte.AddBit( bit_state );

            if( i == 0 )
                starting_sample = scl_rising_edge;
        }
        else
        {
            return;
        }
    }

    BitState ack_bit_state;
    U64 scl_rising_edge;
    S64 last_valid_sample = mScl->GetSampleNumber();
    bool result = GetBitPartOne( ack_bit_state, scl_rising_edge, potential_ending_sample ); // GetBit( ack_bit_state, scl_rising_edge );


    Frame frame;
    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = result ? potential_ending_sample : last_valid_sample;
    frame.mData1 = U8( value );

    if( !result )
    {
        frame.mFlags = SPMI_MISSING_FLAG_ACK;
    }
    else
    {
        bool ack = ack_bit_state == BIT_LOW;

        // true == ack, false == nak
        // framev2.AddBoolean( "ack", ack );
        if( ack )
        {
            frame.mFlags = SPMI_FLAG_ACK;
        }
    }

    if( mNeedAddress == true && result == true ) // if result is false, then we have already recorded a stop bit and toggled mNeedAddress
    {
        mNeedAddress = false;
        bool is_read = value & 0x01;
        U8 address = value >> 1;
        frame.mType = SPMIAddress;
    }
    else
    {
        frame.mType = SPMIData;
    }

    mResults->AddFrame( frame );

    U32 count = mArrowLocations.size();
    for( U32 i = 0; i < count; i++ )
    {
        mResults->AddMarker( mArrowLocations[ i ], AnalyzerResults::UpArrow, mSettings->mSclChannel );
    }

    mResults->CommitResults();

    result &= GetBitPartTwo();
}

bool SPMIAnalyzer::GetBit( BitState& bit_state, U64& sck_rising_edge )
{
    // SCL must be low coming into this function
    mScl->AdvanceToNextEdge(); // posedge
    sck_rising_edge = mScl->GetSampleNumber();
    mSda->AdvanceToAbsPosition( sck_rising_edge ); // data read on SCL posedge

    bit_state = mSda->GetBitState();
    bool result = true;

    // this while loop is only important if you need to be careful and check for things that that might happen at the very end of a data
    // set, and you don't want to get stuck waithing on a channel that never changes.
    while( mScl->DoMoreTransitionsExistInCurrentData() == false )
    {
        // there are no more SCL transtitions, at least yet.
        if( mSda->DoMoreTransitionsExistInCurrentData() == true )
        {
            // there ARE some SDA transtions, let's double check to make sure there's still no SDA activity
            auto next_data_edge = mSda->GetSampleOfNextEdge();
            if( mScl->WouldAdvancingToAbsPositionCauseTransition( next_data_edge - 1 ) )
            {
                break;
            }

            // ok, for sure we can advance to the next SDA edge without running past any SCL events.
            mSda->AdvanceToNextEdge();

            RecordStartStopBit();
            result = false;
        }
    }

    mScl->AdvanceToNextEdge(); // negedge; we'll leave the clock here
    while( mSda->WouldAdvancingToAbsPositionCauseTransition( mScl->GetSampleNumber() - 1 ) == true )
    {
        // clock is high -- SDA changes indicate start, stop, etc.
        mSda->AdvanceToNextEdge();
        RecordStartStopBit();
        result = false;
    }

    return result;
}

bool SPMIAnalyzer::GetBitPartOne( BitState& bit_state, U64& sck_rising_edge, U64& frame_end_sample )
{
    // SCL must be low coming into this function
    mScl->AdvanceToNextEdge(); // posedge
    sck_rising_edge = mScl->GetSampleNumber();
    frame_end_sample = sck_rising_edge;
    mSda->AdvanceToAbsPosition( sck_rising_edge ); // data read on SCL posedge

    bit_state = mSda->GetBitState();

    // clock is on the rising edge, and data is at the same location.

    while( mScl->DoMoreTransitionsExistInCurrentData() == false )
    {
        // there are no more SCL transtitions, at least yet.
        if( mSda->DoMoreTransitionsExistInCurrentData() == true )
        {
            // there ARE some SDA transtions, let's double check to make sure there's still no SDA activity
            auto next_data_edge = mSda->GetSampleOfNextEdge();
            if( mScl->WouldAdvancingToAbsPositionCauseTransition( next_data_edge - 1 ) )
            {
                break;
            }

            // ok, for sure we can advance to the next SDA edge without running past any SCL events.
            mSda->AdvanceToNextEdge();
            mScl->AdvanceToAbsPosition( mSda->GetSampleNumber() ); // clock is still high, we're just moving it to the stop condition here.
            RecordStartStopBit();
            return false;
        }
    }

    // ok, so there are more transitions on the clock channel, so the above code path didn't run.
    U64 sample_of_next_clock_falling_edge = mScl->GetSampleOfNextEdge();
    while( mSda->WouldAdvancingToAbsPositionCauseTransition( sample_of_next_clock_falling_edge - 1 ) == true )
    {
        // clock is high -- SDA changes indicate start, stop, etc.
        mSda->AdvanceToNextEdge();
        mScl->AdvanceToAbsPosition( mSda->GetSampleNumber() ); // advance the clock to match the SDA channel.
        RecordStartStopBit();
        return false;
    }

    if( mScl->DoMoreTransitionsExistInCurrentData() == true )
    {
        frame_end_sample = mScl->GetSampleOfNextEdge();
    }

    return true;
}

bool SPMIAnalyzer::GetBitPartTwo()
{
    // the sda and scl should be synced up, and we are either on a stop/start condition (clock high) or we're on a regular bit( clock high).
    // we also should not expect any more start/stop conditions before the next falling edge, I beleive.

    // move to next falling edge.
    bool result = true;
    mScl->AdvanceToNextEdge();
    while( mSda->WouldAdvancingToAbsPositionCauseTransition( mScl->GetSampleNumber() - 1 ) == true )
    {
        // clock is high -- SDA changes indicate start, stop, etc.
        mSda->AdvanceToNextEdge();
        RecordStartStopBit();
        result = false;
    }
    return result;
}

void SPMIAnalyzer::RecordStartStopBit()
{
    bool start = mSda->GetBitState() == BIT_LOW;
    if( start )
    {
        // negedge -> START / restart
        mResults->AddMarker( mSda->GetSampleNumber(), AnalyzerResults::Start, mSettings->mSdaChannel );

        // FrameV2 framev2;
        // mResults->AddFrameV2( framev2, "start", mSda->GetSampleNumber(), mSda->GetSampleNumber() + 1 );
    }
    else
    {
        // posedge -> STOP
        mResults->AddMarker( mSda->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mSdaChannel );
    }

    mNeedAddress = true;
    mResults->CommitPacketAndStartNewPacket();
    mResults->CommitResults();

    if( !start )
    {
        // FrameV2 framev2;
        // mResults->AddFrameV2( framev2, "stop", mSda->GetSampleNumber(), mSda->GetSampleNumber() + 1 );
        ;
    }
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
                if (mScl->AdvanceToAbsPosition(mSda->GetSampleNumber()) == 0)
                    if ( mScl->GetBitState() == BIT_LOW ) {
                        mResults->AddMarker( mSda->GetSampleNumber(), AnalyzerResults::Start, mSettings->mSdaChannel ); 
                        mResults->CommitResults();
                        break;
                    }
            }
        }
    }

    //RecordStartStopBit();
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
