extern "C"
{
    #include "jendefs.h"
    #include "zps_gen.h"
    #include "dbg.h"
    #include "string.h"
}

#include "BasicClusterEndpoint.h"
#include "EndpointManager.h"
#include "LEDTask.h"
#include "DumpFunctions.h"
#ifdef CLD_ELECTRICAL_MEASUREMENT
#include "EnergyMeterTask.h"
#endif

BasicClusterEndpoint::BasicClusterEndpoint()
{

}

void BasicClusterEndpoint::registerBasicCluster()
{
    // Create an instance of a basic cluster as a server
    teZCL_Status status = eCLD_BasicCreateBasic(&clusterInstances.sBasicServer,
                                                TRUE,
                                                &sCLD_Basic,
                                                &sBasicServerCluster,
                                                &au8BasicClusterAttributeControlBits[0]);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerBasicCluster(): Failed to create Basic Cluster instance. Status=%d\n", status);
}

void BasicClusterEndpoint::registerIdentifyCluster()
{
    // Create an instance of a basic cluster as a server
    teZCL_Status status = eCLD_IdentifyCreateIdentify(&clusterInstances.sIdentifyServer,
                                                TRUE,
                                                &sCLD_Identify,
                                                &sIdentifyServerCluster,
                                                &au8IdentifyAttributeControlBits[0],
                                                &sIdentifyClusterData);
    
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerIdentifyCluster(): Failed to create Identify Cluster instance. Status=%d\n", status);
}

void BasicClusterEndpoint::registerOtaCluster()
{
    // Create an instance of an OTA cluster as a client */
    teZCL_Status status = eOTA_Create(&clusterInstances.sOTAClient,
                                      FALSE,  /* client */
                                      &sCLD_OTA,
                                      &sOTAClientCluster,  /* cluster definition */
                                      getEndpointId(),
                                      NULL,
                                      &sOTACustomDataStruct);

    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerOtaCluster(): Failed to create OTA Cluster instance. Status=%d\n", status);
}

void BasicClusterEndpoint::registerDeviceTemperatureCluster()
{
    // Create an instance of a device temperature configuration cluster as a server
    teZCL_Status status = eCLD_DeviceTemperatureConfigurationCreateDeviceTemperatureConfiguration(
        &clusterInstances.sDeviceTemperatureServer,
        TRUE,
        &sCLD_DeviceTemperatureConfiguration,
        &sDeviceTemperatureServerCluster,
        &au8DeviceTempConfigClusterAttributeControlBits[0]);

    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerDeviceTemperatureCluster(): Failed to create Device Temperature Configuration Cluster instance. Status=%d\n", status);
}

#ifdef CLD_SM
void BasicClusterEndpoint::registerSimpleMeteringCluster()
{
    // Create an instance of a simple metering cluster as a server
    teZCL_Status status = eSE_SMCreate(getEndpointId(),
                                       TRUE,
                                       &au8SimpleMeteringServerAttributeControlBits[0],
                                       &clusterInstances.sSimpleMeteringServer,
                                       &sCLD_SimpleMetering,
                                       &sSimpleMeteringCustomDataStruct,
                                       &sSimpleMeteringServerCluster);

    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerSimpleMeteringCluster(): Failed to create Simple Metering Cluster instance. Status=%d\n", status);
}
#endif

#ifdef CLD_ELECTRICAL_MEASUREMENT
void BasicClusterEndpoint::registerElectricalMeasurementCluster()
{
    // Create an instance of an electrical measurement cluster as a server
    teZCL_Status status = eCLD_ElectricalMeasurementCreateElectricalMeasurement(
        &clusterInstances.sElectricalMeasurementServer,
        TRUE,
        &sCLD_ElectricalMeasurement,
        &sElectricalMeasurementServerCluster,
        &au8ElectricalMeasurementAttributeControlBits[0]);

    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerElectricalMeasurementCluster(): Failed to create Electrical Measurement Cluster instance. Status=%d\n", status);
}
#endif

void BasicClusterEndpoint::registerEndpoint()
{
    // Fill in end point details
    endPoint.u8EndPointNumber = getEndpointId();
    endPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    endPoint.u16ProfileEnum = HA_PROFILE_ID;
    endPoint.bIsManufacturerSpecificProfile = FALSE;
    endPoint.u16NumberOfClusters = sizeof(BasicClusterInstances) / sizeof(tsZCL_ClusterInstance);
    endPoint.psClusterInstance = (tsZCL_ClusterInstance*)&clusterInstances;
    endPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    endPoint.pCallBackFunctions = &EndpointManager::handleZclEvent;

    // Register the endpoint with all the clusters above
    teZCL_Status status = eZCL_Register(&endPoint);
    DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerEndpoint(): Register Basic Cluster Endpoint. Status=%d\n", status);
}

void BasicClusterEndpoint::init()
{
    registerBasicCluster();
    registerIdentifyCluster();
    registerOtaCluster();
    registerDeviceTemperatureCluster();
#ifdef CLD_ELECTRICAL_MEASUREMENT
    registerElectricalMeasurementCluster();
#endif
#ifdef CLD_SM
    registerSimpleMeteringCluster();
#endif
    registerEndpoint();

    // Fill Basic cluster attributes
    memcpy(sBasicServerCluster.au8ManufacturerName, CLD_BAS_MANUF_NAME_STR, CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBasicServerCluster.au8ModelIdentifier, CLD_BAS_MODEL_ID_STR, CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBasicServerCluster.au8DateCode, CLD_BAS_DATE_STR, CLD_BAS_DATE_SIZE);
    memcpy(sBasicServerCluster.au8SWBuildID, CLD_BAS_SW_BUILD_STR, CLD_BAS_SW_BUILD_SIZE);
    sBasicServerCluster.eGenericDeviceType = E_CLD_BAS_GENERIC_DEVICE_TYPE_WALL_SWITCH;

#ifdef CLD_ELECTRICAL_MEASUREMENT
    // Bit 0 = active measurement (AC)
    sElectricalMeasurementServerCluster.u32MeasurementType = 1;
    // ActivePower is in W, RMSVoltage in 0.1 V, RMSCurrent in mA
    sElectricalMeasurementServerCluster.u16ACPowerMultiplier = 1;
    sElectricalMeasurementServerCluster.u16ACPowerDivisor = 1;
    sElectricalMeasurementServerCluster.u16ACVoltageMultiplier = 1;
    sElectricalMeasurementServerCluster.u16ACVoltageDivisor = 10;
    sElectricalMeasurementServerCluster.u16ACCurrentMultiplier = 1;
    sElectricalMeasurementServerCluster.u16ACCurentDivisor = 1000;  // (sic - SDK field name typo)
#endif

#ifdef CLD_SM
    // CurrentSummationDelivered is in Wh: kWh = value * multiplier / divisor
    sSimpleMeteringServerCluster.eUnitOfMeasure = 0;                // kWh
    sSimpleMeteringServerCluster.u24Multiplier = 1;
    sSimpleMeteringServerCluster.u24Divisor = 1000;
    sSimpleMeteringServerCluster.u8SummationFormatting = 0xBB;      // suppress zeros, 7 int + 3 frac digits
    sSimpleMeteringServerCluster.u8MeterStatus = 0;
    sSimpleMeteringServerCluster.eMeteringDeviceType = 0;           // electric metering
#endif

#ifdef CLD_ELECTRICAL_MEASUREMENT
    // The RP flag in the attribute definition table (vendored cluster files) makes the
    // reporting engine include these attributes; the per-instance control bits below are
    // what the configure-reporting command handler checks (E_ZCL_ACF_RP) - both are needed
    eZCL_SetReportableFlag(getEndpointId(), MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, TRUE, FALSE, E_CLD_ELECTMEAS_ATTR_ID_ACTIVE_POWER);
    eZCL_SetReportableFlag(getEndpointId(), MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, TRUE, FALSE, E_CLD_ELECTMEAS_ATTR_ID_RMS_VOLATGE);  // (sic - SDK enum typo)
    eZCL_SetReportableFlag(getEndpointId(), MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, TRUE, FALSE, E_CLD_ELECTMEAS_ATTR_ID_RMS_CURRENT);
#endif
#ifdef CLD_SM
    eZCL_SetReportableFlag(getEndpointId(), SE_CLUSTER_ID_SIMPLE_METERING, TRUE, FALSE, E_CLD_SM_ATTR_ID_CURRENT_SUMMATION_DELIVERED);
#endif

#ifdef SUPPORTS_POWER_METERING
    // From now on the meter task pushes fresh values into the cluster structs
    EnergyMeterTask::getInstance()->setMeteringEndpoint(this);
#endif

    // Initialize OTA
    otaHandlers.initOTA(getEndpointId());
}

void BasicClusterEndpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;

    switch(clusterId)
    {
        case GENERAL_CLUSTER_ID_IDENTIFY:
            handleIdentifyClusterUpdate(psEvent);
            break;

        case OTA_CLUSTER_ID:
            handleOTAClusterUpdate(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Warning: Unexpected cluster update message ClusterID=%04x\n", clusterId);
            break;
    }
}

void BasicClusterEndpoint::handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->uMessage.sClusterCustomMessage.u16ClusterId;

    switch(clusterId)
    {
        case GENERAL_CLUSTER_ID_IDENTIFY:
            handleIdentifyClusterEvent(psEvent);
            break;

        case OTA_CLUSTER_ID:
            handleOTAClusterEvent(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Warning: Unexpected custom cluster event ClusterID=%04x\n", 
                        getEndpointId(), clusterId);
            break;
    }
}

void BasicClusterEndpoint::handleIdentifyClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    tsCLD_IdentifyCallBackMessage * msg = (tsCLD_IdentifyCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 commandId = msg->u8CommandId;

    DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Identify cluster command Cmd=%d\n",
                psEvent->u8EndPoint,
                commandId);

    switch(commandId)
    {
        case E_CLD_IDENTIFY_CMD_IDENTIFY:
            LEDTask::getInstance()->triggerEffect(BASIC_ENDPOINT, E_CLD_IDENTIFY_EFFECT_BREATHE);
            break;

        case E_CLD_IDENTIFY_CMD_TRIGGER_EFFECT:
            LEDTask::getInstance()->triggerEffect(BASIC_ENDPOINT, msg->uMessage.psTriggerEffectRequestPayload->eEffectId);
            break;

        default:
            break;
    }
}

void BasicClusterEndpoint::handleIdentifyClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    zuint16 identifyTime = sIdentifyServerCluster.u16IdentifyTime;
    DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Identify cluster update event. Identify Time = %d\n",
                psEvent->u8EndPoint, 
                identifyTime);

    if(identifyTime == 0)
        LEDTask::getInstance()->stopEffect();
}

void BasicClusterEndpoint::handleOTAClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    tsOTA_CallBackMessage *psCallBackMessage = (tsOTA_CallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    otaHandlers.handleOTAMessage(psCallBackMessage);
}

void BasicClusterEndpoint::handleOTAClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    // Parse and process OTA message
    tsOTA_CallBackMessage *psCallBackMessage = (tsOTA_CallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    otaHandlers.handleOTAMessage(psCallBackMessage);
}

teZCL_CommandStatus BasicClusterEndpoint::handleReadAttribute(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->pZPSevent->uEvent.sApsDataIndEvent.u16ClusterId;

    switch(clusterId)
    {
        case GENERAL_CLUSTER_ID_DEVICE_TEMPERATURE_CONFIGURATION:
            readDeviceTemperature();
            break;

#ifdef CLD_ELECTRICAL_MEASUREMENT
        case MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT:
#ifdef CLD_SM
        case SE_CLUSTER_ID_SIMPLE_METERING:
#endif
            // Values are pushed every sampling window; refresh once more so a
            // read straddling the window boundary gets the newest data
            updateMeteringAttributes();
            break;
#endif
    }

    return E_ZCL_CMDS_SUCCESS;
}

void BasicClusterEndpoint::readDeviceTemperature()
{
    // Enable the ADC and configure it to measure the temperature
    vAHI_ApConfigure(E_AHI_AP_REGULATOR_ENABLE, E_AHI_AP_INT_DISABLE, E_AHI_AP_SAMPLE_2, E_AHI_AP_CLOCKDIV_500KHZ, E_AHI_AP_INTREF);
    vAHI_AdcEnable(E_AHI_ADC_SINGLE_SHOT, E_AHI_AP_INPUT_RANGE_1, E_AHI_ADC_SRC_TEMP);

    // Perform single shot measurement
    vAHI_AdcStartSample();
    while((bAHI_AdcPoll()))
        ;

    // Convert the raw value to temperature in Celsius
    uint16 rawValue = u16AHI_AdcRead();
    float mV = (float)rawValue * 1.2  / 1024 * 1000;
    sDeviceTemperatureServerCluster.i16CurrentTemperature = (int)((mV - 720.) / (-1.66) + 25);

    DBG_vPrintf(TRUE, "BasicClusterEndpoint: Temperature: %d (raw value: %d)\n", sDeviceTemperatureServerCluster.i16CurrentTemperature, rawValue);

    // Disable the ADC
    vAHI_AdcDisable();
}

#ifdef CLD_ELECTRICAL_MEASUREMENT
void BasicClusterEndpoint::updateMeteringAttributes()
{
    uint16 cfFreqDHz = EnergyMeterTask::getInstance()->getCfFreqDHz();
    uint16 voltageFreqDHz = EnergyMeterTask::getInstance()->getVoltageFreqDHz();
    uint16 currentFreqDHz = EnergyMeterTask::getInstance()->getCurrentFreqDHz();

    // CF -> active power (W); CF1 -> RMS voltage (0.1 V) / RMS current (mA) per SEL mode
    uint32 watts = (uint32)cfFreqDHz * METERING_W_PER_DHZ_E5 / 100000;
    uint32 mA = (uint32)currentFreqDHz * METERING_MA_PER_DHZ_E5 / 100000;
    sElectricalMeasurementServerCluster.i16ActivePower = (watts > 32767) ? 32767 : watts;
    sElectricalMeasurementServerCluster.u16RMSVoltage = (uint32)voltageFreqDHz * METERING_DV_PER_DHZ_E5 / 100000;
    sElectricalMeasurementServerCluster.u16RMSCurrent = (mA > 65535) ? 65535 : mA;

    // Cumulative pulse counts for integrative calibration (0xFF00/0xFF01)
    sElectricalMeasurementServerCluster.u32ManSpecificApparentPower = (uint32)EnergyMeterTask::getInstance()->getCfTotal();
    sElectricalMeasurementServerCluster.u32NonActivePower = EnergyMeterTask::getInstance()->getCf1Total();

#ifdef CLD_SM
    // Lifetime energy in Wh: pulses * (W-per-dHz / 1e5) J/pulse * 10 / 3600
    sSimpleMeteringServerCluster.u48CurrentSummationDelivered =
        EnergyMeterTask::getInstance()->getCfTotal() * METERING_W_PER_DHZ_E5 / 36000000;
#endif
}
#endif
