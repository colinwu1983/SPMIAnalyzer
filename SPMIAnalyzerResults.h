#ifndef SERIAL_ANALYZER_RESULTS
#define SERIAL_ANALYZER_RESULTS

#include <AnalyzerResults.h>

#define SPMI_FLAG_ACK ( 1 << 0 )
#define SPMI_MISSING_FLAG_ACK ( 1 << 1 )

enum SPMIFrameType
{
    SPMIAddress, //old, not used
    SPMIData, //old, not used
    SPMIcwutest,
    SPMIslaveaddr, //cmd frame has 4bit address field
    SPMIcmd, //cmd frame has 8bit cmd field, and a single parity bit
    SPMIregaddr, //data or addr frame, 8bit, and a single parity bit
    SPMIregwritedata,
    SPMIregreaddata,
    SPMInoresp, //no response frame, 9bit for data frame, 13 bit for cmd frame
};

class SPMIAnalyzer;
class SPMIAnalyzerSettings;

class SPMIAnalyzerResults : public AnalyzerResults
{
  public:
    SPMIAnalyzerResults( SPMIAnalyzer* analyzer, SPMIAnalyzerSettings* settings );
    virtual ~SPMIAnalyzerResults();

    virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
    virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

    virtual void GenerateFrameTabularText( U64 frame_index, DisplayBase display_base );
    virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
    virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

  protected: // functions
  protected: // vars
    SPMIAnalyzerSettings* mSettings;
    SPMIAnalyzer* mAnalyzer;
};

#endif // SERIAL_ANALYZER_RESULTS
