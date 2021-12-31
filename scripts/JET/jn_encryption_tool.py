#!/usr/bin/env python3

# Note: this file is decompiled from the official JET tool (a part of SDK). 
# Do not expect code beauty here.

import optparse
import os
import shutil
import struct
import sys
from typing import List, Tuple

from Crypto.Cipher import AES  # type: ignore

__VERSION__ = '1.1.11'


crctab = [
    0, 79764919, 159529838, 222504665,
    319059676, 398814059, 445009330, 507990021,
    638119352, 583659535, 797628118, 726387553,
    890018660, 835552979, 1015980042, 944750013,
    1276238704, 1221641927, 1167319070, 1095957929,
    1595256236, 1540665371, 1452775106, 1381403509,
    1780037320, 1859660671, 1671105958, 1733955601,
    2031960084, 2111593891, 1889500026, 1952343757,
    2552477408, 2632100695, 2443283854, 2506133561,
    2334638140, 2414271883, 2191915858, 2254759653,
    3190512472, 3135915759, 3081330742, 3009969537,
    2905550212, 2850959411, 2762807018, 2691435357,
    3560074640, 3505614887, 3719321342, 3648080713,
    3342211916, 3287746299, 3467911202, 3396681109,
    4063920168, 4143685023, 4223187782, 4286162673,
    3779000052, 3858754371, 3904687514, 3967668269,
    881225847, 809987520, 1023691545, 969234094,
    662832811, 591600412, 771767749, 717299826,
    311336399, 374308984, 453813921, 533576470,
    25881363, 88864420, 134795389, 214552010,
    2023205639, 2086057648, 1897238633, 1976864222,
    1804852699, 1867694188, 1645340341, 1724971778,
    1587496639, 1516133128, 1461550545, 1406951526,
    1302016099, 1230646740, 1142491917, 1087903418,
    2896545431, 2825181984, 2770861561, 2716262478,
    3215044683, 3143675388, 3055782693, 3001194130,
    2326604591, 2389456536, 2200899649, 2280525302,
    2578013683, 2640855108, 2418763421, 2498394922,
    3769900519, 3832873040, 3912640137, 3992402750,
    4088425275, 4151408268, 4197601365, 4277358050,
    3334271071, 3263032808, 3476998961, 3422541446,
    3585640067, 3514407732, 3694837229, 3640369242,
    1762451694, 1842216281, 1619975040, 1682949687,
    2047383090, 2127137669, 1938468188, 2001449195,
    1325665622, 1271206113, 1183200824, 1111960463,
    1543535498, 1489069629, 1434599652, 1363369299,
    622672798, 568075817, 748617968, 677256519,
    907627842, 853037301, 1067152940, 995781531,
    51762726, 131386257, 177728840, 240578815,
    269590778, 349224269, 429104020, 491947555,
    4046411278, 4126034873, 4172115296, 4234965207,
    3794477266, 3874110821, 3953728444, 4016571915,
    3609705398, 3555108353, 3735388376, 3664026991,
    3290680682, 3236090077, 3449943556, 3378572211,
    3174993278, 3120533705, 3032266256, 2961025959,
    2923101090, 2868635157, 2813903052, 2742672763,
    2604032198, 2683796849, 2461293480, 2524268063,
    2284983834, 2364738477, 2175806836, 2238787779,
    1569362073, 1498123566, 1409854455, 1355396672,
    1317987909, 1246755826, 1192025387, 1137557660,
    2072149281, 2135122070, 1912620623, 1992383480,
    1753615357, 1816598090, 1627664531, 1707420964,
    295390185, 358241886, 404320391, 483945776,
    43990325, 106832002, 186451547, 266083308,
    932423249, 861060070, 1041341759, 986742920,
    613929101, 542559546, 756411363, 701822548,
    3316196985, 3244833742, 3425377559, 3370778784,
    3601682597, 3530312978, 3744426955, 3689838204,
    3819031489, 3881883254, 3928223919, 4007849240,
    4037393693, 4100235434, 4180117107, 4259748804,
    2310601993, 2373574846, 2151335527, 2231098320,
    2596047829, 2659030626, 2470359227, 2550115596,
    2947551409, 2876312838, 2788305887, 2733848168,
    3165939309, 3094707162, 3040238851, 2985771188,
]


def ImageCRC(data: bytes, length: int):
    crc = 0xffffffff
    loop_count = 0
    while length:
        crc = crc << 8 & 0xffffff00 ^ crctab[
            (crc >> 24 & 0xff ^ data[loop_count])]
        length -= 1
        loop_count += 1

    return ~crc & 0xffffffff


def encryptFlashData(nonce: List[int], key: bytes, data: bytes, imageLen) \
        -> bytes:
    encyptedBlock = b''
    imageLen = len(data)
    if imageLen % 16 != 0:
        data = data + b'\xff' * (16 - imageLen % 16)

    r = AES.new(key, AES.MODE_ECB)
    for x in range(imageLen // 16):
        encryptNonce = ''
        for i in nonce:
            tempString = '%08x' % i
            y = 0
            while y < 8:
                encryptNonce = encryptNonce + chr(int(tempString[y:y + 2], 16))
                y = y + 2

        encChunk = r.encrypt(encryptNonce)
        if nonce[3] == 0xffffffff:
            nonce[3] = 0
        else:
            nonce[3] += 1
        chunk = data[x * 16:(x + 1) * 16]
        # loutChunk = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for i in range(16):
            # loutChunk[i] = chunk[i] ^ encChunk[i]
            encyptedBlock = encyptedBlock + bytes([chunk[i] ^ encChunk[i]])

    return encyptedBlock


def decryptFlashData(nonce: List[int], key: bytes, data: bytes, imageLen: int) \
        -> bytes:
    decyptedBlock = b''
    r = AES.new(key, AES.MODE_ECB)
    for x in range(imageLen // 16):
        encryptNonce: bytes = b''
        for i in nonce:
            tempString = '%08x' % i
            y = 0
            while y < 8:
                encryptNonce = encryptNonce + \
                               bytes([int(tempString[y:y + 2], 16)])
                y = y + 2

        encChunk = r.encrypt(encryptNonce)
        if nonce[3] == 0xffffffff:
            nonce[3] = 0
        else:
            nonce[3] += 1
        chunk = data[x * 16:(x + 1) * 16]
        # loutChunk = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for i in range(16):
            # loutChunk[i] = chunk[i] ^ encChunk[i]
            decyptedBlock = decyptedBlock + bytes([chunk[i] ^ encChunk[i]])

    return decyptedBlock


def aParsePassKeyString(sPassKey):
    lstu32Passkey = aParseNonce(sPassKey)
    abEncryptKey = struct.pack('>LLLL', *lstu32Passkey)
    return abEncryptKey


def aParseNonce(sNonceValue):
    lstu32Nonce = [0, 0, 0, 0]
    try:
        lstStrNonce = sNonceValue.split(',')
    except Exception:
        sNonceValue = '0x00000000, 0x00000000, 0x00000000, 0x00000000'
        lstStrNonce = sNonceValue.split(',')

    if len(lstStrNonce) == 4:
        for i in range(4):
            if '0x' in lstStrNonce[i]:
                lstu32Nonce[i] = int(lstStrNonce[i], 16)
            else:
                lstu32Nonce[i] = int(lstStrNonce[i], 10)

    return lstu32Nonce


def CerticomApp(sConfigFile):
    bStatus = True
    try:
        DataList, TotalLines = getPureDataLines(sConfigFile.strip())
        print(DataList)
        a, b = getPureDataLines(DataList[0].strip('\n'))
        p = open('tempprivate.txt', 'w')
        p.write(a[0])
        p.close()
        p = open('tempprivate.txt', 'r')
        private_key = p.readlines()
        private_key = private_key[0].strip('\n')
        p.close()
        print(('The private key : ', private_key))
        a, b = getPureDataLines(DataList[1].strip('\n'))
        p = open('tempmac.txt', 'w')
        p.write(a[0])
        p.close()
        p = open('tempmac.txt', 'r')
        mac_key = p.readlines()
        mac_key = mac_key[0].strip('\n')
        p.close()
        print(('The MAC id  : ', mac_key))
        a, b = getPureDataLines(DataList[2].strip('\n'))
        p = open('certificate.txt', 'w')
        p.write(a[0].strip('\n'))
        p.close()
        p = open('certificate.txt', 'r')
        certificate_key = p.readlines()
        certificate_key = certificate_key[0].strip('\n')
        p.close()
        print('The certificate : ', certificate_key)
        executeline = 'Certi.exe Untitled.bin tempprivate.txt ' \
                      'tempmac.txt certificate.txt'
        os.system(executeline)
    except Exception:
        bStatus = False
        print(' Could not apply the certificate\n')

    print('Done applying the certificate')
    return bStatus


def hextranslate(s: str) -> bytes:
    return bytes.fromhex(s)


def getPureDataLines(sFile) -> Tuple[list, int]:
    try:
        with open(sFile, 'r') as iFile:
            TempList = FileLines = iFile.readlines()
    except Exception:
        raise UserWarning('Could not open file' + sFile, None)

    Element = 0
    for sLine in FileLines:
        stripped_line = sLine.strip()
        if len(stripped_line) == 0:
            pass
        elif stripped_line[0] == '#':
            pass
        else:
            TempList[Element] = sLine
            Element += 1

    return TempList, Element


def CombinedFile(sSrcFilename, sConfigFile, iBootLoaderFlashHeaderType,
                 iDeviceType, iEnableEncrypt, iProgrammerType, sPassKey,
                 iPrivateEncrypt):
    bStatus = True
    sKeyString = sPassKey.strip()
    sIndexString = (
            '0x' + sKeyString[:8] + ',' +
            '0x' + sKeyString[8:16] + ',' +
            '0x' + sKeyString[16:24] + ',' +
            '0x' + sKeyString[24:32]
    )
    aIndexKey = aParsePassKeyString(sIndexString)
    try:
        with open(sSrcFilename, 'rb') as inputFile:
            image: bytes = inputFile.read()
        if iDeviceType == 3:
            sNonce = image[28:44].hex()
            print('sNonce "{0}" is accepted'.format(sNonce))
            offset = 44
            data = image[offset:]
        else:
            tempFileLines = 0
            TotalLines = 0
            DataList = []

            if iDeviceType == 4 or iDeviceType == 5:
                sNonce = image[20:36].hex()
                if iPrivateEncrypt != 1:
                    print(sNonce)
                    print('sNonce is accepted')
                offset = 36
                data = image[offset:]
            else:
                if iDeviceType == 6:
                    signaturevalue = image[36:40].hex()
                    signaturevalue = struct.pack('L', int(signaturevalue, 16))
                    signaturevalue = struct.unpack('>L', signaturevalue)[0]
                    if signaturevalue == 2554624258:
                        print('JN518x ES1')
                        sMagicLocation = image[40:44].hex()
                    else:
                        print('JN518x ES2')
                        sMagicLocation = image[36:40].hex()
                    sMagicLocation = struct.pack('L', int(sMagicLocation, 16))
                    lMagicLocation = struct.unpack('>L', sMagicLocation)[0]
                    lMagicLocation += 12
                    sImageSize = image[lMagicLocation:lMagicLocation + 4].hex()
                    sImageSize = struct.pack('L', int(sImageSize, 16))
                    lImageSize = struct.unpack('>L', sImageSize)[0]
                    sNonce = image[336:352].hex()
                    print(' Nonce : %s' % sNonce)
                    if iPrivateEncrypt != 1:
                        print('sNonce is accepted')
                    offset = 352
                    data = image[offset:]
                elif iBootLoaderFlashHeaderType == 2:
                    if iEnableEncrypt == 1:
                        offset = 40
                        code_length = image[42:44].hex()
                        LengthInInt = int(code_length, 16)
                        LengthInInt = LengthInInt * 4
                        PaddingRequired = 16 - LengthInInt % 16
                        LengthInInt = LengthInInt + PaddingRequired
                        LengthInInt = LengthInInt / 4
                        data = image[offset:42]
                        hexstr = '%02x' % LengthInInt
                        data += hextranslate(hexstr)
                        data += image[44:104]
                        data += image[104:]
                    else:
                        print(
                            'Encrypt Enable option not provided, '
                            'hence appropriate padding not provided\n',
                        )
                        offset = 40
                        data = image[offset:44]
                        data += image[44:104]
                        data += image[104:]
                else:
                    offset = 48
                    data = image[offset:]
                DataList, TotalLines = getPureDataLines(sConfigFile)
                tempFileData = DataList[0].split(',')
                tempDataFile, tempFileLines = getPureDataLines(
                    tempFileData[0].strip())
                previousLines = tempFileLines
                for n in range(1, TotalLines):
                    tempFileData = DataList[n].split(',')
                    tempDataFile, tempFileLines = getPureDataLines(
                        tempFileData[0].strip())
                    if previousLines == tempFileLines:
                        previousLines = tempFileLines
                    else:
                        return False

            for n in range(0, tempFileLines):
                currentLocation = 0
                new_data = ''
                tempData = ''
                nonce_data = ''
                for i in range(0, TotalLines):
                    CurrentFileData = DataList[i].split(',')
                    CurrentDataFile, CurrentFileLines = getPureDataLines(
                        CurrentFileData[0].strip())
                    CurrentDataSize = int(CurrentFileData[2].strip())
                    locationFromStart = int(CurrentFileData[1].strip(), 16)
                    print('Data size %d %d %d' % (
                        CurrentDataSize, locationFromStart, currentLocation))
                    if locationFromStart >= offset:
                        EncOffset = locationFromStart - offset
                        if currentLocation == 0:
                            new_data += data[0:EncOffset]
                        else:
                            new_data += data[currentLocation:EncOffset]
                        dataValue = CurrentDataFile[n].strip()
                        if CurrentDataSize == 8:
                            nonce_data = (
                                '0x{0},0x{1},0x00000000,0x00000000'.format(
                                    dataValue[:8],
                                    dataValue[8:16],
                                )
                            )
                            new_Nonce = aParseNonce(nonce_data)
                        if CurrentDataSize == 21 and iDeviceType == 4 and \
                                iPrivateEncrypt == 1:
                            for j in range(CurrentDataSize):
                                tempData += \
                                    dataValue[j * 2:2 + j * 2].encode().hex()

                            encryptedData = encryptFlashData(
                                new_Nonce,
                                aIndexKey,
                                tempData,
                                len(tempData),
                            )
                            new_data += encryptedData[0:21]
                        else:
                            for j in range(CurrentDataSize):
                                new_data += \
                                    dataValue[j * 2:2 + j * 2].encode().hex()

                        currentLocation = EncOffset + CurrentDataSize
                    if iDeviceType == 3:
                        if locationFromStart == 20:
                            macAddress = CurrentDataFile[n].strip()
                    elif iDeviceType == 4:
                        if locationFromStart == 68:
                            macAddress = CurrentDataFile[n].strip()
                    elif iDeviceType == 5:
                        if locationFromStart == 420:
                            macAddress = CurrentDataFile[n].strip()
                    elif iDeviceType == 6:
                        macAddress = 'FFFFFFFFFFFFFFFF'
                    elif iBootLoaderFlashHeaderType == 2:
                        if locationFromStart == 16:
                            macAddress = CurrentDataFile[n].strip()
                    elif locationFromStart == 48:
                        macAddress = CurrentDataFile[n].strip()

                new_data += data[currentLocation:]
                padding_required = 0
                if len(new_data) % 16 != 0:
                    for x in range(16 - len(new_data) % 16):
                        new_data = new_data + b'\xff'
                        padding_required += 1

                sDestFilename = 'output' + macAddress + '.bin'
                if iDeviceType == 1 or iDeviceType == 2:
                    sNonce = '00000000000000000000000000000000'
                outputFile = open(sDestFilename, 'wb')
                if iDeviceType == 3:
                    if iProgrammerType == 0:
                        outputFile.write(image[0:20])
                    else:
                        outputFile.write(image[4:20])
                    outputFile.write(hextranslate(macAddress))
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(new_data)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iDeviceType == 4 or iDeviceType == 5:
                    if iProgrammerType == 0:
                        outputFile.write(image[0:20])
                    else:
                        outputFile.write(image[4:20])
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(new_data)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iDeviceType == 6:
                    if iProgrammerType == 0:
                        outputFile.write(image[0:336])
                    else:
                        outputFile.write(image[0:336])
                    outputFile.write(hextranslate(sNonce))
                    lImageSize += padding_required
                    invlImageSize = hex(
                        struct.unpack('>L', struct.pack('<L', lImageSize))[0],
                    )[2:].upper().zfill(8)
                    outputFile.write(new_data[0:lMagicLocation - offset])
                    outputFile.write(hextranslate(str(invlImageSize)))
                    outputFile.write(new_data[lMagicLocation - offset + 4:])
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iBootLoaderFlashHeaderType == 2:
                    outputFile.write(image[0:16])
                    outputFile.write(hextranslate(macAddress))
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(new_data)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                else:
                    if iDeviceType == 2:
                        outputFile.write(image[0:8])
                    else:
                        outputFile.write(hextranslate('e1e1e1e1'))
                        outputFile.write(image[4:8])
                    LengthInInt = int(image[8:12].hex(), 16)
                    PaddingRequired = 16 - LengthInInt % 16
                    LengthInInt = LengthInInt + PaddingRequired
                    hexstr = '%08x' % LengthInInt
                    outputFile.write(hextranslate(hexstr))
                    outputFile.write(image[12:48])
                    outputFile.write(new_data)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
    except Exception:
        bStatus = False
        print(' ERROR : Could not process ' + sSrcFilename + ',' + sConfigFile)

    print('Done.')
    return bStatus


def otamerge(server_image: str, client_image: str, output_image: str,
             sConfigFile, ota_header: bool,
             sector_size, ota_hdr, ota_hdr_ext, ota_ext_hdr, header_size,
             total_image_size, sPassKey, embed_hdr, copy_mac, manufacturer,
             image_type, File_Version, iProgrammerType, sign_integrity,
             otahdroffset, image_crc, sNonce, OTA_Header_String):
    DevType = 0
    image_tag = 0
    WriteMac = False
    imagedata = b''
    element_hdr_s = struct.Struct('<HI')
    flash_header_s = struct.Struct('>3IBBH2I4I2H2H2I')
    print('sector size %d ' % sector_size)
    sNonceString = sNonce.strip()
    sNonceString = sNonceString.strip('0x')
    sIvString = (
            '0x' + sNonceString[:8] + ',' +
            '0x' + sNonceString[8:16] + ',' +
            '0x' + sNonceString[16:24] + ',' +
            '0x' + sNonceString[24:28] + '0000'
    )
    aParseNonce(sIvString)
    try:
        client_file = open(client_image, 'rb')
        clientfiledata = client_file.read()
        header = int(clientfiledata[0:4].hex(), 16)
        if header == 33947704:
            DevType = 3
        elif header == 117637128 or header == 251854859:
            DevType = 4
        elif header == 167772943 or header == 167772935 or header == 167772932:
            DevType = 5
        else:
            DevType = 6
        if DevType == 3 or DevType == 4 or DevType == 5 or DevType == 6:
            if DevType != 6:
                clientfiledata = clientfiledata[4:]
            temp_file = open('temp_client_file.bin', 'wb')
            temp_file.write(clientfiledata)
            temp_file.close()
            client_file.close()
            client_file = open('temp_client_file.bin', 'rb')
            clientfiledata = client_file.read()
        magic_number = int(clientfiledata[0:12].hex(), 16)
        if magic_number == 5634002657765593205201860488 and copy_mac is True:
            WriteMac = True
            print('Copying Image MAC to Bootloader MAC')
            macAddress = clientfiledata[16:24]
            print('macAddress :' + macAddress.hex())
            if sPassKey != '':
                sKeyString = sPassKey.strip()
                sPassString = (
                        '0x' + sKeyString[:8] + ',' +
                        '0x' + sKeyString[8:16] + ',' +
                        '0x' + sKeyString[16:24] + ',' +
                        '0x' + sKeyString[24:32]
                )
                aPassKey = aParsePassKeyString(sPassString)
                encodedMac = encryptFlashData(
                    [0x10, 0x11121314, 0x15161718, 0],
                    aPassKey,
                    macAddress,
                    len(macAddress),
                )
            else:
                encodedMac = clientfiledata[16:24]
    except IOError:
        print('Failed to open client image file')
        quit()

    manufacturer = hex(manufacturer)
    image_type = hex(image_type)
    # embedVersion = File_Version
    File_Version = hex(File_Version)
    manufacturer = manufacturer.replace('0x', '')
    image_type = image_type.replace('0x', '')
    File_Version = File_Version.replace('0x', '')
    manufacturer = manufacturer.upper()
    image_type = image_type.upper()
    File_Version = File_Version.upper()
    File_Version_Length = len(File_Version)
    if File_Version_Length != 8:
        while File_Version_Length < 8:
            File_Version = File_Version + '0'
            File_Version_Length = len(File_Version)

    if OTA_Header_String != '':
        OTA_Header_String = [ord(temp) for temp in OTA_Header_String]
        OTA_Header_String_Length = len(OTA_Header_String)
        if OTA_Header_String_Length != 32:
            print('Invalid OTA Header String Size')
            quit()
        OTA_Header_String = [hex(temp) for temp in OTA_Header_String]
        OTA_Header_String = [temp.replace('0x', '') for temp in
                             OTA_Header_String]
        OTA_Header_String = [temp.upper() for temp in OTA_Header_String]
        OTA_Header_String = ''.join(temp for temp in OTA_Header_String)
        print('OTA Header String Given: ' + OTA_Header_String)
    if output_image == 'out.bin':
        output_image = (
                manufacturer + '-' + image_type + '-' + File_Version +
                '-upgrademe.zigbee'
        )
    if server_image is not None:
        try:
            if WriteMac and DevType != 6:
                try:
                    with open(server_image, 'rb') as output_file:
                        outputfiledata = output_file.read()

                    with open('interim.bin', 'wb') as usertemp:
                        usertemp.write(outputfiledata[0:48])
                        usertemp.write(encodedMac)
                        if sPassKey != '':
                            usertemp.write(outputfiledata[64:])
                        else:
                            usertemp.write(outputfiledata[56:])
                    shutil.copyfile('interim.bin', output_image)
                    os.remove('interim.bin')
                except Exception:
                    print('Failed to open file' + server_image)
                    quit()

            else:
                shutil.copyfile(server_image, output_image)
        except IOError:
            print('Failed to copy server image file')
            quit()

    try:
        if server_image is not None:
            output_file = open(output_image, 'ab')
        else:
            output_file = open(output_image, 'wb')
    except IOError:
        raise IOError('Failed to open output image file')

    server_size = output_file.seek(0, 2)
    server_size = output_file.tell()
    padding = sector_size - server_size % sector_size
    if padding < sector_size:
        output_file.truncate(server_size + padding)
    client_file.seek(0)
    if DevType != 6:
        # flash_header = flash_header_s.unpack_from(
        flash_header_s.unpack_from(
            client_file.read(flash_header_s.size))
        # bss_offset = flash_header[14] * 4
    client_file.seek(0, 2)
    client_size = client_file.tell()
    print('Sizes:')
    print('|  Server  |OTA Header|  Client  | Total Client')
    print('|%10d|%10d|%10d|%10d' % (
        server_size, header_size, client_size, total_image_size))
    if DevType == 3 and iProgrammerType == 0:
        output_file.write(hextranslate('02060038'))
    if DevType == 4 and iProgrammerType == 0:
        if header == 117637128:
            output_file.write(hextranslate('07030008'))
        if header == 251854859:
            output_file.write(hextranslate('0f03000b'))
    if DevType == 5 and iProgrammerType == 0:
        if header == 167772943:
            output_file.write(hextranslate('0a00030f'))
        if header == 167772935:
            output_file.write(hextranslate('0a000307'))
        if header == 167772932:
            output_file.write(hextranslate('0a000304'))
    if ota_header:
        output_file.write(ota_hdr)
        if ota_ext_hdr:
            output_file.write(ota_hdr_ext)
        image_hdr = element_hdr_s.pack(image_tag, client_size)
        output_file.write(image_hdr)
    client_file.seek(0)
    # client_start = output_file.tell()
    if embed_hdr and DevType == 4:
        imagedata += client_file.read(otahdroffset - 4)
        imagedata += ota_hdr
        if ota_ext_hdr:
            imagedata += ota_hdr_ext
        client_file.seek(header_size, 1)
    if embed_hdr and DevType == 5:
        imagedata += client_file.read(otahdroffset - 4)
        imagedata += ota_hdr
        if ota_ext_hdr:
            imagedata += ota_hdr_ext
        client_file.seek(header_size, 1)
    if embed_hdr and DevType == 6:
        imagedata += client_file.read(otahdroffset)
        imagedata += ota_hdr
        if ota_ext_hdr:
            imagedata += ota_hdr_ext
        client_file.seek(header_size, 1)
    if embed_hdr and DevType != 4 and DevType != 5 and DevType != 6:
        imagedata += client_file.read(otahdroffset - 4)
        imagedata += ota_hdr
        if ota_ext_hdr:
            imagedata += ota_hdr_ext
        client_file.seek(header_size, 1)
    imagedata += client_file.read()
    if DevType == 6:
        startCrcOffset = 69
        endCrcOffset = 73
    else:
        startCrcOffset = 65
        endCrcOffset = 69
    if embed_hdr and image_crc:
        imagedata = (
            imagedata[:otahdroffset + startCrcOffset] +
            struct.pack('I', 0) +
            imagedata[otahdroffset + endCrcOffset:]
        )
        imagecrc = ImageCRC(imagedata, len(imagedata))
        print('\nimage crc: 0x%08X' % imagecrc)
        imagedata = (
            imagedata[:otahdroffset + startCrcOffset] +
            struct.pack('I', imagecrc) +
            imagedata[otahdroffset + endCrcOffset:]
        )
    output_file.write(imagedata)
    client_file.close()
    output_file.close()
    if DevType == 3 or DevType == 4 or DevType == 5 or DevType == 6:
        os.remove('temp_client_file.bin')
    if sConfigFile != 'no_option' or sign_integrity == 3:
        tempprivate = 'tempprivate.txt'
        tempmac = 'tempmac.txt'
        certificate = 'certificate.txt'
        if sign_integrity != 3:
            DataList, TotalLines = getPureDataLines(sConfigFile.strip())
            a, b = getPureDataLines(DataList[0].strip('\n'))
            with open('tempprivate.txt', 'w') as p:
                p.write(a[0])
            with open('tempprivate.txt', 'r') as p:
                private_key = p.readline().strip('\n')
            print('the private key : ', private_key)
            a, b = getPureDataLines(DataList[1].strip('\n'))
            with open('tempmac.txt', 'w') as p:
                p.write(a[0])
            with open('tempmac.txt', 'r') as p:
                mac_key = p.readline().strip('\n')
            print(('the MAC id  : ', mac_key))
            a, b = getPureDataLines(DataList[2].strip('\n'))
            with open('certificate.txt', 'w') as p:
                p.write(a[0].strip('\n'))
            with open('certificate.txt', 'r') as p:
                certificate_key = p.readline().strip('\n')
            print('the certificate : ', certificate_key)
        if DevType in (4, 5) and iProgrammerType == 0:
            with open(output_image, 'rb') as output_image_temp, \
                    open('interim.bin', 'wb') as usertemp:
                outputfiledata = output_image_temp.read()
                usertemp.write(outputfiledata[4:])
            with open(output_image, 'wb') as output_image_temp, \
                    open('interim.bin', 'rb') as usertemp:
                file_contents = usertemp.read()
                output_image_temp.write(file_contents)
            os.system('del interim.bin')
            executeline = 'Certi.exe {0} {1} {2} {3} {4}'.format(
                str(sign_integrity),
                output_image,
                tempprivate,
                tempmac,
                certificate,
            )

        elif DevType == 3 and iProgrammerType == 0:
            with open(output_image, 'rb') as output_image_temp, \
                    open('interim.bin', 'wb') as usertemp:
                outputfiledata = output_image_temp.read()
                usertemp.write(outputfiledata[4:])
            with open(output_image, 'wb') as output_image_temp, \
                    open('interim.bin', 'rb') as usertemp:
                file_contents = usertemp.read()
                output_image_temp.write(file_contents)
            os.system('del interim.bin')
            executeline = 'Certi.exe {0} {1} {2} {3} {4}'.format(
                str(sign_integrity),
                output_image,
                tempprivate,
                tempmac,
                certificate,
            )
        else:
            executeline = 'Certi.exe {0} {1} {2} {3} {4}'.format(
                str(sign_integrity),
                output_image,
                tempprivate,
                tempmac,
                certificate,
            )
        if os.system(executeline) != 0:
            raise UserWarning('Failed to Add Signature Element to Tag', None)
        if sign_integrity != 3:
            os.system('del certificate.txt')
            os.system('del tempmac.txt')
            os.system('del tempprivate.txt')
        if (DevType == 4 or DevType == 3 or DevType == 5) and\
                iProgrammerType == 0:
            temp_file_name = 'Signed_' + output_image
            pointer_file = open(temp_file_name, 'rb')
            file_contents = pointer_file.read()
            pointer_file.close()
            pointer_file = open(temp_file_name, 'wb')
            if DevType == 4 or DevType == 5:
                if header == 117637128:
                    pointer_file.write(hextranslate('07030008'))
                if header == 251854859:
                    pointer_file.write(hextranslate('0f03000b'))
                if header == 167772943:
                    pointer_file.write(hextranslate('0a00030f'))
                if header == 167772935:
                    pointer_file.write(hextranslate('0a000307'))
                if header == 167772932:
                    pointer_file.write(hextranslate('0a000304'))
            else:
                pointer_file.write(hextranslate('02060038'))
            pointer_file.write(file_contents)
            pointer_file.close()
    return True


def encryptCombinedFile(sSrcFilename, sConfigFile, sPassKey, iDeviceType,
                        iBootLoaderFlashHeaderType, sNonce, ota_header, ota_hdr,
                        ota_hdr_ext, ota_ext_hdr, header_size, iProgrammerType):
    bStatus = True
    sKeyString = sPassKey.strip()
    sPassString = (
        '0x' + sKeyString[:8] + ',' +
        '0x' + sKeyString[8:16] + ',' +
        '0x' + sKeyString[16:24] + ',' +
        '0x' + sKeyString[24:32]
    )
    aPassKey = aParsePassKeyString(sPassString)
    sNonceString = sNonce.strip()
    sIvString = (
            '0x' + sNonceString[:8] + ',' +
            '0x' + sNonceString[8:16] + ',' +
            '0x' + sNonceString[16:24] + ',' +
            '0x' + sNonceString[24:28] + '0000'
    )
    # aNonce = aParseNonce(sIvString)
    aParseNonce(sIvString)
    try:
        with open(sSrcFilename, 'rb') as inputFile:
            image = inputFile.read()
        if iDeviceType == 3:
            if sNonceString[28:32] != '0000':
                print('Warning : Lower 2 Bytes Ignored')
            offset = 44
            data = image[offset:46]
            data += image[46:108]
            if ota_header:
                data += ota_hdr
                hdrsize = 108 + header_size
                if ota_ext_hdr:
                    data += ota_hdr_ext
                data += image[hdrsize:]
            else:
                data += image[108:]
        else:
            if iDeviceType == 4 or iDeviceType == 5:
                if sNonceString[28:32] != '0000':
                    print('Warning : Lower 2 Bytes Ignored')
                offset = 36
                data = image[offset:46]
                data += image[46:84]
                if ota_header:
                    data += ota_hdr
                    hdrsize = 84 + header_size
                    if ota_ext_hdr:
                        data += ota_hdr_ext
                    data += image[hdrsize:]
                else:
                    data += image[84:]
            else:
                if iDeviceType == 6:
                    signaturevalue = image[36:40].hex()
                    signaturevalue = struct.pack('L', int(signaturevalue, 16))
                    signaturevalue = struct.unpack('>L', signaturevalue)[0]
                    if signaturevalue == 2554624258:
                        print('JN518x ES1')
                        sMagicLocation = image[40:44].hex()
                    else:
                        print('JN518x ES2')
                        sMagicLocation = image[36:40].hex()
                    sMagicLocation = struct.pack('L', int(sMagicLocation, 16))
                    lMagicLocation = struct.unpack('>L', sMagicLocation)[0]
                    lMagicLocation += 12
                    sImageSize = image[lMagicLocation:lMagicLocation + 4].hex()
                    sImageSize = struct.pack('L', int(sImageSize, 16))
                    lImageSize = struct.unpack('>L', sImageSize)[0]
                    sNonce = image[336:352].hex()
                    print(' Nonce : %s' % sNonce)
                    offset = 352
                    data = image[offset:]
                    padding_required = 0
                    if len(data) % 16 != 0:
                        padding_required = 16 - len(data) % 16
                    lImageSize += padding_required
                    invlImageSize = hex(
                        struct.unpack('>L', struct.pack('<L', lImageSize))[0])[
                                    2:].upper().zfill(8)
                    tempdata = image[0:lMagicLocation]
                    tempdata += hextranslate(str(invlImageSize))
                    tempdata += data[lMagicLocation + 4:]
                    data = tempdata[offset:]
                elif iBootLoaderFlashHeaderType == 2:
                    offset = 40
                    code_length = image[42:44].hex()
                    LengthInInt = int(code_length, 16)
                    LengthInInt = LengthInInt * 4
                    PaddingRequired = 16 - LengthInInt % 16
                    LengthInInt = LengthInInt + PaddingRequired
                    LengthInInt = LengthInInt / 4
                    data = image[offset:42]
                    hexstr = '%02x' % LengthInInt
                    data += hextranslate(hexstr)
                    data += image[44:104]
                    if ota_header:
                        data += ota_hdr
                        hdrsize = 104 + header_size
                        if ota_ext_hdr:
                            data += ota_hdr_ext
                        data += image[hdrsize:]
                    else:
                        data += image[104:]
                else:
                    offset = 48
                    data = image[offset:]
                print(' Start Encrypting ...')
                DataList, TotalLines = getPureDataLines(sConfigFile)
                tempFileData = DataList[0].split(',')
                tempDataFile, tempFileLines = getPureDataLines(
                    tempFileData[0].strip())
                previousLines = tempFileLines
                for n in range(1, TotalLines):
                    tempFileData = DataList[n].split(',')
                    tempDataFile, tempFileLines = getPureDataLines(
                        tempFileData[0].strip())
                    if previousLines == tempFileLines:
                        previousLines = tempFileLines
                    else:
                        print(' Number of entries mismatch at location %d' % n)
                        return False

            for n in range(0, tempFileLines):
                currentLocation = 0
                new_data = ''
                temp_nonce = aParseNonce(sIvString)
                for i in range(0, TotalLines):
                    CurrentFileData = DataList[i].split(',')
                    CurrentDataFile, CurrentFileLines = getPureDataLines(
                        CurrentFileData[0].strip())
                    CurrentDataSize = int(CurrentFileData[2].strip())
                    locationFromStart = int(CurrentFileData[1].strip(), 16)
                    if locationFromStart >= offset:
                        EncOffset = locationFromStart - offset
                        if currentLocation == 0:
                            new_data += data[0:EncOffset]
                        else:
                            new_data += data[currentLocation:EncOffset]
                        dataValue = CurrentDataFile[n].strip()
                        for j in range(CurrentDataSize):
                            new_data += dataValue[j * 2:2 + j * 2].hex()

                        currentLocation = EncOffset + CurrentDataSize
                    if iDeviceType == 3:
                        if locationFromStart == 20:
                            macAddress = CurrentDataFile[n].strip()
                    elif iDeviceType == 4:
                        if locationFromStart == 68:
                            macAddress = CurrentDataFile[n].strip()
                    elif iDeviceType == 5:
                        if locationFromStart == 420:
                            macAddress = CurrentDataFile[n].strip()
                    elif iDeviceType == 6:
                        macAddress = 'FFFFFFFFFFFFFFFF'
                    elif iBootLoaderFlashHeaderType == 2:
                        if locationFromStart == 16:
                            macAddress = CurrentDataFile[n].strip()
                    elif locationFromStart == 48:
                        macAddress = CurrentDataFile[n].strip()

                new_data += data[currentLocation:]
                encryptedData = encryptFlashData(temp_nonce, aPassKey,
                                                 new_data,
                                                 len(new_data))
                sDestFilename = 'output' + macAddress + '.bin'
                outputFile = open(sDestFilename, 'wb')
                if iDeviceType == 3:
                    if iProgrammerType == 0:
                        outputFile.write(image[0:20])
                    else:
                        outputFile.write(image[4:20])
                    outputFile.write(hextranslate(macAddress))
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iDeviceType == 4 or iDeviceType == 5:
                    if iProgrammerType == 0:
                        outputFile.write(image[0:20])
                    else:
                        outputFile.write(image[4:20])
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iDeviceType == 6:
                    if iProgrammerType == 0:
                        outputFile.write(image[0:336])
                    else:
                        outputFile.write(image[0:336])
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iBootLoaderFlashHeaderType == 2:
                    outputFile.write(image[0:16])
                    outputFile.write(hextranslate(macAddress))
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                else:
                    if iDeviceType == 2:
                        outputFile.write(image[0:8])
                    else:
                        outputFile.write(hextranslate('e1e1e1e1'))
                        outputFile.write(image[4:8])
                    LengthInInt = int(image[8:12].hex(), 16)
                    PaddingRequired = 16 - LengthInInt % 16
                    LengthInInt = LengthInInt + PaddingRequired
                    hexstr = '%08x' % LengthInInt
                    outputFile.write(hextranslate(hexstr))
                    outputFile.write(image[12:48])
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)

    except Exception:
        bStatus = False
        print(' ERROR : Could not process ' + sSrcFilename + ',' + sConfigFile)

    print('Done.')
    return bStatus


def encryptBinFile(sSrcFilename, sDestFilename, sPassKey, iDeviceType,
                   iBootLoaderFlashHeaderType, sNonce, ota_header: bool,
                   ota_hdr: bytes,
                   ota_hdr_ext, ota_ext_hdr, header_size, otahrdoffset,
                   encoffset, iProgrammerType):
    bStatus = True
    sKeyString = sPassKey.strip()
    sPassString = (
        '0x' + sKeyString[:8] + ',' +
        '0x' + sKeyString[8:16] + ',' +
        '0x' + sKeyString[16:24] + ',' +
        '0x' + sKeyString[24:32]
    )
    aPassKey = aParsePassKeyString(sPassString)
    sNonceString = sNonce.strip()
    sIvString = (
        '0x' + sNonceString[:8] + ',' +
        '0x' + sNonceString[8:16] + ',' +
        '0x' + sNonceString[16:24] + ',' +
        '0x' + sNonceString[24:28] + '0000'
    )
    aNonce = aParseNonce(sIvString)
    print('Started Encrypting')
    try:
        with open(sSrcFilename, 'rb') as inputFile:
            image = inputFile.read()
        if iDeviceType == 3:
            if sNonceString[28:32] != '0000':
                print('Warning : Lower 2 Bytes Ignored')
            data = image[encoffset:otahrdoffset]
            if ota_header:
                data += ota_hdr
                hdrsize = 108 + header_size
                if ota_ext_hdr:
                    data += ota_hdr_ext
                data += image[hdrsize:]
            else:
                data += image[108:]
        else:
            if iDeviceType == 4 or iDeviceType == 5 or iDeviceType == 6:
                if sNonceString[28:32] != '0000':
                    print('Warning : Lower 2 Bytes Ignored')
                data = image[encoffset:otahrdoffset]
                if ota_header:
                    data += ota_hdr
                    hdrsize = otahrdoffset + header_size
                    if ota_ext_hdr:
                        data += ota_hdr_ext
                    data += image[hdrsize:]
                else:
                    data += image[otahrdoffset:]
            elif iBootLoaderFlashHeaderType == 2:
                data = image[encoffset:otahrdoffset]
                if ota_header:
                    data += ota_hdr
                    hdrsize = otahrdoffset + header_size
                    if ota_ext_hdr:
                        data += ota_hdr_ext
                    data += image[hdrsize:]
                else:
                    data += image[otahrdoffset:]
            else:
                offset = 48
                data = image[offset:]
            try:
                encryptedData = encryptFlashData(aNonce, aPassKey,
                                                 data, len(data))
                outputFile = open(sDestFilename, 'wb')
                if iDeviceType == 3:
                    if iProgrammerType == 0:
                        outputFile.write(image[0:28])
                    else:
                        outputFile.write(image[4:28])
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iDeviceType == 4 or iDeviceType == 5 or iDeviceType == 6:
                    if iDeviceType != 6:
                        if iProgrammerType == 0:
                            outputFile.write(image[0:20])
                        else:
                            outputFile.write(image[4:20])
                    else:
                        outputFile.write(image[0:336])
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                elif iBootLoaderFlashHeaderType == 2:
                    outputFile.write(image[0:24])
                    outputFile.write(hextranslate(sNonce))
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
                else:
                    if iDeviceType == 2:
                        outputFile.write(image[0:8])
                    else:
                        outputFile.write(hextranslate('e1e1e1e1'))
                        outputFile.write(image[4:8])
                    LengthInInt = int(image[8:12].hex(), 16)
                    PaddingRequired = 16 - LengthInInt % 16
                    LengthInInt = LengthInInt + PaddingRequired
                    hexstr = '%08x' % LengthInInt
                    outputFile.write(hextranslate(hexstr))
                    outputFile.write(image[12:48])
                    outputFile.write(encryptedData)
                    outputFile.close()
                    print(' Created  ' + sDestFilename)
            except Exception:
                print(' ERROR: could not open the output file ' + sDestFilename)
                bStatus = False

    except Exception:
        bStatus = False
        print(' ERROR : Could not process ' + sSrcFilename)

    print(' Done ')
    return bStatus


def encryptSerialisatioinFile(sInputFile, sConfigFile, sOutputFile, sPassKey,
                              iBootLoaderFlashHeaderType, sNonce, ota_header,
                              ota_hdr, ota_hdr_ext, ota_ext_hdr, header_size,
                              iDeviceType, iProgrammerType):
    bStatus = True
    sKeyString = sPassKey.strip()
    sPassString = (
        '0x' + sKeyString[:8] + ',' +
        '0x' + sKeyString[8:16] + ',' +
        '0x' + sKeyString[16:24] + ',' +
        '0x' + sKeyString[24:32]
    )
    aPassKey = aParsePassKeyString(sPassString)
    sNonceString = sNonce.strip()
    sIvString = (
        '0x' + sNonceString[:8] + ',' +
        '0x' + sNonceString[8:16] + ',' +
        '0x' + sNonceString[16:24] + ',' +
        '0x' + sNonceString[24:28] + '0000'
    )
    # aNonce = aParseNonce(sIvString)
    aParseNonce(sIvString)
    try:
        with open(sInputFile, 'rb') as inputFile:
            image = inputFile.read()
        if iDeviceType == 3:
            if sNonceString[28:32] != '0000':
                print('Warning : Lower 2 Bytes Ignored')
            offset = 44
            data = image[offset:46]
            data += image[46:108]
            if ota_header:
                data += ota_hdr
                hdrsize = 108 + header_size
                if ota_ext_hdr:
                    data += ota_hdr_ext
                data += image[hdrsize:]
            else:
                data += image[108:]
        else:
            if iBootLoaderFlashHeaderType == 2:
                offset = 40
                code_length = image[42:43].hex()
                LengthInInt = int(code_length, 16)
                LengthInInt = LengthInInt * 4
                PaddingRequired = 16 - LengthInInt % 16
                LengthInInt = LengthInInt + PaddingRequired
                LengthInInt = LengthInInt / 4
                if ota_header:
                    data = image[offset:42]
                    hexstr = '%04x' % LengthInInt
                    data += hextranslate(hexstr)
                    data += image[44:104]
                    data += ota_hdr
                    hdrsize = 104 + header_size
                    if ota_ext_hdr:
                        data += ota_hdr_ext
                    data = image[hdrsize:]
                else:
                    data = image[offset:]
            else:
                offset = 48
                data = image[offset:]
            print(' Start Encrypting ...')
            DataList, TotalLines = getPureDataLines(sConfigFile)
            tempFileData = DataList[0].split(',')
            tempDataFile, tempFileLines = getPureDataLines(
                tempFileData[0].strip())
            previousLines = tempFileLines
            for n in range(1, TotalLines):
                tempFileData = DataList[n].split(',')
                tempDataFile, tempFileLines = getPureDataLines(
                    tempFileData[0].strip())
                if previousLines == tempFileLines:
                    previousLines = tempFileLines
                else:
                    print(' Number of entries mismatch at location %d' % n)
                    return False

            try:
                with open(sOutputFile, 'w') as outputFile:
                    for n in range(0, tempFileLines):
                        sUpdatedLicenceText = '0'
                        currentLocation = 0
                        new_data = ''
                        temp_nonce = aParseNonce(sIvString)
                        for i in range(0, TotalLines):
                            FileData = DataList[i].split(',')
                            CurrentDataFile, CurrentFileLines = \
                                getPureDataLines(FileData[0].strip())
                            CurrentDataSize = int(FileData[2].strip())
                            locationFromStart = int(FileData[1].strip(), 16)
                            if locationFromStart >= offset:
                                EncOffset = locationFromStart - offset
                                if currentLocation == 0:
                                    new_data += data[0:EncOffset]
                                else:
                                    new_data += data[currentLocation:EncOffset]
                                dataValue = CurrentDataFile[n].strip()
                                for j in range(CurrentDataSize):
                                    new_data += dataValue[j * 2:2 + j * 2].hex()

                                currentLocation = EncOffset + CurrentDataSize
                            if iDeviceType == 3:
                                if locationFromStart == 20:
                                    macAddress = CurrentDataFile[n].strip()
                            elif iBootLoaderFlashHeaderType == 2:
                                if locationFromStart == 16:
                                    macAddress = CurrentDataFile[n].strip()
                            elif locationFromStart == 48:
                                macAddress = CurrentDataFile[n].strip()

                        new_data += data[currentLocation:]
                        encryptedData = encryptFlashData(
                            temp_nonce, aPassKey,
                            new_data,
                            len(new_data),
                        )
                        if iBootLoaderFlashHeaderType == 2:
                            macValue = macAddress.hex()
                            encryptedMacAddress = encryptFlashData(
                                [16, 286397204, 353769240, 0],
                                aPassKey,
                                macValue,
                                len(macValue),
                            )
                        if iBootLoaderFlashHeaderType == 2:
                            sUpdatedLicenceText += (
                                ',0030,8,' + encryptedMacAddress[0:8].hex()
                            )
                        for i in range(0, TotalLines):
                            FileData = DataList[i].split(',')
                            CurrentDataFile, CurrentFileLines = getPureDataLines(
                                FileData[0].strip())
                            CurrentDataSize = FileData[2].strip()
                            locationFromStart = FileData[1].strip()
                            if iBootLoaderFlashHeaderType == 2:
                                temp = int(locationFromStart, 16) + 65536
                                tempstr = hex(temp).lstrip('0x')
                                if int(locationFromStart, 16) == 16:
                                    sUpdatedLicenceText += (
                                        ',{0},{1},{2}'.format(tempstr,
                                                              CurrentDataSize,
                                                              macAddress)
                                    )
                                if int(locationFromStart, 16) >= offset:
                                    sUpdatedLicenceText += (
                                        ',{0},{1},'.format(tempstr,
                                                           CurrentDataSize)
                                    )
                                    CurrentDataLocation = \
                                        int(locationFromStart, 16) - offset
                                    sUpdatedLicenceText += \
                                        encryptedData[CurrentDataLocation:CurrentDataLocation + int(CurrentDataSize)].hex()  # noqa
                            else:
                                if int(locationFromStart, 16) == 20:
                                    if iProgrammerType == 1:
                                        int_location = int(locationFromStart, 16) - 4
                                        temp_locationFromStart = hex(int_location).lstrip('0x')
                                    else:
                                        temp_locationFromStart = locationFromStart
                                    sUpdatedLicenceText += (
                                        ',{0},{1},{2}'.format(
                                            temp_locationFromStart,
                                            CurrentDataSize,
                                            macAddress,
                                        )
                                    )
                                if int(locationFromStart, 16) >= offset:
                                    if iProgrammerType == 1:
                                        int_location = int(locationFromStart, 16) - 4
                                        temp_locationFromStart = hex(int_location).lstrip('0x')
                                    else:
                                        temp_locationFromStart = locationFromStart
                                    sUpdatedLicenceText += (
                                        ',' + temp_locationFromStart + ',' +
                                        CurrentDataSize + ','
                                    )
                                    CurrentDataLocation = \
                                        int(locationFromStart, 16) - offset
                                    sUpdatedLicenceText += encryptedData[CurrentDataLocation:CurrentDataLocation + int(CurrentDataSize)].hex()  # noqa

                        sUpdatedLicenceText += '\n'
                        outputFile.write(sUpdatedLicenceText)

                print(' Created  ' + sOutputFile)
            except IOError:
                bStatus = False
                print(' ERROR : Could not process output.txt')
                return bStatus
    except Exception:
        bStatus = False
        print(' ERROR : Could not process ' + sConfigFile)
    return bStatus


def CLIMain():
    sUsage = '%prog [options] \n\nJennic Flash Encryption Tool {0}\n'.format(
        __VERSION__,
    )
    parser = optparse.OptionParser(usage=sUsage, version='%prog ' + __VERSION__)
    parser.add_option(
        '-m',
        '--mode',
        dest='sModeType',
        type='string',
        default='',
        metavar='MODETYPE',
        help="The tool can be used to encrypt binary file, serialisation "
             "data \nor combine the binary and serialisation data into a "
             "single encrypted binary image.\nIt can also be used to generate "
             "OTA files with the header  To define one of these modes of use, "
             "provide :\n'bin' for binary only 'sde' for serialisation data "
             "only 'com' to combine serialisation data\n and binary image and "
             "then output individual binary encrypted image.'otamerge' is "
             "used to merge files together to generate server and client "
             "images for\n OTA download\n'combine' is used to combine a given "
             "bin file with the configuration details file\n to give an "
             "output bin file with name as MAC address",
    )
    parser.add_option(
        '-k',
        '--PassKey',
        dest='sPassKey',
        type='string',
        default='',
        metavar='EFUSEKEY',
        help='Key used for Encryption\n',
    )
    parser.add_option(
        '-g',
        '--encPrivate',
        dest='iPrivateEncrypt',
        type='int',
        default=0,
        metavar='PrivateENCRYPT',
        help='Set option to 1 if private key needs to be encrypted for '
             '6x device',
    )
    parser.add_option(
        '-i',
        '--iVector',
        dest='sNonce',
        type='string',
        default='00000010111213141516171800000000',
        metavar='IVECTOR',
        help='Initial vector used for Encryption\n',
    )
    parser.add_option(
        '-j',
        '--OTA_Header_String',
        dest='OTA_Header_String',
        type='string',
        default='',
        metavar='OTAHEADERSTRING',
        help='The 32 bit OTA Header String\n',
    )
    parser.add_option(
        '-u',
        '--manufacturer',
        dest='manufacturer',
        help='manufacturer code',
        type='int',
    )
    parser.add_option(
        '-t',
        '--image_type',
        dest='image_type',
        help='OTA header image type',
        type='int',
    )
    parser.add_option(
        '-r',
        '--Header_Version',
        dest='Header_Version',
        help='OTA header version',
        type='int',
    )
    parser.add_option(
        '-n',
        '--File_Version',
        dest='File_Version',
        help='OTA File version',
        type='int',
    )
    parser.add_option(
        '-z',
        '--Stack_Version',
        dest='stack_version',
        help='OTA stack version',
        type='int',
    )
    parser.add_option(
        '-d', '--destination',
        dest='dest_mac',
        help='IEEE address of destination node',
        type='int',
        metavar='MAC',
    )
    parser.add_option(
        '--security',
        dest='security_version',
        help='security credential version',
        type='int',
        metavar='VERSION',
    )
    parser.add_option(
        '--hardware',
        dest='hardware_version',
        help='hardware min and max versions',
        nargs=2,
        type='int',
        metavar='MIN MAX',
    )
    parser.add_option(
        '--ota',
        dest='ota_header',
        action='store_true',
        help="in 'otamerge' mode Puts the OTA header at the start of the "
             "Image\nin any of the encryption modes the OTA header is "
             "embedded \ninside the image before encrypting of the image",
    )
    parser.add_option(
        '-b',
        '--blFlashHdType',
        dest='iBootLoaderFlashHeaderType',
        type='int',
        default=1,
        metavar='BLFLASHHDTYPE',
        help="The tool can be used to encrypt binaries with legacy flash "
             "header\nor the new flash header\n"
             "if nothing is provided it's assumed Legacy\n"
             "flash header:\n"
             " '1' for legacy flash header \n"
             " '2' for new flash header ",
    )
    group = optparse.OptionGroup(
        parser,
        'Encryption Options',
        'These options are to be used for Encryption functionality',
    )
    group.add_option(
        '-v',
        '--devType',
        dest='iDeviceType',
        type='string',
        default='JN516X',
        metavar='DEVICETYPE',
        help='The OTA image built for chiptype JN513x, JN514x, JN516x, '
             'JN517x, JN517x\n',
    )
    group.add_option(
        '-f',
        '--InputFile',
        dest='sInputFile',
        type='string',
        default='',
        metavar='INPUTFILE',
        help='Input unencrypted binary file\n',
    )
    group.add_option(
        '-e',
        '--OutputFile',
        dest='sOutputFile',
        type='string',
        default='',
        metavar='OUTPUTFILE',
        help="Output Encrypted File in 'bin' mode. For 'com' and 'sde' mode "
             "this is the input serialisation data file\n",
    )
    group.add_option(
        '-x', '--ConfigFile',
        dest='sConfigFile',
        type='string',
        default='no_option',
        metavar='CONFIGFILE',
        help='The configuration file which contains the details of the '
             'following text files\nMAC address, LinkKey, Zigbee Certificate, '
             'Private Key and the Custom data file',
    )
    group.add_option(
        '-a', '--EnableEncrypt',
        dest='iEnableEncrypt',
        type='int',
        default=0,
        metavar='ENABLEENCRYPT',
        help="This option is complusory so that the appropriate padding can "
             "be added to the binary file \nand then succesful encryption "
             "can be achieved. use '1' to enable",
    )
    group.add_option(
        '-p', '--ProgrammerType',
        dest='iProgrammerType',
        type='int',
        default=0,
        metavar='PROGRAMMERTYPE',
        help="This option strips the version number files at the starting "
             "of the out put binary file \nuse '1' to strip off the 4 byte "
             "version field ",
    )
    parser.add_option_group(group)
    group = optparse.OptionGroup(
        parser,
        'OTA Merge Options',
        'These options are to be used for OTA merge tool',
    )
    group.add_option(
        '-s', '--server',
        dest='server_image',
        help='server image from FILE only for otamerge mode',
        metavar='FILE',
    )
    group.add_option(
        '-c', '--client',
        dest='client_image',
        help='client image from FILE only for otamerge mode',
        metavar='FILE',
    )
    group.add_option(
        '-o', '--output',
        dest='output_image',
        help='output OTA image to FILE only for otamerge mode',
        metavar='FILE',
    )
    group.add_option(
        '--sector_size',
        dest='sector_size',
        help='sector size to align client image to',
        type='int',
    )
    group.add_option(
        '--sign_integrity',
        dest='sign_integrity',
        type='int',
        default=0,
        help='Add Signature or Image Integrity code\n'
             '1 - Signature Curve1\n'
             '2 - Signature Curve2\n'
             '3 - Image Integrity Code\n',
    )
    group.add_option(
        '--embed_hdr',
        dest='embed_hdr',
        action='store_true',
        help='Embeds the ota hdr. This needs to be done for '
             'un-encrypted images',
    )
    group.add_option(
        '--otahdroffset',
        dest='otahdroffset',
        type='int',
        default=0,
        help='The Embeded OTA header start offset and if not specified '
             'it take Legacy value',
    )
    group.add_option(
        '--encoffset',
        dest='encoffset',
        type='int',
        default=0,
        help='The encryption start offset for encrypted image if not '
             'specified it take Legacy value',
    )
    group.add_option(
        '--donotembedcrc',
        dest='image_crc',
        action='store_false',
        help='By default image crc embeded. To disable crc this option '
             'used along with --embed_hdr',
    )
    group.add_option(
        '--copy_mac',
        dest='copy_mac',
        action='store_true',
        help='Copies the image mac address into the bootloader mac address.',
    )
    parser.add_option_group(group)
    parser.set_defaults(client_image='Client.bin')
    parser.set_defaults(output_image='out.bin')
    parser.set_defaults(ota_header=False)
    parser.set_defaults(sign_integrity=0)
    parser.set_defaults(image_crc=True)
    parser.set_defaults(copy_mac=False)
    parser.set_defaults(embed_hdr=False)
    parser.set_defaults(Header_Version=256)
    parser.set_defaults(File_Version=1)
    parser.set_defaults(stack_version=2)
    parser.set_defaults(manufacturer=19022)
    parser.set_defaults(image_type=20808)
    parser.set_defaults(sector_size=65536)
    parser.set_defaults(size=56)
    options, args = parser.parse_args()
    if len(sys.argv) == 1:
        parser.print_help()
        return
    else:
        print('\nJennic Flash Encryption Tool %s\n' % __VERSION__)
        try:
            if options.sModeType == '':
                raise UserWarning('Mode not provided', None)
            if options.sModeType == 'certi':
                if options.sConfigFile == '':
                    raise UserWarning('No Config File provided', None)
                if not CerticomApp(options.sConfigFile):
                    raise RuntimeError(
                        'Failed to launch the Certificate application',
                    )
                sys.exit()
            options.iDeviceType = options.iDeviceType.upper()
            if options.iDeviceType in (
                'JN513X', 'JN514X', 'JN516X', 'JN517X', 'JN5168',
                'JN5169', 'JN5179', 'JN5178', 'JN518X', 'JN5180',
            ):
                if options.iDeviceType == 'JN513X':
                    options.iDeviceType = 1
                elif options.iDeviceType == 'JN514X':
                    options.iDeviceType = 2
                elif options.iDeviceType in ('JN516X',  'JN5168', 'JN5169'):
                    options.iDeviceType = 4
                elif options.iDeviceType in ('JN517X', 'JN5178', 'JN5179'):
                    options.iDeviceType = 5
                elif options.iDeviceType in ('JN518x', 'JN5180'):
                    options.iDeviceType = 6
                else:
                    raise UserWarning('Correct chip type not specified', None)
            else:
                options.iDeviceType = int(options.iDeviceType)
            if not options.otahdroffset:
                if options.iDeviceType == 3:
                    options.otahdroffset = 108
                elif options.iDeviceType == 4:
                    options.otahdroffset = 84
                elif options.iDeviceType == 5:
                    options.otahdroffset = 436
                elif options.iDeviceType == 6:
                    options.otahdroffset = 352
                else:
                    options.otahdroffset = 104
            if not options.encoffset:
                if options.iDeviceType == 3:
                    options.encoffset = 44
                elif options.iDeviceType == 4 or options.iDeviceType == 5:
                    options.encoffset = 36
                elif options.iDeviceType == 6:
                    options.encoffset = 352
                else:
                    options.encoffset = 40
            if options.iBootLoaderFlashHeaderType not in [1, 2]:
                raise UserWarning('Please specifiy right flash type', None)
            if options.sModeType != 'otamerge':
                if options.sModeType == 'combine':
                    if options.sInputFile == '':
                        raise UserWarning(
                            'No Input Configuration File provided', None)
                    if options.sConfigFile == '':
                        raise UserWarning('No Config File provided', None)
                    if options.iPrivateEncrypt == 1:
                        if options.sPassKey == '':
                            raise UserWarning('No Index Key Provided', None)
                        if options.sPassKey.find('0x') == -1:
                            sPassKey = options.sPassKey
                        else:
                            sPassKey = options.sPassKey[2:]
                    else:
                        sPassKey = ''
                else:
                    if options.sInputFile == '':
                        raise UserWarning(
                            'No Input Configuration File provided', None)
                    if options.iDeviceType not in [1, 2, 3, 4, 5, 6]:
                        raise UserWarning('Please specifiy device type', None)
                    if options.sOutputFile == '':
                        if options.sModeType != 'com':
                            raise UserWarning(
                                'No Output Configuration File provided', None)
                    if options.iBootLoaderFlashHeaderType == 2:
                        if options.sNonce == '':
                            raise UserWarning('No Nonce Provided', None)
                    elif options.iDeviceType not in [3, 4, 5, 6]:
                        if options.sNonce != '':
                            print(
                                'Warning : Nonce Value for legacy bootloader '
                                'is fixed so provided value will be ignored',
                            )
                        options.sNonce = '00000010111213141516171800000000'
                    if options.sPassKey.find('0x') == -1:
                        sPassKey = options.sPassKey
                    else:
                        sPassKey = options.sPassKey[2:]
                    if options.sNonce.find('0x') == -1:
                        sNonce = options.sNonce
                    else:
                        sNonce = options.sNonce[2:]
                    print('sNonce = ' + sNonce)
                    print('last 2 bytes = ' + sNonce[28:32])
                    if len(sNonce) != 32:
                        raise UserWarning('Please check the Nonce value', None)
                    input_file = open(options.client_image, 'rb')
                    inputfiledataIV = input_file.read()
                    input_file.close()
                    if options.iDeviceType == 3:
                        SWconfig = inputfiledataIV[42:44].hex()
                        print('SWconfig = ' + SWconfig)
                    else:
                        SWconfig = inputfiledataIV[34:36].hex()
                        print('SWconfig = ' + SWconfig)
                    sNonce = sNonce[0:28] + SWconfig
                    print('updated sNonce = ' + sNonce)
                    if options.sPassKey == '':
                        raise UserWarning('No eFuse Key provided', None)
                    if len(sPassKey) != 32:
                        raise UserWarning('Please check the Pass key ', None)
            else:
                if options.sPassKey != '':
                    if options.sPassKey.find('0x') == -1:
                        sPassKey = options.sPassKey
                    else:
                        sPassKey = options.sPassKey[2:]
                else:
                    sPassKey = ''
                ota_hdr_s = struct.Struct('<I5HIH32sI')
                ota_hdr_ext_s = struct.Struct('<BQHH')
                ota_hdr_ext_security = struct.Struct('<B')
                ota_hdr_ext_mac = struct.Struct('<Q')
                ota_hdr_ext_hardware = struct.Struct('<HH')
                ota_hdr_ext_security_mac = struct.Struct('<BQ')
                ota_hdr_ext_security_hardware = struct.Struct('<BHH')
                ota_hdr_ext_mac_hardware = struct.Struct('<QHH')
                ota_ext_hdr = False
                if options.sModeType == 'otamerge':
                    try:
                        with open(options.client_image, 'rb') as client_file:
                            client_file.seek(0, 2)
                            client_size = client_file.tell()
                        print('client image size : %d' % client_size)
                    except IOError:
                        print('Failed to open input file ota merge')
                        sys.exit()

                else:
                    try:
                        if options.sModeType == 'combine':
                            if not CombinedFile(
                                    options.sInputFile,
                                    options.sConfigFile,
                                    options.iBootLoaderFlashHeaderType,
                                    options.iDeviceType,
                                    options.iEnableEncrypt,
                                    options.iProgrammerType,
                                    sPassKey,
                                    options.iPrivateEncrypt,
                            ):
                                raise UserWarning('Failed to combine files',
                                                  None)
                            print('going to exit')
                            sys.exit()
                        else:
                            try:
                                with open(
                                    options.sInputFile,
                                    'rb',
                                ) as client_file:
                                    client_file.seek(0, 2)
                                    client_size = client_file.tell()
                            except IOError:
                                print('Failed to open input file')
                                sys.exit()

                    except IOError:
                        print('Failed to Combine files')
                        sys.exit()

            # total_image_size = client_size
            header_size = ota_hdr_s.size
            ota_ext_hdr_value = 0
            if options.security_version is not None:
                header_size += 1
                ota_ext_hdr = True
                ota_ext_hdr_value = 1
            if options.hardware_version is not None:
                header_size += 4
                ota_ext_hdr = True
                ota_ext_hdr_value |= 4
            if options.dest_mac is not None:
                header_size += 8
                ota_ext_hdr = True
                ota_ext_hdr_value |= 2
            if options.sign_integrity == 1:
                total_image_size = header_size + client_size + 18 + 50 + 48
            elif options.sign_integrity == 2:
                total_image_size = header_size + client_size + 18 + 80 + 74
            elif options.sign_integrity == 3:
                total_image_size = header_size + client_size + 6 + 22
            else:
                total_image_size = header_size + client_size + 6
            if options.iDeviceType in (3, 4, 5):
                total_image_size = total_image_size - 4
            if options.OTA_Header_String == '':
                with open(options.client_image, 'rb') as client_file:
                    clientfiledataOTAStringHeader = client_file.read()
                OTA_Header_String = \
                    clientfiledataOTAStringHeader[options.otahdroffset + 20:136].decode('latin1')
                if options.manufacturer == 0:
                    manufacturer = clientfiledataOTAStringHeader[94:96]
                    print('Manufacturer Code Fetched From Bin: 0x{0}'.format(
                        manufacturer.hex(),
                    ))
                    manufacturer = int.from_bytes(
                        manufacturer, byteorder='little')
                else:
                    manufacturer = options.manufacturer
                    print('Manufacturer Code: ' + hex(manufacturer))
                if options.image_type == 0:
                    image_type = clientfiledataOTAStringHeader[96:98]
                    print('Image Type Fetched From Bin: ' + image_type.hex())
                    manufacturer = int.from_bytes(image_type, byteorder='little')
                else:
                    image_type = options.image_type
                    print('Image Type: ' + hex(image_type))
                print('OTA Header String Fetched From Bin ' + OTA_Header_String)
                ota_hdr = ota_hdr_s.pack(
                    0xbeef11e,
                    options.Header_Version,
                    header_size,
                    ota_ext_hdr_value if ota_ext_hdr else 0,
                    manufacturer,
                    image_type,
                    options.File_Version,
                    options.stack_version,
                    OTA_Header_String.encode(),
                    total_image_size,
                )
            else:
                ota_hdr = ota_hdr_s.pack(
                    0xbeef11e,
                    options.Header_Version,
                    header_size,
                    ota_ext_hdr_value if ota_ext_hdr else 0,
                    options.manufacturer,
                    options.image_type,
                    options.File_Version,
                    options.stack_version,
                    options.OTA_Header_String.encode(),
                    total_image_size,
                )
            if options.security_version is None:
                options.security_version = 0
            if options.hardware_version is None:
                options.hardware_version = (0, 0)
            if options.dest_mac is None:
                options.dest_mac = 0

            ota_hdr_ext = []
            if ota_ext_hdr_value == 1:
                ota_hdr_ext = ota_hdr_ext_security.pack(
                    options.security_version)
            if ota_ext_hdr_value & 2 == 2:
                ota_hdr_ext = ota_hdr_ext_mac.pack(options.dest_mac)
            if ota_ext_hdr_value & 4 == 4:
                ota_hdr_ext = ota_hdr_ext_hardware.pack(
                    options.hardware_version[0], options.hardware_version[1])
            if ota_ext_hdr_value & 1 == 1 and ota_ext_hdr_value & 2 == 2:
                ota_hdr_ext = ota_hdr_ext_security_mac.pack(
                    options.security_version, options.dest_mac)
            if ota_ext_hdr_value & 1 == 1 and ota_ext_hdr_value & 4 == 4:
                ota_hdr_ext = ota_hdr_ext_security_hardware.pack(
                    options.security_version, options.hardware_version[0],
                    options.hardware_version[1])
            if ota_ext_hdr_value & 2 == 2 and ota_ext_hdr_value & 4 == 4:
                ota_hdr_ext = ota_hdr_ext_mac_hardware.pack(
                    options.dest_mac,
                    options.hardware_version[0],
                    options.hardware_version[1],
                )
            if ota_ext_hdr_value & 1 == 1 and ota_ext_hdr_value & 2 == 2 and \
                    ota_ext_hdr_value & 4 == 4:
                ota_hdr_ext = ota_hdr_ext_s.pack(
                    options.security_version,
                    options.dest_mac,
                    options.hardware_version[0],
                    options.hardware_version[1],
                )
            if options.sModeType == 'bin':
                if not encryptBinFile(
                    options.sInputFile, options.sOutputFile,
                    sPassKey, options.iDeviceType,
                    options.iBootLoaderFlashHeaderType, sNonce,
                    options.ota_header, ota_hdr, ota_hdr_ext,
                    ota_ext_hdr, header_size,
                    options.otahdroffset, options.encoffset,
                    options.iProgrammerType,
                ):
                    raise UserWarning('Failed to encrypt binary file', None)
            elif options.sModeType == 'sde':
                encryptSerialisatioinFile(
                    options.sInputFile,
                    options.sConfigFile,
                    options.sOutputFile,
                    sPassKey,
                    options.iBootLoaderFlashHeaderType,
                    sNonce,
                    options.ota_header,
                    ota_hdr,
                    ota_hdr_ext,
                    ota_ext_hdr,
                    header_size,
                    options.iDeviceType,
                    options.iProgrammerType,
                )
            elif options.sModeType == 'com':
                if not encryptCombinedFile(
                        options.sInputFile,
                        options.sConfigFile,
                        sPassKey,
                        options.iDeviceType,
                        options.iBootLoaderFlashHeaderType,
                        sNonce,
                        options.ota_header,
                        ota_hdr,
                        ota_hdr_ext,
                        ota_ext_hdr,
                        header_size,
                        options.iProgrammerType,
                ):
                    raise UserWarning('Failed to encrypt binary file', None)
            elif options.sModeType == 'otamerge':
                if options.iBootLoaderFlashHeaderType == 2 or \
                        options.iDeviceType in (3, 4, 5, 6):
                    if not otamerge(
                        options.server_image,
                        options.client_image,
                        options.output_image,
                        options.sConfigFile,
                        options.ota_header,
                        options.sector_size,
                        ota_hdr,
                        ota_hdr_ext,
                        ota_ext_hdr,
                        header_size,
                        total_image_size,
                        sPassKey,
                        options.embed_hdr,
                        options.copy_mac,
                        options.manufacturer,
                        options.image_type,
                        options.File_Version,
                        options.iProgrammerType,
                        options.sign_integrity,
                        options.otahdroffset,
                        options.image_crc,
                        options.sNonce,
                        options.OTA_Header_String,
                    ):
                        raise UserWarning('Failed to merge files', None)
                else:
                    raise UserWarning(
                        'ota is supported only for new bootloader use -b 2 '
                        'option',
                        None)
            else:
                raise UserWarning('Invalid File Type option used', None)
        except UserWarning as w:
            print(w[0])

        return


if __name__ == '__main__':
    CLIMain()
