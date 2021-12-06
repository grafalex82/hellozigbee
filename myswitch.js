const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const utils = require('zigbee-herdsman-converters/lib/utils');

const ota = require('zigbee-herdsman-converters/lib/ota')
const otacommon = require('zigbee-herdsman-converters/lib/ota/common')
const assert = require('assert');
const fs = require('fs');
const crypto = require('crypto');
const path = require('path');

const e = exposes.presets;
const ea = exposes.access;

// A subset of data types defined in dataType.ts (zigbee-herdsman project)
const DataType = {
    uint16: 0x21,
    enum8: 0x30,
}

const switchModeValues = ['toggle', 'momentary', 'multifunction'];
const switchActionValues = ['onOff', 'offOn', 'toggle'];
const relayModeValues = ['unlinked', 'front', 'single', 'double', 'tripple', 'long'];


const manufacturerOptions = {
    jennic : {manufacturerCode: 0x1037}
}

const getKey = (object, value) => {
    for (const key in object) {
        if (object[key] == value) return key;
    }
};

const fromZigbee_OnOffSwitchCfg = {
    cluster: 'genOnOffSwitchCfg',
    type: ['attributeReport', 'readResponse'],

    convert: (model, msg, publish, options, meta) => {

        meta.logger.debug(`+_+_+_ fromZigbeeConverter() msg.endpoint=[${JSON.stringify(msg.endpoint)}], msg.device=[${JSON.stringify(msg.device)}]`);
        meta.logger.debug(`+_+_+_ fromZigbeeConverter() model=[${JSON.stringify(model)}]`);
        meta.logger.debug(`+_+_+_ fromZigbeeConverter() msg=[${JSON.stringify(msg)}]`);
        meta.logger.debug(`+_+_+_ fromZigbeeConverter() publish=[${JSON.stringify(publish)}]`);
        meta.logger.debug(`+_+_+_ fromZigbeeConverter() options=[${JSON.stringify(options)}]`);

        const ep_name = getKey(model.endpoint(msg.device), msg.endpoint.ID);
        const result = {};

        // switch type
        if(msg.data.hasOwnProperty('65280')) {
            result[`switch_mode_${ep_name}`] = switchModeValues[msg.data['65280']];
        }

        // switch action
        if(msg.data.hasOwnProperty('switchActions')) { // use standard 'switchActions' attribute identifier
            result[`switch_actions_${ep_name}`] = switchActionValues[msg.data['switchActions']];
        }

        // relay mode
        if(msg.data.hasOwnProperty('65281')) {
            result[`relay_mode_${ep_name}`] = relayModeValues[msg.data['65281']];
        }


        // Maximum pause between button clicks to be treates a single multiclick
        if(msg.data.hasOwnProperty('65282')) {
            result[`max_pause_${ep_name}`] = msg.data['65282'];
        }

        // Minimal duration for the long press
        if(msg.data.hasOwnProperty('65283')) {
            result[`min_long_press_${ep_name}`] = msg.data['65283'];
        }

        meta.logger.debug(`+_+_+_ fromZigbeeConverter() result=[${JSON.stringify(result)}]`);
        return result;
    },
}


const toZigbee_OnOffSwitchCfg = {
    key: ['switch_mode', 'switch_actions', 'relay_mode', 'max_pause', 'min_long_press'],

    convertGet: async (entity, key, meta) => {
        meta.logger.debug(`+_+_+_ toZigbeeConverter::convertGet() key=${key}, entity=[${JSON.stringify(entity)}]`);

        if(key == 'switch_actions') {
            meta.logger.debug(`+_+_+_ #1 getting value for key=[${key}]`);
            await entity.read('genOnOffSwitchCfg', ['switchActions']);
        }
        else {
            const lookup = {
                switch_mode: 65280,
                relay_mode: 65281,
                max_pause: 65282,
                min_long_press: 65283
            };
            meta.logger.debug(`+_+_+_ #2 getting value for key=[${lookup[key]}]`);
            await entity.read('genOnOffSwitchCfg', [lookup[key]], manufacturerOptions.jennic);
        }
    },

    convertSet: async (entity, key, value, meta) => {

        meta.logger.debug(`+_+_+_ toZigbeeConverter::convertSet() key=${key}, value=[${value}], epName=[${meta.endpoint_name}], entity=[${JSON.stringify(entity)}]`);

        let payload = {};
        let newValue = value;

        switch(key) {
            case 'switch_mode':
                newValue = switchModeValues.indexOf(value);
                payload = {65280: {'value': newValue, 'type': DataType.enum8}};
                meta.logger.debug(`payload=[${JSON.stringify(payload)}]`);
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'switch_actions':
                newValue = switchActionValues.indexOf(value);
                payload = {switchActions: newValue};
                meta.logger.debug(`payload=[${JSON.stringify(payload)}]`);
                await entity.write('genOnOffSwitchCfg', payload);
                break;

            case 'relay_mode':
                newValue = relayModeValues.indexOf(value);
                payload = {65281: {'value': newValue, 'type': DataType.enum8}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'max_pause':
                payload = {65282: {'value': value, 'type': DataType.uint16}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'min_long_press':
                payload = {65283: {'value': value, 'type': DataType.uint16}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            default:
                meta.logger.debug(`convertSet(): Unrecognized key=${key} (value=${value})`);
                break;
        }

        result = {state: {[key]: value}}
        meta.logger.debug(`result2=[${JSON.stringify(result)}]`);
        return result;
    },
}


function genSwitchEndpoint(epName) {
    return [
        e.switch().withEndpoint(epName),
        exposes.enum('switch_mode', ea.ALL, switchModeValues).withEndpoint(epName),
        exposes.enum('switch_actions', ea.ALL, switchActionValues).withEndpoint(epName),
        exposes.enum('relay_mode', ea.ALL, relayModeValues).withEndpoint(epName),
        exposes.numeric('max_pause', ea.ALL).withEndpoint(epName),
        exposes.numeric('min_long_press', ea.ALL).withEndpoint(epName),
    ]
}

function genSwitchEndpoints(endpoinsCount) {
    let features = [];

    for (let i = 1; i <= endpoinsCount; i++) {
        const epName = `button_${i}`;
        features.push(...genSwitchEndpoint(epName));
    }

    return features;
}


function genSwitchActions(endpoinsCount) {
    let actions = [];

    for (let i = 1; i <= endpoinsCount; i++) {
        const epName = `button_${i}`;
        actions.push(... ['single', 'double', 'triple', 'hold', 'release'].map(action => action + "_" + epName));
    }

    return actions;
}

const fromZigbee_MultistateInput = {
    cluster: 'genMultistateInput',
    type: ['attributeReport', 'readResponse'],

    convert: (model, msg, publish, options, meta) => {
        const actionLookup = {0: 'release', 1: 'single', 2: 'double', 3: 'tripple', 255: 'hold'};
        const value = msg.data['presentValue'];
        const action = actionLookup[value];

        const result = {action: utils.postfixWithEndpointName(action, msg, model)};
        meta.logger.debug(`+_+_+_ Multistate::fromZigbee() result=[${JSON.stringify(result)}]`);
        return result;
    },
}

async function getImageMeta(current, logger, device) {
    logger.debug(`My getImageMeta()`);
    logger.debug(`device='${JSON.stringify(device)}'`);
    logger.debug(`current='${JSON.stringify(current)}'`);

    const modelId = device.modelID;

    const fileName = '/app/data/HelloZigbee.ota';
    const buffer = fs.readFileSync(fileName);
    const parsed = otacommon.parseImage(buffer);

    const hash = crypto.createHash('sha512');
    hash.update(buffer);

    const ret = {
        fileVersion: parsed.header.fileVersion,
        fileSize: parsed.header.totalImageSize,
        url: fileName,
        sha512: hash.digest('hex'),
    };

    logger.debug(`My getImageMeta(): ret='${JSON.stringify(ret)}'`);
    return ret;
}

async function getNewImage(current, logger, device, getImageMeta, downloadImage) {
    const meta = await getImageMeta(current, logger, device);
    logger.debug(`getNewImage for '${device.ieeeAddr}', meta ${JSON.stringify(meta)}`);

    const fileName = '/app/data/HelloZigbee.ota';
    const buffer = fs.readFileSync(fileName);
    const image = otacommon.parseImage(buffer);

    logger.debug(`getNewImage for '${device.ieeeAddr}', image header ${JSON.stringify(image.header)}`);

    if ('minimumHardwareVersion' in image.header && 'maximumHardwareVersion' in image.header) {
        assert(image.header.minimumHardwareVersion <= device.hardwareVersion &&
            device.hardwareVersion <= image.header.maximumHardwareVersion, 'Hardware version mismatch');
    }
    return image;
}

const MyOta = {
    isUpdateAvailable: async (device, logger, requestPayload=null) => {
        logger.debug(`My isUpdateAvailable()`);
        return otacommon.isUpdateAvailable(device, logger, otacommon.isNewImageAvailable, requestPayload, getImageMeta);
    },

    updateToLatest: async (device, logger, onProgress) => {
        logger.debug(`My updateToLatest()`);
        return otacommon.updateToLatest(device, logger, onProgress, getNewImage, getImageMeta);
    }
}

const device = {
    zigbeeModel: ['Hello Zigbee Switch'],
    model: 'Hello Zigbee Switch',
    vendor: 'NXP',
    description: 'Hello Zigbee Switch',
    fromZigbee: [fz.on_off, fromZigbee_OnOffSwitchCfg, fromZigbee_MultistateInput],
    toZigbee: [tz.on_off, toZigbee_OnOffSwitchCfg],
    configure: async (device, coordinatorEndpoint, logger) => {
        device.endpoints.forEach(async (ep) => {
            await ep.read('genOnOff', ['onOff']);
            await ep.read('genOnOffSwitchCfg', ['switchActions']);
            await ep.read('genOnOffSwitchCfg', [65280, 65281, 65282, 65283], manufacturerOptions.jennic);
        });
    },
    exposes: [
        e.action(genSwitchActions(2)),
        ...genSwitchEndpoints(2)
    ],
    endpoint: (device) => {
        return {
            button_1: 2,
            button_2: 3
        };
    },
    meta: {multiEndpoint: true},
    ota: MyOta
};

module.exports = device;
