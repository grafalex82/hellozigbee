const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const e = exposes.presets;
const ea = exposes.access;

// A subset of data types defined in dataType.ts (zigbee-herdsman project)
const DataType = {
    uint16: 0x21,
    enum8: 0x30,
}

const switchModesValues = ['toggle', 'momentary'];
const switchActionValues = ['onOff', 'offOn', 'toggle'];
const relayModeValues = ['unlinked', 'front', 'single', 'double', 'tripple', 'long'];
const buttonModeValues = ['simple', 'smart'];


const manufacturerOptions = {
    jennic : {manufacturerCode: 0x1037}
}

const getKey = (object, value) => {
    for (const key in object) {
        if (object[key] == value) return key;
    }
};

const fromZigbeeConverter = {
    cluster: 'genOnOffSwitchCfg',
    type: ['attributeReport', 'readResponse'],

    convert: (model, msg, publish, options, meta) => {

        meta.logger.debug(`+_+_+_ fromZigbeeConverter() msg.endpoint=[${JSON.stringify(msg.endpoint)}], msg.device=[${JSON.stringify(msg.device)}], model=[${JSON.stringify(model)}]`);

        const button = getKey(model.endpoint(msg.device), msg.endpoint.ID);
        const result = {};

        // switch mode
        if(msg.data.hasOwnProperty('65280')) {
            result[`switch_mode_${button}`] = switchModesValues[msg.data['65280']];
        }

        // switch action
        if(msg.data.hasOwnProperty('switchActions')) { // use standard 'switchActions' attribute identifier
            result[`switch_action_${button}`] = switchActionValues[msg.data['switchActions']];
        }

        // relay mode
        if(msg.data.hasOwnProperty('65281')) {
            result[`relay_mode_${button}`] = relayModeValues[msg.data['65281']];
        }

        // button mode
        if(msg.data.hasOwnProperty('65282')) {
            result[`button_mode_${button}`] = buttonModeValues[msg.data['65282']];
        }

        // Maximum pause between button clicks to be treates a single multiclick
        if(msg.data.hasOwnProperty('65283')) {
            result[`max_pause_${button}`] = msg.data['65283'];
        }

        // Munimal duration for the long press
        if(msg.data.hasOwnProperty('65284')) {
            result[`min_long_press_${button}`] = msg.data['65284'];
        }

        return result;
    },
}


const toZigbeeConverter = {
    key: ['switch_mode', 'switch_actions', 'relay_mode', 'button_mode', 'max_pause', 'min_long_press'],

    convertGet: async (entity, key, meta) => {
        meta.logger.debug(`+_+_+_ toZigbeeConverter::convertGet() key=${key}, entity=[${JSON.stringify(entity)}]`);
        await entity.read('genOnOffSwitchCfg', [65280, 65281, 65282, 65283, 65284], manufacturerOptions.jennic);
        await entity.read('genOnOffSwitchCfg', ['switchActions']);
    },

    convertSet: async (entity, key, value, meta) => {

        meta.logger.debug(`+_+_+_ toZigbeeConverter::convertSet() key=${key}, value=[${value}], entity=[${JSON.stringify(entity)}]`);

        let payload = {};
        let newValue = value;

        switch(key) {
            case 'switch_mode':
                newValue = switchModesValues.indexOf(value);
                payload = {65280: {value: newValue, type: DataType.enum8}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'switch_actions':
                newValue = switchActionValues.indexOf(value);
                payload = {switchActions: {value: newValue}};
                await entity.write('genOnOffSwitchCfg', payload);
                break;

            case 'relay_mode':
                newValue = relayModeValues.indexOf(value);
                payload = {65281: {value: newValue, type: DataType.enum8}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'button_mode':
                newValue = buttonModeValues.indexOf(value)
                payload = {65282: {value: newValue, type: DataType.enum8}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;

            case 'max_pause':
                payload = {65283: {value: value, type: DataType.uint16}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;
            case 'min_long_press':
                payload = {65284: {value: value, type: DataType.uint16}};
                await entity.write('genOnOffSwitchCfg', payload, manufacturerOptions.jennic);
                break;
            default:
                meta.logger.debug(`convertSet(): Unrecognized key=${key} (value=${value})`);
                break;
        }

        return {state: {switch_mode_button_1: value}};
    },
}



/*
    diyruz_freepad_config: {
        cluster: 'genOnOffSwitchCfg',
        type: ['readResponse'],
        convert: (model, msg, publish, options, meta) => {
            const button = getKey(model.endpoint(msg.device), msg.endpoint.ID);
            const {switchActions, switchType} = msg.data;
            const switchTypesLookup = ['toggle', 'momentary', 'multifunction'];
            const switchActionsLookup = ['on', 'off', 'toggle'];
            return {
                [`switch_type_${button}`]: switchTypesLookup[switchType],
                [`switch_actions_${button}`]: switchActionsLookup[switchActions],
            };
        },
    },


    aqara_opple_operation_mode: {
        key: ['operation_mode'],
        convertSet: async (entity, key, value, meta) => {
            // modes:
            // 0 - 'command' mode. keys send commands. useful for binding
            // 1 - 'event' mode. keys send events. useful for handling
            const lookup = {command: 0, event: 1};
            const endpoint = meta.device.getEndpoint(1);
            await endpoint.write('aqaraOpple', {'mode': lookup[value.toLowerCase()]}, {manufacturerCode: 0x115f});
            return {state: {operation_mode: value.toLowerCase()}};
        },
        convertGet: async (entity, key, meta) => {
            const endpoint = meta.device.getEndpoint(1);
            await endpoint.read('aqaraOpple', ['mode'], {manufacturerCode: 0x115f});
        },
    },


    hue_motion_sensitivity: {
        cluster: 'msOccupancySensing',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            if (msg.data.hasOwnProperty('48')) {
                const lookup = ['low', 'medium', 'high'];
                return {motion_sensitivity: lookup[msg.data['48']]};
            }
        },
    },


    hue_motion_sensitivity: {
        // motion detect sensitivity, philips specific
        key: ['motion_sensitivity'],
        convertSet: async (entity, key, value, meta) => {
            // hue_sml:
            // 0: low, 1: medium, 2: high (default)
            // make sure you write to second endpoint!
            const lookup = {'low': 0, 'medium': 1, 'high': 2};
            value = value.toLowerCase();
            utils.validateValue(value, Object.keys(lookup));

            const payload = {48: {value: lookup[value], type: 32}};
            await entity.write('msOccupancySensing', payload, manufacturerOptions.hue);
            return {state: {motion_sensitivity: value}};
        },
        convertGet: async (entity, key, meta) => {
            await entity.read('msOccupancySensing', [48], manufacturerOptions.hue);
        },
    },
    hue_motion_led_indication: {
        key: ['led_indication'],
        convertSet: async (entity, key, value, meta) => {
            const payload = {0x0033: {value, type: 0x10}};
            await entity.write('genBasic', payload, manufacturerOptions.hue);
            return {state: {led_indication: value}};
        },
        convertGet: async (entity, key, meta) => {
            await entity.read('genBasic', [0x0033], manufacturerOptions.hue);
        },
    },

*/


/*

    hue_motion_sensitivity: {
        cluster: 'msOccupancySensing',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            if (msg.data.hasOwnProperty('48')) {
                const lookup = ['low', 'medium', 'high'];
                return {motion_sensitivity: lookup[msg.data['48']]};
            }
        },
    },
*/


function genEndpoint(epName) {
    return [
        e.switch().withEndpoint(epName),
        exposes.enum('switch_mode', ea.ALL, ['toggle', 'momentary']).withEndpoint(epName),
        exposes.enum('switch_actions', ea.ALL, ['on', 'off', 'toggle']).withEndpoint(epName),
        exposes.enum('relay_mode', ea.ALL, ['unlinked', 'front', 'single', 'double', 'tripple', 'long']).withEndpoint(epName),
        exposes.enum('button_mode', ea.ALL, ['simple', 'smart']).withEndpoint(epName),
        exposes.numeric('max_pause', ea.ALL).withEndpoint(epName),
        exposes.numeric('min_long_press', ea.ALL).withEndpoint(epName),
    ]
}

function genEndpoints(endpoinsCount) {
    let features = [];

    for (let i = 1; i <= endpoinsCount; i++) {
        const epName = `button_${i}`;
        features.push(...genEndpoint(epName));
    }

    return features;
}


const device = {
    zigbeeModel: ['Hello Zigbee Switch'],
    model: 'Hello Zigbee Switch',
    vendor: 'NXP',
    description: 'Hello Zigbee Switch',
    fromZigbee: [fz.on_off, fromZigbeeConverter],
    toZigbee: [tz.on_off, toZigbeeConverter],
//    exposes: [ e.battery() /*, e.action(['*_single', '*_double', '*_triple', '*_quadruple', '*_release'])*/].concat(genEndpoints(1)),
    exposes: genEndpoints(1),
    endpoint: (device) => {
        return {button_1: 2};
    },
};

module.exports = device;





/*
    {
        exposes: [e.battery(), e.action(['*_single', '*_double', '*_triple', '*_quadruple', '*_release'])].concat(

((enpoinsCount) => {
            const features = [];
            for (let i = 1; i <= enpoinsCount; i++) {
                const epName = `button_${i}`;
                features.push(exposes.enum('switch_type', ea.ALL, ['toggle', 'momentary', 'multifunction']).withEndpoint(epName));
                features.push(exposes.enum('switch_actions', ea.ALL, ['on', 'off', 'toggle']).withEndpoint(epName));
            }
            return features;
        }

)(20)

),
        toZigbee: [tz.diyruz_freepad_on_off_config, tz.factory_reset],
    },
*/
/*
    {
        exposes: [e.battery(), e.action(['*_single', '*_double', '*_triple', '*_quadruple', '*_release'])].concat(((enpoinsCount) => {
            const features = [];
            for (let i = 1; i <= enpoinsCount; i++) {
                const epName = `button_${i}`;
                features.push(
                    exposes.enum('switch_type', ea.ALL, ['toggle', 'momentary', 'multifunction']).withEndpoint(epName));
                features.push(exposes.enum('switch_actions', ea.ALL, ['on', 'off', 'toggle']).withEndpoint(epName));
            }
            return features;
        })(8)),
        toZigbee: [tz.diyruz_freepad_on_off_config, tz.factory_reset],
    },
*/