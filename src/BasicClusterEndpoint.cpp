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
#include "ZigbeeDevice.h"

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

void BasicClusterEndpoint::registerElectricalMeasurementCluster()
{
    // Create an instance of a device temperature configuration cluster as a server
    teZCL_Status status = eCLD_ElectricalMeasurementCreateElectricalMeasurement(
        &clusterInstances.sElectricalMeasurementServer,
        TRUE,
        &sCLD_ElectricalMeasurement,
        &sElectricalMeasurementServerCluster,
        &au8ElectricalMeasurementAttributeControlBits[0]);

    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerElectricalMeasurementCluster(): Failed to create Electrical Measurements Cluster instance. Status=%d\n", status);
}

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
#ifdef SUPPORTS_HLW8012    
    registerElectricalMeasurementCluster();
#endif // SUPPORTS_HLW8012
    registerEndpoint();

    // Fill Basic cluster attributes
    memcpy(sBasicServerCluster.au8ManufacturerName, CLD_BAS_MANUF_NAME_STR, CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBasicServerCluster.au8ModelIdentifier, CLD_BAS_MODEL_ID_STR, CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBasicServerCluster.au8DateCode, CLD_BAS_DATE_STR, CLD_BAS_DATE_SIZE);
    memcpy(sBasicServerCluster.au8SWBuildID, CLD_BAS_SW_BUILD_STR, CLD_BAS_SW_BUILD_SIZE);
    sBasicServerCluster.eGenericDeviceType = E_CLD_BAS_GENERIC_DEVICE_TYPE_WALL_SWITCH;

    // Initialize OTA
    otaHandlers.initOTA(getEndpointId());

    // Enable reporting temperature
    enableAttributeReporting(GENERAL_CLUSTER_ID_DEVICE_TEMPERATURE_CONFIGURATION, E_CLD_DEVTEMPCFG_ATTR_ID_CURRENT_TEMPERATURE);

    // The cluster instance will be reporting voltage and power, including applied multiplier/divisor
    enableAttributeReporting(MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, E_CLD_ELECTMEAS_ATTR_ID_RMS_VOLATGE);
    enableAttributeReporting(MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, E_CLD_ELECTMEAS_ATTR_ID_AC_VOLTAGE_MULTIPLIER);
    enableAttributeReporting(MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, E_CLD_ELECTMEAS_ATTR_ID_AC_VOLTAGE_DIVISOR);
    enableAttributeReporting(MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, E_CLD_ELECTMEAS_ATTR_ID_ACTIVE_POWER);
    enableAttributeReporting(MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, E_CLD_ELECTMEAS_ATTR_ID_AC_POWER_MULTIPLIER);
    enableAttributeReporting(MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT, E_CLD_ELECTMEAS_ATTR_ID_AC_POWER_DIVISOR);

    // Applying a 100 multiplicator for voltage. Thus 220V becomes 22000, which perfectly fits 16 bit
    sElectricalMeasurementServerCluster.u16ACVoltageMultiplier = 1;
    sElectricalMeasurementServerCluster.u16ACVoltageDivisor = 100;

    // Applying a 10 multiplicator for power. Thus maximum power 5000W become 50000, which still fits 16 bit integer
    sElectricalMeasurementServerCluster.u16ACPowerMultiplier = 1;
    sElectricalMeasurementServerCluster.u16ACPowerDivisor = 10;
}

void BasicClusterEndpoint::enableAttributeReporting(uint16 clusterID, uint16 attributeId)
{
    teZCL_Status status = eZCL_SetReportableFlag(getEndpointId(), clusterID, TRUE, FALSE, attributeId);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "Failed to enable reporting for cluster=%04x attribute=%04x, status: %02x\n", clusterID, attributeId, status);
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

void BasicClusterEndpoint::updatePowerMeasurements(float voltage, float power)
{
    // Save voltage value, applying a 100 multiplicator (so that 220V becomes 22000, which perfectly fits 16 bit)
    sElectricalMeasurementServerCluster.u16RMSVoltage = (uint16)(voltage * 100);
    sElectricalMeasurementServerCluster.u16ACVoltageMultiplier = 1;
    sElectricalMeasurementServerCluster.u16ACVoltageDivisor = 100;

    // Save power value, applying a 10 multiplicator (so that maximum power 5000W become 50000, which still fits 16 bit integer)
    sElectricalMeasurementServerCluster.i16ActivePower = (uint16)(power * 10);
    sElectricalMeasurementServerCluster.u16ACPowerMultiplier = 1;
    sElectricalMeasurementServerCluster.u16ACPowerDivisor = 10;

    // Prevent bothering Zigbee API if not connected
    if(!ZigbeeDevice::getInstance()->isJoined())
    {
        DBG_vPrintf(TRUE, "Device has not yet joined the network. Ignore power consumption reporting\n");
        return;
    }

    // Destination address - 0x0000 (coordinator)
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_SHORT;

    // Send the report
    DBG_vPrintf(TRUE, "Reporting electrical measurements Voltage=%d Power=%d... ", 
        sElectricalMeasurementServerCluster.u16RMSVoltage, 
        sElectricalMeasurementServerCluster.i16ActivePower);
    PDUM_thAPduInstance myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
    teZCL_Status status = eZCL_ReportAttribute(&addr,
                                               MEASUREMENT_AND_SENSING_CLUSTER_ID_ELECTRICAL_MEASUREMENT,
                                               E_CLD_ELECTMEAS_ATTR_ID_RMS_VOLATGE,
                                               getEndpointId(),
                                               1,
                                               myPDUM_thAPduInstance);
    PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
    DBG_vPrintf(TRUE, "status: %02x\n", status);

}
