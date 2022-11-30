#ifndef SERIAL_ANALYZER_H
#define SERIAL_ANALYZER_H

#include <Analyzer.h>
#include "SPMIAnalyzerResults.h"
#include "SPMISimulationDataGenerator.h"

#define spmi_ext_reg_write_long_l  0x30 // Extended Register Write Long lowest
#define spmi_ext_reg_write_long_h  0x37 // Extended Register Write Long highest
#define spmi_ext_reg_read_long_l  0x38 // Extended Register Read Long lowest
#define spmi_ext_reg_read_long_h  0x3F // Extended Register Read Long highest

#define spmi_ext_reg_write_l  0x00 // Extended Register Write lowest
#define spmi_ext_reg_write_h  0x0F // Extended Register Write highest
#define spmi_ext_reg_read_l  0x20 // Extended Register Read lowest
#define spmi_ext_reg_read_h  0x2F // Extended Register Read highest

#define spmi_reg_write_l  0x40 // Register Write lowest
#define spmi_reg_write_h  0x4f // Register Write highest
#define spmi_reg_read_l  0x60 // Register Read lowest
#define spmi_reg_read_h  0x7F // Register Read highest

#define spmi_master_write_l  0x16 // Master Write lowest
#define spmi_master_write_h  0x16 // Master Write highest
#define spmi_master_read_l  0x15 // Master Read lowest
#define spmi_master_read_h  0x15 // Master Read highest

#define SPMI_LEN_SLAVE_ADDR 4
#define SPMI_LEN_CMD 8
#define SPMI_LEN_REG_ADDR 8
#define SPMI_LEN_DATA 8
#define SPMI_LEN_PARITY 1

typedef union _SPMI_FRAME {
    struct {
        U8 slaveAddr : 4; //4
        U8 cmd: 8; //12
        U8 p_cmd : 1; //13
        U8 extregAddrH: 8; //21
        U8 p_extregAddrH: 1; //22
        U8 extregAddrL: 8; //30
        U8 p_extregAddrL: 1; //31
        U8 isWrite: 1; //59
        U8 park: 1; //60
        U8 Data0: 8; //39
        U8 p_Data0: 1; //40
        U8 Data1: 8; //48
        U8 p_Data1: 1; //49     
        U8 Data2: 8; //57
        U8 p_Data2: 1; //58     
        // U8 :24;
    } map;

    U64 data;
} SPMI_FRAME;

class SPMIAnalyzerSettings;
class SPMIAnalyzer : public Analyzer2
{
  public:
    SPMIAnalyzer();
    virtual ~SPMIAnalyzer();
    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();


#pragma warning( push )
#pragma warning(                                                                                                                           \
    disable : 4251 ) // warning C4251: 'SerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

  protected: // functions
    void AdvanceToStartBit();
    void GetFrameTest();
    U64 GetFrame(U8 frameLength, SPMIFrameType frameType);
    void GetByte();
    U8 GetData(U8 len);
    bool GetBit( BitState& bit_state, U64& sck_rising_edge );
    bool GetBitPartOne( BitState& bit_state, U64& sck_rising_edge, U64& frame_end_sample );
    bool GetBitPartTwo();
    void RecordStartStopBit();

  protected: // vars
    std::auto_ptr<SPMIAnalyzerSettings> mSettings;
    std::auto_ptr<SPMIAnalyzerResults> mResults;
    AnalyzerChannelData* mSda;
    AnalyzerChannelData* mScl;

    SPMISimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    // Serial analysis vars:
    U32 mSampleRateHz;
    bool mNeedAddress;
    std::vector<U64> mArrowLocations;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // SERIAL_ANALYZER_H
