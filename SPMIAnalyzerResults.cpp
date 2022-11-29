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

    char ack[ 32 ];
    if( ( frame.mFlags & SPMI_FLAG_ACK ) != 0 )
        snprintf( ack, sizeof( ack ), "ACK" );
    else if( ( frame.mFlags & SPMI_MISSING_FLAG_ACK ) != 0 )
        snprintf( ack, sizeof( ack ), "Missing ACK/NAK" );
    else
        snprintf( ack, sizeof( ack ), "NAK" );

    if( frame.mType == SPMIcwutest ) {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
        std::stringstream ss;
        AddResultString( "SSC" );

        // ss << "SSC[" << number_str << "]";
        ss << "S";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        ss << "SSC [" << number_str << "]";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        // ss << "SSC [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
        // ss.str( "" );

        // ss << "Setup Read to [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
    
    }
    else if( frame.mType == SPMIslaveaddr ) {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 4, number_str, 128 );
        std::stringstream ss;
        AddResultString( "SA" );

        // ss << "SSC[" << number_str << "]";
        ss << "SA";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        ss << "SA [" << number_str << "]";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        // ss << "SSC [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
        // ss.str( "" );

        // ss << "Setup Read to [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
    
    }
    else if( frame.mType == SPMIcmd ) {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1 >> 1, display_base, 8, number_str, 128 );
        std::stringstream ss;
        AddResultString( "C" );

        // ss << "SSC[" << number_str << "]";
        ss << "C";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        ss << "CMD [" << number_str << "]";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        // ss << "SSC [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
        // ss.str( "" );

        // ss << "Setup Read to [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
    
    }
    else if( frame.mType == SPMIregaddr ) {
        char number_str[ 128 ];
        U64 regaddr = ((frame.mData1 >> 1 ) & 0xFF) + (((frame.mData1 >> 10 ) & 0xFF) << 8);
        // U64 regaddr = (((frame.mData1 >> 10 ) & 0xFF) << 8);
        // AnalyzerHelpers::GetNumberString( frame.mData1 >> 1, display_base, 17, number_str, 128 );
        AnalyzerHelpers::GetNumberString( regaddr, display_base, 16, number_str, 128 );
        // AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 18, number_str, 128 );
        std::stringstream ss;
        AddResultString( "REG" );

        // ss << "SSC[" << number_str << "]";
        ss << "REG";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        ss << "REG [" << number_str << "]";
        // ss << "RA " << number_str ;
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        // ss << "SSC [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
        // ss.str( "" );

        // ss << "Setup Read to [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
    
    }
    else if( frame.mType == SPMIregwritedata ) {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1 >> 1, display_base, 8, number_str, 128 );
        std::stringstream ss;
        AddResultString( "D" );

        // ss << "SSC[" << number_str << "]";
        ss << "D";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        ss << "D [" << number_str << "]";
        // ss << "RA " << number_str ;
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        // ss << "SSC [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
        // ss.str( "" );

        // ss << "Setup Read to [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
    
    }
    else if( frame.mType == SPMIregreaddata ) {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( (frame.mData1 >> 1) & 0xFF, display_base, 8, number_str, 128 );
        std::stringstream ss;
        AddResultString( "D" );

        // ss << "SSC[" << number_str << "]";
        ss << "D";
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        ss << "D [" << number_str << "]";
        // ss << "RA " << number_str ;
        AddResultString( ss.str().c_str() );
        ss.str( "" );

        // ss << "SSC [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
        // ss.str( "" );

        // ss << "Setup Read to [" << number_str << "] + " << ack;
        // AddResultString( ss.str().c_str() );
    
    }
    else
    {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

        AddResultString( number_str );

        std::stringstream ss;
        ss << number_str << " + " << ack;
        AddResultString( ss.str().c_str() );
    }
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

    char ack[ 32 ];
    if( ( frame.mFlags & SPMI_FLAG_ACK ) != 0 )
        snprintf( ack, sizeof( ack ), "ACK" );
    else if( ( frame.mFlags & SPMI_MISSING_FLAG_ACK ) != 0 )
        snprintf( ack, sizeof( ack ), "Missing ACK/NAK" );
    else
        snprintf( ack, sizeof( ack ), "NAK" );

    if( frame.mType == SPMIAddress )
    {
        char number_str[ 128 ];
        // AnalyzerHelpers::GetNumberString( frame.mData1 >> 1, display_base, 8, number_str, 128 );
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 13, number_str, 128 );

        SPMIDirection direction;
        if( ( frame.mData1 & 0x1 ) != 0 )
            direction = SPMI_READ;
        else
            direction = SPMI_WRITE;

        if( direction == SPMI_READ )
        {
            std::stringstream ss;
            ss << "Setup Read to [" << number_str << "] + " << ack;
            AddTabularText( ss.str().c_str() );
        }
        else
        {
            std::stringstream ss;
            ss << "Setup Write to [" << number_str << "] + " << ack;
            AddTabularText( ss.str().c_str() );
        }
    }
    else
    {
        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
        std::stringstream ss;
        ss << number_str << " + " << ack;
        AddTabularText( ss.str().c_str() );
    }
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
