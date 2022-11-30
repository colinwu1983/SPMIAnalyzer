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
    

    //never tested >= 3bytes ext_str
    //at least space is needed...
    char ext_str[64];
    if (BC) {
        for (U8 i=0; i < BC; i++) {
            AnalyzerHelpers::GetNumberString( frame.mData2 >> (i * 8), display_base, 8, & ext_str[i * 4], 128 );
        }
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
        ss <<"W " << " REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " D " << data0 << " " << ext_str;
    }
    else {
        ss <<"R " << " REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " D " << data0 << " " << ext_str;
    }
    AddResultString( ss.str().c_str() );
    ss.str( "" );
    ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " DATA " << data0 << " " << ext_str;
    AddResultString( ss.str().c_str() );
    ss.str( "" );
    if (BC) {
        if (s.map.isWrite) {
            ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " WRITE REG " << regAddrH << regAddrL[2]  << regAddrL[3] 
            << " Multi Data " << data0 << " " << ext_str;
        }
        else {
            ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " READ REG " << regAddrH << regAddrL[2]  << regAddrL[3] 
            << " Multi Data " << data0  << " " << ext_str;
        }
    }
    else {
        if (s.map.isWrite) {
            ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " WRITE REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " DATA " << data0;
        }
        else {
            ss << "SLAVE " << slaveAddr <<" CMD " << cmd << " READ REG " << regAddrH << regAddrL[2]  << regAddrL[3] << " DATA " << data0;
        }
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

    ss << "Time [s], Slave, Command, Read/Write, Register, data" << std::endl;
    AnalyzerHelpers::AppendToFile( ( U8* )ss.str().c_str(), ss.str().length(), f );
    ss.str( std::string() );

    U64 num_frames = GetNumFrames();
    for( U32 i = 0; i < num_frames; i++ )
    {
        Frame frame = GetFrame( i );

        char time[ 128 ];
        AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time, 128 );

        char data[ 128 ];
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
        //never tested >= 3bytes ext_str
        //at least space is needed...
        char ext_str[64];
        if (BC) {
            for (U8 i=0; i < BC; i++) {
                AnalyzerHelpers::GetNumberString( frame.mData2 >> (i * 8), display_base, 8, & ext_str[i * 4], 128 );
            }
        }
        std::stringstream ss;

        if (BC) {
            if (s.map.isWrite) {
                ss << time
                << "," << slaveAddr <<"," << cmd << "," << "Write," << regAddrH << regAddrL[2]  << regAddrL[3] 
                << "," << data0 << " " << ext_str << std::endl;
            }
            else {
                ss << time
                << "," << slaveAddr <<"," << cmd << "," << "Read," << regAddrH << regAddrL[2]  << regAddrL[3] 
                << "," << data0  << " " << ext_str << std::endl;
            }
        }
        else {
            if (s.map.isWrite) {
                ss << time
                << "," << slaveAddr <<"," << cmd << "," << "Write," << regAddrH << regAddrL[2]  << regAddrL[3] << "," << data0 << std::endl;
            }
            else {
                ss << time
                << "," << slaveAddr <<"," << cmd << "," << "Read," << regAddrH << regAddrL[2]  << regAddrL[3] << "," << data0 << std::endl;
            }
        }
        // AddResultString( ss.str().c_str() );

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

    U8 BC = s.map.cmd & 0x7;
    char ext_str[64] = "";
    char *mbit;
    if (BC) {
        mbit = "multi ";
        for (U8 i=0; i < BC; i++) {
            AnalyzerHelpers::GetNumberString( frame.mData2 >> (i * 8), display_base, 8, & ext_str[i * 4], 128 );
        }
    }
    else {
        mbit = "";
    }
    // mbit = (BC) ? "m" : "";
    if (s.map.isWrite) {
        ss << mbit << "W " << slaveAddr << " " << regAddrH << regAddrL[2]  << regAddrL[3] << " " << data0 << " " << ext_str;
    }
    else {
        ss << mbit << "R " << slaveAddr << " " << regAddrH << regAddrL[2]  << regAddrL[3] << " " << data0 << " " << ext_str;
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
