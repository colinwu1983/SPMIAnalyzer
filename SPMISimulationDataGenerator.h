#ifndef SERIAL_SIMULATION_DATA_GENERATOR
#define SERIAL_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>
#include "SPMIAnalyzerSettings.h"
#include <stdlib.h>

class SPMISimulationDataGenerator
{
  public:
    SPMISimulationDataGenerator();
    ~SPMISimulationDataGenerator();

    void Initialize( U32 simulation_sample_rate, SPMIAnalyzerSettings* settings );
    U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );

  protected:
    SPMIAnalyzerSettings* mSettings;
    U32 mSimulationSampleRateHz;
    U8 mValue;

  protected: // SPMI specific
             // functions
    void CreateSPMITransaction( U8 address, SPMIDirection direction, U8 data );
    void CreateSPMIByte( U8 data, SPMIResponse reply );
    void CreateBit( BitState bit_state );
    void CreateStart();
    void CreateStop();
    void CreateRestart();
    void SafeChangeSda( BitState bit_state );

  protected: // vars
    ClockGenerator mClockGenerator;

    SimulationChannelDescriptorGroup mSPMISimulationChannels;
    SimulationChannelDescriptor* mSda;
    SimulationChannelDescriptor* mScl;
};
#endif // UNIO_SIMULATION_DATA_GENERATOR
