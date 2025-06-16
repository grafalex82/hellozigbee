#ifndef IPOWERMEASUREMENTS_H
#define IPOWERMEASUREMENTS_H

class IPowerMeasurementsConsumer
{
public:
    virtual void updatePowerMeasurements(float voltage, float power);
};

#endif //IPOWERMEASUREMENTS_H
