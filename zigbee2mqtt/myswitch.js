const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const utils = require('zigbee-herdsman-converters/lib/utils');

const ota = require('zigbee-herdsman-converters/lib/ota')
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
const longPressModeValues = ['none', 'levelCtrlUp', 'levelCtrlDown'];
const operationModeValues = ['server', 'client'];
const interlockModeValues = ['none', 'mutualExclusion', 'opposite'];


const manufacturerOptions = {
    jennic : {manufacturerCode: 0x1037}
}

const getKey = (object, value) => {
    for (const key in object) {
        if (object[key] == value) return key;
    }
};

function getInterlockEp(ep) {
    // endpoints 2 (left) and 3 (right) are interlocked to each other
    if (ep == 2) return 3;
    if (ep == 3) return 2;
    return null;
}

const fromZigbee_OnOffSwitchCfg = {
    cluster: 'genOnOffSwitchCfg',
    type: ['attributeReport', 'readResponse'],

    convert: (model, msg, publish, options, meta) => {
        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() msg.endpoint=[${JSON.stringify(msg.endpoint)}], msg.device=[${JSON.stringify(msg.device)}]`);
        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() model=[${JSON.stringify(model)}]`);
        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() msg=[${JSON.stringify(msg)}]`);
        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() publish=[${JSON.stringify(publish)}]`);
        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() options=[${JSON.stringify(options)}]`);

        const ep_name = getKey(model.endpoint(msg.device), msg.endpoint.ID);
        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() model endpoint=[${JSON.stringify(model.endpoint(msg.device))}}]`);
        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() epName=[${ep_name}}]`);
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

        // Long press mode
        if(msg.data.hasOwnProperty('65284')) {
            result[`long_press_mode_${ep_name}`] = longPressModeValues[msg.data['65284']];
        }

        // Operation mode
        if(msg.data.hasOwnProperty('65285')) {
            result[`operation_mode_${ep_name}`] = operationModeValues[msg.data['65285']];
        }

        // Interlock mode
        if(msg.data.hasOwnProperty('65286')) {
            result[`interlock_mode_${ep_name}`] = interlockModeValues[msg.data['65286']];
        }

        // meta.logger.debug(`+_+_+_ fromZigbeeConverter() result=[${JSON.stringify(result)}]`);
        return result;
    },
}


const toZigbee_OnOffSwitchCfg = {
    key: ['switch_mode', 'switch_actions', 'relay_mode', 'max_pause', 'min_long_press', 'long_press_mode', 'operation_mode', 'interlock_mode'],

    convertGet: async (entity, key, meta) => {
        // meta.logger.debug(`+_+_+_ toZigbeeConverter::convertGet() key=${key}, entity=[${JSON.stringify(entity)}]`);

        if(key == 'switch_actions') {
            // meta.logger.debug(`+_+_+_ #1 getting value for key=[${key}]`);
            await entity.read('genOnOffSwitchCfg', ['switchActions']);
        }
        else {
            const lookup = {
                switch_mode: 65280,
                relay_mode: 65281,
                max_pause: 65282,
                min_long_press: 65283,
                long_press_mode: 65284,
                operation_mode: 65285,
                interlock_mode: 65286,
            };
            // meta.logger.debug(`+_+_+_ #2 getting value for key=[${lookup[key]}]`);
            await entity.read('genOnOffSwitchCfg', [lookup[key]], manufacturerOptions.jennic);
        }
    },

    convertSet: async (entity, key, value, meta) => {

        // meta.logger.debug(`+_+_+_ toZigbeeConverter::convertSet() key=${key}, value=[${value}], epName=[${meta.endpoint_name}], entity=[${JSON.stringify(entity)}]`);

        let payload = {};
        let newValue = value;

        switch(key) {
            case 'switch_mode':
                newValue = switchModeValues.indexOf(value);
                payload = {65280: {'value': newValue, 'type': DataType.enum8}};
                // meta.logger.debug(`payload=[${JSON.stringify(payload)}]`);
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'switch_actions':
                newValue = switchActionValues.indexOf(value);
                payload = {switchActions: newValue};
                // meta.logger.debug(`payload=[${JSON.stringify(payload)}]`);
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

            case 'long_press_mode':
                newValue = longPressModeValues.indexOf(value);
                payload = {65284: {'value': newValue, 'type': DataType.enum8}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'operation_mode':
                newValue = operationModeValues.indexOf(value);
                payload = {65285: {'value': newValue, 'type': DataType.enum8}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;
    
            case 'interlock_mode':
                newValue = interlockModeValues.indexOf(value);
                payload = {65286: {'value': newValue, 'type': DataType.enum8}};
                // Intentionally no `await` as interlocked endpoint may send state change message
                // and tests will go out of sync.
                /*await*/ entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);

                // Schedule reading of buddy endpoint setting to update setting on the Z2M dashboard.
                // Intentionally no `await` here to keep the order of requests for automated tests
                // Just schedule the read which will happen some time later.
                const interlockEp = getInterlockEp(entity.ID);
                if(interlockEp)                     
                    /*await*/ meta.device.getEndpoint(interlockEp).read('genOnOffSwitchCfg', [65286], manufacturerOptions.jennic);
                break;
    
            default:
                meta.logger.debug(`convertSet(): Unrecognized key=${key} (value=${value})`);
                break;
        }

        result = {state: {[key]: value}}
        meta.logger.debug(`result = [${JSON.stringify(result)}]`);
        return result;
    },
}

function addSwitchSettings(sw) {
    // const switch_mode_description = `Toggle - each press toggles the light state (fastest feedback).
    // Momentary - activates on button press, deactivates on release.
    // Multifunction - smart mode, supports single, double, triple, and long presses with customizable actions.`;
    sw.withFeature(e.enum('switch_mode', ea.ALL, switchModeValues));

    // const switch_actions_description = `onOff - activates on press, deactivates on release.
	// offOn - deactivates on press, activates on release.
	// toggle - toggles state with each press or release.`;
    sw.withFeature(e.enum('switch_actions', ea.ALL, switchActionValues));

    // const relay_mode_description = `unlinked - relay decoupled, button triggers only logical actions.
	// front - relay toggles on initial press for immediate response.
    // single - relay toggles on one press, generates 'single' action.
	// double - relay toggles on two presses, generates 'double' action.
	// triple - relay toggles on three presses, generates 'triple' action.
	// long - relay toggles on press-and-hold, generates 'hold' action, and 'release' action upon button release`;
    sw.withFeature(e.enum('relay_mode', ea.ALL, relayModeValues));

    // const long_press_mode_description = `None - long press has no extra function, only emits 'hold' action.
	// levelCtrlUp - press to emit 'MoveUpWithOnOff', release for 'Stop'.
	// levelCtrlDown - press for 'MoveDownWithOnOff', release for 'Stop'.`;
    sw.withFeature(e.enum('long_press_mode', ea.ALL, longPressModeValues));

    // const max_pause_description = `Maximum time between button clicks so that consecutive clicks are consodered as a part of a multi-click action`;
    // sw.withFeature(e.numeric('max_pause', ea.ALL));

    // const min_long_press_description = `Defines the minimum duration for pressing the button to trigger a 'hold' action`;
    // sw.withFeature(e.numeric('min_long_press', ea.ALL));

    return sw;
}

function genSwitchEndpoint(epName, enableInterlock) {
    // Create composite section for switch settings
    let sw = e.composite(epName + " Button", epName, ea.ALL);
    sw.withDescription('Settings for the ' + epName + ' button');

    // The switch toggle
    sw.withFeature(e.binary('On/Off state', ea.ALL, 'ON', 'OFF').withValueToggle('TOGGLE').withProperty('state'));

    // Add the operation mode selector (applicable only for switch endpoints that allow server mode)
    // const operation_mode_description = `Server - the endpoint maintains internal state, generate reports on its change.
    // Client - the endpoint generates On/Off/Toggle commands to bound devices`;
    sw.withFeature(e.enum('operation_mode', ea.ALL, operationModeValues));

    // Add other switch settings
    addSwitchSettings(sw);

    if(enableInterlock) {
        // Add the interlock mode selector (applicable only for double-gang switch endpoints)
        // const interlock_mode_description = `None - Switch endpoints work independently.
        // Mutual Exclusion - two endpoints cannot be ON at the same time,
        // Opposite - two endpoints always have state opposite to each other.`;
        sw.withFeature(e.enum('interlock_mode', ea.ALL, interlockModeValues));
    }

    // Make sure whole created block acts as a group of parameters, rather than a single composite object
    sw.withProperty('').withEndpoint(epName);

    return sw;
}

function genBothButtonsEndpoint(epName) {
    // Create composite section for switch settings
    let sw = e.composite("Both Buttons", epName, ea.ALL);
    sw.withDescription('Settings for the both button pressed together');

    // Add other switch settings
    addSwitchSettings(sw);

    // Make sure whole created block acts as a group of parameters, rather than a single composite object
    sw.withProperty('').withEndpoint(epName);

    return sw;
}

function getGenericSettings() {
    return [e.device_temperature()];
}

function genSwitchActions(endpoints) {
    const actions = ['single', 'double', 'triple', 'hold', 'release'];
    return endpoints.flatMap(endpoint => actions.map(action => action + "_" + endpoint))
}

const fromZigbee_MultistateInput = {
    cluster: 'genMultistateInput',
    type: ['attributeReport', 'readResponse'],

    convert: (model, msg, publish, options, meta) => {
        const actionLookup = {0: 'release', 1: 'single', 2: 'double', 3: 'tripple', 255: 'hold'};
        const value = msg.data['presentValue'];
        const action = actionLookup[value];

        const result = {action: utils.postfixWithEndpointName(action, msg, model, meta)};
        // meta.logger.debug(`+_+_+_ Multistate::fromZigbee() result=[${JSON.stringify(result)}]`);
        return result;
    },
}

// Special handler used in automation testing to catch OnOff commands from the device, and then verify values in tests
const fromZigbee_OnOff = {
    cluster: 'genOnOff',
    type: ['commandOn', 'commandOff', 'commandToggle'],

    convert: (model, msg, publish, options, meta) => {
        meta.logger.debug(`+_+_+_ LevelCtrl::fromZigbee() result=[${JSON.stringify(msg)}]`);
        const cmd = msg['type'];
        const payload = msg['data'];
        const srcEp = msg['endpoint']['ID']

        const result = {debug: {command: cmd, payload: payload, endpoint: srcEp}};
        return result;
    },
}

// Special handler used in automation testing to catch LevelCtrl commands from the device, and then verify values in tests
const fromZigbee_LevelCtrl = {
    cluster: 'genLevelCtrl',
    type: ['commandMoveToLevel', 'commandMoveToLevelWithOnOff', 'commandMove', 'commandMoveWithOnOff', 
           'commandStop', 'commandStopWithOnOff', 'commandStep', 'commandStepWithOnOff'],

    convert: (model, msg, publish, options, meta) => {
        const cmd = msg['type'];
        const payload = msg['data'];
        const srcEp = msg['endpoint']['ID']

        const result = {debug: {command: cmd, endpoint: srcEp, payload: payload}};
        return result;
    },
}

const common_definition = {
    vendor: 'DIY',
    fromZigbee: [fz.on_off, fromZigbee_OnOffSwitchCfg, fromZigbee_MultistateInput, fromZigbee_OnOff, fromZigbee_LevelCtrl, fz.device_temperature],
    toZigbee: [tz.on_off, toZigbee_OnOffSwitchCfg],
    configure: async (device, coordinatorEndpoint, logger) => {
        for (const ep of device.endpoints) {
            if(ep.supportsInputCluster('genOnOff')) {
                await ep.read('genOnOff', ['onOff']);
            }
            if(ep.supportsOutputCluster('genOnOff')) {
                await ep.read('genOnOffSwitchCfg', ['switchActions']);
                await ep.read('genOnOffSwitchCfg', [65280, 65281, 65282, 65283, 65284, 65285], manufacturerOptions.jennic);
            }
        }
    },
    meta: {multiEndpoint: true},
    ota: ota.zigbeeOTA
};

const definitions = [
    {
        ...common_definition,
        zigbeeModel: ['hello.zigbee.E75-2G4M10S'],
        model: 'E75-2G4M10S',
        description: 'Hello Zigbee Switch based on E75-2G4M10S module',
        exposes: [
            e.action(genSwitchActions(["left", "right", "both"])),
            genSwitchEndpoint("left", true),
            genSwitchEndpoint("right", true),
            genBothButtonsEndpoint("both"),
            ...getGenericSettings()
        ],
        endpoint: (device) => {
            return {
                "common": 1,
                "left": 2,
                "right": 3,
                "both": 4
            };
        },
    },
    {
        ...common_definition,
        zigbeeModel: ['hello.zigbee.QBKG11LM'],
        model: 'QBKG11LM',
        description: 'Hello Zigbee Switch firmware for Aqara QBKG11LM',
        exposes: [
            e.action(genSwitchActions(["button"])),
            genSwitchEndpoint("button", false),
            ...getGenericSettings()
        ],
        endpoint: (device) => {
            return {
                "common": 1,
                "button": 2
            };
        },
    },
    {
        ...common_definition,
        zigbeeModel: ['hello.zigbee.QBKG12LM'],
        model: 'QBKG12LM',
        description: 'Hello Zigbee Switch firmware for Aqara QBKG12LM',
        exposes: [
            e.action(genSwitchActions(["left", "right", "both"])),
            genSwitchEndpoint("left", true),
            genSwitchEndpoint("right", true),
            genBothButtonsEndpoint("both"),
            ...getGenericSettings()
        ],
        endpoint: (device) => {
            return {
                "common": 1,
                "left": 2,
                "right": 3,
                "both": 4
            };
        },
    },
];

module.exports = definitions;
