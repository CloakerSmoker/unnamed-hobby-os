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

import codecs

for line in codecs.open(in_path, 'r', encoding='utf-8', errors='ignore').readlines():
    trimmed = trim(line)

    if trimmed == '# List of known device classes, subclasses and protocols':
        break
    
    if len(trimmed) == 0 or trimmed[0] == '#':
        continue
    
    try:
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
    except:
        print(f'Malformed line: "{line}"')
        sys.exit()

import pprint

pprint.pprint(vendor_id_to_devices)

vendors = io.BytesIO()
devices = io.BytesIO()

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

header_fields = 1 + 1 + 1 + 1
sz = header_fields * 4

meta = io.BytesIO()

meta.write(struct.pack('i', len(vendor_id_to_name)))

meta.write(struct.pack('i', sz)) # vendors offset
meta.write(struct.pack('i', sz + vendors.tell())) # devices offset
meta.write(struct.pack('i', sz + vendors.tell() + devices.tell())) # strings offset

f = open(out_path, 'wb')
f.write(meta.getbuffer())
f.write(vendors.getbuffer())
f.write(devices.getbuffer())
f.write(strings.getbuffer())
f.close()

#print(vendor_id_to_name['8086'], vendor_id_to_devices['8086']['8c26'])
