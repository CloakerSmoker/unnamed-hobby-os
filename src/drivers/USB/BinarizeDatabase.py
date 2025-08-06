import io
import struct
import sys


#in_path, out_path = sys.argv[1:]
in_path = 'build/usb.ids'
out_path = 'build/usbids.bin'

WHITESPACE = '\r\n\t '

def trim(s):
    return s.lstrip(WHITESPACE).rstrip(WHITESPACE)

vendor_id_to_name = {}
vendor_id_to_devices = {}
current_vendor_id = None

class_id_to_subclasses = {}
current_class_id = None
current_subclass_id = None

usage_page_id_to_usages = {}
current_usage_page_id = None

import codecs

mode = 'vendor_device'

for line in codecs.open(in_path, 'r', encoding='utf-8', errors='ignore').readlines():
    trimmed = trim(line)

    if trimmed == '# List of known device classes, subclasses and protocols':
        mode = 'class_subclass_protocol'
    elif trimmed == '# List of Audio Class Terminal Types':
        mode = 'skip'
    elif trimmed == '# List of HID Usages':
        mode = 'usage_page_usage'
    elif trimmed == '# List of Languages':
        break
    
    if len(trimmed) == 0 or trimmed[0] == '#':
        continue
    
    try:
        if mode == 'vendor_device':
            if line.startswith('\t'):
                # device  device_name
                device, device_name = trimmed.split('  ', 1)

                vendor_id_to_devices[current_vendor_id][device] = device_name
            else:
                # vendor  vendor_name
                vendor, vendor_name = trimmed.split('  ', 1)

                current_vendor_id = vendor
                vendor_id_to_name[vendor] = vendor_name
                vendor_id_to_devices[vendor] = {}
        elif mode == 'class_subclass_protocol':
            tabs = len(line) - len(line.lstrip('\t'))

            if line.startswith('C'):
                # C class_id  class_name
                class_id, class_name = trimmed[2:].split('  ', 1)

                class_id_to_subclasses[class_id] = {
                    'name': class_name,
                    'subclasses': {}
                }

                current_class_id = class_id
            elif tabs == 1:
                #    subclass_id  subclass_name
                subclass_id, subclass_name = trimmed.split('  ', 1)

                class_id_to_subclasses[current_class_id]['subclasses'][subclass_id] = {
                    'name': subclass_name,
                    'protocols': {}
                }

                current_subclass_id = subclass_id
            elif tabs == 2:
                #        protocol_id  protocol_name
                protocol_id, protocol_name = trimmed.split('  ', 1)

                class_id_to_subclasses[current_class_id]['subclasses'][current_subclass_id]['protocols'][protocol_id] = protocol_name
        elif mode == 'usage_page_usage':
            if trimmed.startswith('HUT'):
                # HUT usage_page_id  usage_page_name
                usage_page_id, usage_page_name = trimmed[4:].split('  ', 1)

                usage_page_id_to_usages[usage_page_id] = {
                    'name': usage_page_name,
                    'usages': {}
                }

                current_usage_page_id = usage_page_id
            else:
                #    usage_id  usage_name
                usage_id, usage_name = trimmed.split('  ', 1)

                usage_page_id_to_usages[current_usage_page_id]['usages'][usage_id] = usage_name

    except Exception as e:
        print(f'Malformed line: "{line}", {e}')
        sys.exit()

import pprint

pprint.pprint(usage_page_id_to_usages)

#sys.exit()

vendors = io.BytesIO()
devices = io.BytesIO()

classes = io.BytesIO()
subclasses = io.BytesIO()
protocols = io.BytesIO()

usage_pages = io.BytesIO()
usages = io.BytesIO()

strings = io.BytesIO()

def add_str(string):
    global strings
    
    index = strings.tell()
    strings.write(string.encode('UTF-8'))
    strings.write(bytearray([0x00]))
    return index

for id, name in vendor_id_to_name.items():
    vendors.write(struct.pack('i', int(id, base=16))) # vendor.id
    vendors.write(struct.pack('i', add_str(name))) # vendor.name_offset
    vendors.write(struct.pack('i', len(vendor_id_to_devices[id]))) # vendor.device_count
    vendors.write(struct.pack('i', int(devices.tell() / 8))) # vendor.first_device_index

    for device_id, device_name in vendor_id_to_devices[id].items():
        devices.write(struct.pack('i', int(device_id, base=16))) # vendor.devices[N].id
        devices.write(struct.pack('i', add_str(device_name))) # vendor.devices[N].name_offset

for id, info in class_id_to_subclasses.items():
    classes.write(struct.pack('i', int(id, base=16))) # class.id
    classes.write(struct.pack('i', add_str(info['name']))) # class.name_offset
    classes.write(struct.pack('i', len(info['subclasses']))) # class.subclass_count
    classes.write(struct.pack('i', int(subclasses.tell() / 8))) # class.first_subclass_index

    for subclass_id, subclass_info in info['subclasses'].items():
        subclasses.write(struct.pack('i', int(subclass_id, base=16))) # class.subclasses[N].id
        subclasses.write(struct.pack('i', add_str(subclass_info['name']))) # class.subclasses[N].name_offset
        subclasses.write(struct.pack('i', len(subclass_info['protocols']))) # class.subclasses[N].protocol_count
        subclasses.write(struct.pack('i', int(protocols.tell() / 8))) # class.subclasses[N].first_protocol_index

        for protocol_id, protocol_name in subclass_info['protocols'].items():
            protocols.write(struct.pack('i', int(protocol_id, base=16))) # class.subclasses[N].protocols[M].id
            protocols.write(struct.pack('i', add_str(protocol_name))) # class.subclasses[N].protocols[M].name_offset

for id, info in usage_page_id_to_usages.items():
    usage_pages.write(struct.pack('i', int(id, base=16))) # usage_page.id
    usage_pages.write(struct.pack('i', add_str(info['name']))) # usage_page.name_offset
    usage_pages.write(struct.pack('i', len(info['usages']))) # usage_page.usage_count
    usage_pages.write(struct.pack('i', int(usages.tell() / 8))) # usage_page.first_usage_index

    for usage_id, usage_name in info['usages'].items():
        usages.write(struct.pack('i', int(usage_id, base=16))) # usage_page.usages[N].id
        usages.write(struct.pack('i', add_str(usage_name))) # usage_page.usages[N].name_offset

header_fields = 11
header_size = header_fields * 4

meta = io.BytesIO()


meta.write(struct.pack('i', len(vendor_id_to_name)))
meta.write(struct.pack('i', len(class_id_to_subclasses)))
meta.write(struct.pack('i', len(usage_page_id_to_usages)))

offset = header_size

meta.write(struct.pack('i', offset)) # vendors offset
offset += vendors.tell()

meta.write(struct.pack('i', offset)) # devices offset
offset += devices.tell()

meta.write(struct.pack('i', offset)) # classes offset
offset += classes.tell()

meta.write(struct.pack('i', offset)) # subclasses offset
offset += subclasses.tell()

meta.write(struct.pack('i', offset)) # protocols offset
offset += protocols.tell()

meta.write(struct.pack('i', offset)) # usage_pages offset
offset += usage_pages.tell()

meta.write(struct.pack('i', offset)) # usages offset
offset += usages.tell()

meta.write(struct.pack('i', offset)) # strings offset
offset += strings.tell()

f = open(out_path, 'wb')
f.write(meta.getbuffer())

f.write(vendors.getbuffer())
f.write(devices.getbuffer())

f.write(classes.getbuffer())
f.write(subclasses.getbuffer())
f.write(protocols.getbuffer())

f.write(usage_pages.getbuffer())
f.write(usages.getbuffer())

f.write(strings.getbuffer())

f.close()

#print(vendor_id_to_name['8086'], vendor_id_to_devices['8086']['8c26'])
