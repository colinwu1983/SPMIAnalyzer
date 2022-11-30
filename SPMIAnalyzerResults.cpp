#include "SPMIAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SPMIAnalyzer.h"
#include "SPMIAnalyzerSettings.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

SPMIAnalyzerResults::SPMIAnalyzerResults( SPMIAnalyzer* analyzer, SPMIAnalyzerSettings* settings )
    : AnalyzerResults(), mSettings( settings ), mAnalyzer( analyzer )
{
}

SPMIAnalyzerResults::~SPMIAnalyzerResults()
{
}


void SPMIAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& /*channel*/,
                                             DisplayBase display_base ) // unrefereced vars commented out to remove warnings.
{
    // we only need to pay attention to 'channel' if we're making bubbles for more than one channel (as set by
    // AddChannelBubblesWillAppearOn)
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );

    SPMI_FRAME s;
    s.data = frame.mData1;
    char slaveAddr[4];
    AnalyzerHelpers::GetNumberString( s.map.slaveAddr, display_base, 4, slaveAddr, 128 );
    char cmd[8];
    AnalyzerHelpers::GetNumberString( s.map.cmd, display_base, 8, cmd, 128 );
    char regAddrH[8];
    AnalyzerHelpers::GetNumberString( s.map.extregAddrH, display_base, 8, regAddrH, 128 );
    char regAddrL[8];
    AnalyzerHelpers::GetNumberString( s.map.extregAddrL, display_base, 8, regAddrL, 128 );
    char data0[8], data1[8], data2[8];
    AnalyzerHelpers::GetNumberString( s.map.Data0, display_base, 8, data0, 128 );

    U8 BC = s.map.cmd & 0x7;
    switch (BC) {
    case 0:
        break;
    case 1:
        // AnalyzerHelpers::GetNumberString( s.map.Data1, display_base, 8, data1, 128 );
        AnalyzerHelpers::GetNumberString( frame.mData2, display_base, 8, data1, 128 );
        data0[4] = data1[2];
        data0[5] = data1[3];
        break;
    case 2:
        AnalyzerHelpers::GetNumberString( s.map.Data1, display_base, 8, data1, 128 );
        AnalyzerHelpers::GetNumberString( s.map.Data2, display_base, 8, data2, 128 );
        data0[4] = data1[2];
        data0[5] = data1[3]; 
        //never tested
        data0[6] = data2[2]; 
        data0[7] = data2[3]; 
        break;
    case 3:
        // to be developed
        break;
    }

    std::stringstream ss;
    if (s.map.isWrite) {
        AddResultString("W");
    }
    else {
        AddResultString("R");
    }
    ss.str( "" );
    if (s.map.isWrite) {
        ss <<"W " << " REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " D " << data0;
    }
    else {
        ss <<"R " << " REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " D " << data0;
    }
    AddResultString( ss.str().c_str() );
    ss.str( "" );
    ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " DATA " << data0;
    AddResultString( ss.str().c_str() );
    ss.str( "" );
    if (s.map.isWrite) {
        ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " WRITE REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " DATA " << data0;
        // ss << "SLAVE " <<  data1 <<" CMD " << data2 << " WRITE REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " DATA " << data0;
    }
    else {
        ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " READ REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " DATA " << data0;
    }
    AddResultString( ss.str().c_str() );
    ss.str( "" );

}


void SPMIAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 /*export_type_user_id*/ )
{
    // export_type_user_id is only important if we have more than one export type.

    std::stringstream ss;
    void* f = AnalyzerHelpers::StartFile( file );
    ;

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    ss << "Time [s],Packet ID,Address,Data,Read/Write,ACK/NAK" << std::endl;

    char address[ 128 ] = "";
    char rw[ 128 ] = "";
    U64 num_frames = GetNumFrames();
    for( U32 i = 0; i < num_frames; i++ )
    {
        Frame frame = GetFrame( i );

        if( frame.mType == SPMIAddress )
        {
            AnalyzerHelpers::GetNumberString( frame.mData1 >> 1, display_base, 7, address, 128 );
            if( ( frame.mData1 & 0x1 ) != 0 )
                snprintf( rw, sizeof( rw ), "Read" );
            else
                snprintf( rw, sizeof( rw ), "Write" );

            // check to see if the address packet is NAKed. If it is, we need to export the line here.
            if( ( frame.mFlags & SPMI_FLAG_ACK ) == 0 )
            {
                char ack[ 32 ];
                if( ( frame.mFlags & SPMI_MISSING_FLAG_ACK ) != 0 )
                    snprintf( ack, sizeof( ack ), "Missing ACK/NAK" );
                else
                    snprintf( ack, sizeof( ack ), "NAK" );
                // we need to write out the line here.
                char time[ 128 ];
                AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time, 128 );

                ss << time << ",," << address << ","
                   << ""
                   << "," << rw << "," << ack << std::endl;
                AnalyzerHelpers::AppendToFile( ( U8* )ss.str().c_str(), ss.str().length(), f );
                ss.str( std::string() );
            }
        }
        else
        {
            char time[ 128 ];
            AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time, 128 );

            char data[ 128 ];
            AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, data, 128 );

            char ack[ 32 ];
            if( ( frame.mFlags & SPMI_FLAG_ACK ) != 0 )
                snprintf( ack, sizeof( ack ), "ACK" );
            else if( ( frame.mFlags & SPMI_MISSING_FLAG_ACK ) != 0 )
                snprintf( ack, sizeof( ack ), "Missing ACK/NAK" );
            else
                snprintf( ack, sizeof( ack ), "NAK" );


            U64 packet_id = GetPacketContainingFrameSequential( i );
            if( packet_id != INVALID_RESULT_INDEX )
                ss << time << "," << packet_id << "," << address << "," << data << "," << rw << "," << ack << std::endl;
            else
                ss << time << ",," << address << "," << data << "," << rw << "," << ack << std::endl;
        }

        AnalyzerHelpers::AppendToFile( ( U8* )ss.str().c_str(), ss.str().length(), f );
        ss.str( std::string() );


        if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
        {
            AnalyzerHelpers::EndFile( f );
            return;
        }
    }

    UpdateExportProgressAndCheckForCancel( num_frames, num_frames );
    AnalyzerHelpers::EndFile( f );
}

void SPMIAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
    ClearTabularText();

    Frame frame = GetFrame( frame_index );
    std::stringstream ss;
    SPMI_FRAME s;
    s.data = frame.mData1;
    char slaveAddr[4];
    AnalyzerHelpers::GetNumberString( s.map.slaveAddr, display_base, 4, slaveAddr, 128 );
    char cmd[8];
    AnalyzerHelpers::GetNumberString( s.map.cmd, display_base, 8, cmd, 128 );
    char regAddrH[8];
    AnalyzerHelpers::GetNumberString( s.map.extregAddrH, display_base, 8, regAddrH, 128 );
    char regAddrL[8];
    AnalyzerHelpers::GetNumberString( s.map.extregAddrL, display_base, 8, regAddrL, 128 );
    char data0[8];
    AnalyzerHelpers::GetNumberString( s.map.Data0, display_base, 8, data0, 128 );

    if (s.map.isWrite) {
        ss << "W " << slaveAddr << " " << regAddrH << regAddrL[2]  << regAddrL[3] << " " << data0;
    }
    else {
        ss << "R " << slaveAddr << " " << regAddrH << regAddrL[2]  << regAddrL[3] << " " << data0;
    }
 
    AddTabularText( ss.str().c_str() );
}

void SPMIAnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/,
                                                    DisplayBase /*display_base*/ ) // unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString( "not supported" );
}

void
    SPMIAnalyzerResults::GenerateTransactionTabularText( U64 /*transaction_id*/,
                                                        DisplayBase /*display_base*/ ) // unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString( "not supported" );
}
