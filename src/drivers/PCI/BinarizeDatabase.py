import io
import struct
import sys

in_path, out_path = sys.argv[1:]

WHITESPACE = '\r\n\t '

def trim(s):
    return s.lstrip(WHITESPACE).rstrip(WHITESPACE)

vendor_id_to_name = {}
vendor_id_to_devices = {}
current_vendor_id = None

reached_device_classes = False
device_classes = {}
current_class = None
current_subclass = None

for line in open(in_path, 'r').readlines():
    trimmed = trim(line)

    if trimmed == '# List of known device classes, subclasses and programming interfaces':
        reached_device_classes = True
    
    if len(trimmed) == 0 or trimmed[0] == '#':
        continue
    
    try:
        if line.startswith('\t\t'):
            if reached_device_classes:
                # prog-if  prof-if_name
                progif_id, progif_name = trimmed.split('  ', 1)

                current_subclass[progif_id] = progif_name
            else:
                # subvendor subdevice  subsystem_name
                pass
        elif line.startswith('\t'):
            if reached_device_classes:
                # subclass  subclass_name
                subclass_id, subclass_name = trimmed.split('  ', 1)

                current_subclass = {}
                current_class[subclass_id] = (subclass_name, current_subclass)
            else:
                # device  device_name
                device, name = trimmed.split('  ', 1)
                
                vendor_id_to_devices[current_vendor_id][device] = name
        else:
            if reached_device_classes:
                # C class  class_name
                assert trimmed[0] == 'C'

                class_id, class_name = trimmed[2:].split('  ', 1)
                print(class_id, class_name)

                current_class = {}
                device_classes[class_id] = (class_name, current_class)

                print(device_classes)
            else:
                # vendor  vendor_name
                id, name = trimmed.split('  ', 1)

                vendor_id_to_name[id] = name
                vendor_id_to_devices[id] = {}
                current_vendor_id = id
    except:
        print(f'Malformed line: "{line}"')
        print(f'{reached_device_classes}')
        sys.exit()

import pprint

pprint.pprint(vendor_id_to_devices)
pprint.pprint(device_classes)

vendors = io.BytesIO()
devices = io.BytesIO()

all_classes = io.BytesIO()
all_subclasses = io.BytesIO()
all_progifs = io.BytesIO()

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

for id, info in device_classes.items():
    name, subclasses = info

    all_classes.write(struct.pack('i', int(id, base=16)))
    all_classes.write(struct.pack('i', add_str(name)))
    all_classes.write(struct.pack('i', len(subclasses)))
    all_classes.write(struct.pack('i', int(all_subclasses.tell() / 16)))

    for subclass_id, info in subclasses.items():
        subclass_name, progifs = info

        all_subclasses.write(struct.pack('i', int(subclass_id, base=16)))
        all_subclasses.write(struct.pack('i', add_str(subclass_name)))
        all_subclasses.write(struct.pack('i', len(progifs)))
        all_subclasses.write(struct.pack('i', int(all_progifs.tell() / 8)))

        for progif_id, progif_name in progifs.items():
            all_progifs.write(struct.pack('i', int(progif_id, base=16)))
            all_progifs.write(struct.pack('i', add_str(progif_name)))

header_fields = 2 + 2 + 3 + 1
sz = header_fields * 4

meta = io.BytesIO()

meta.write(struct.pack('i', len(vendor_id_to_name)))
meta.write(struct.pack('i', len(device_classes)))

meta.write(struct.pack('i', sz)) # vendors offset
meta.write(struct.pack('i', sz + vendors.tell())) # devices offset

meta.write(struct.pack('i', sz + vendors.tell() + devices.tell())) # classes offset
meta.write(struct.pack('i', sz + vendors.tell() + devices.tell() + all_classes.tell())) # subclasses offset
meta.write(struct.pack('i', sz + vendors.tell() + devices.tell() + all_classes.tell() + all_subclasses.tell())) # progifs offset

meta.write(struct.pack('i', sz + vendors.tell() + devices.tell() + all_classes.tell() + all_subclasses.tell() + all_progifs.tell())) # strings offset


f = open(out_path, 'wb')
f.write(meta.getbuffer())
f.write(vendors.getbuffer())
f.write(devices.getbuffer())
f.write(all_classes.getbuffer())
f.write(all_subclasses.getbuffer())
f.write(all_progifs.getbuffer())
f.write(strings.getbuffer())
f.close()

#print(vendor_id_to_name['8086'], vendor_id_to_devices['8086']['8c26'])
