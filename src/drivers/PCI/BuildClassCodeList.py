
import io
import struct


meta = io.BytesIO()
strings = io.BytesIO()

input = open('ClassCodes.txt', 'r').read()

classes = {}
current_class = {}
current_subclass = {}

def add_str(string):
    global strings
    
    index = strings.tell()
    strings.write(string.encode('UTF-8'))
    strings.write(bytearray([0x00]))
    return index

for line in input.splitlines():
    if line[0] == '\t':
        if line[1] == '\t':
            # progif entry
            
            progif_id, progif_name = line[2:].split('  ')
            
            current_progif = {'name': add_str(progif_name)}
            
            current_subclass[int(progif_id, 16)] = current_progif
        else:
            # subclass entry
            
            subclass_id, subclass_name = line[1:].split('  ')
            
            current_subclass = {}
            
            current_class[int(subclass_id, 16)] = {'name': add_str(subclass_name), 'progifs': current_subclass}
    else:
        # class entry
        class_id, class_name = line.split('  ')
        
        current_class = {}
        
        classes[int(class_id, 16)] = {'name': add_str(class_name), 'subclasses': current_class}

class_count = len(classes)

meta.write(struct.pack('i', 0))
meta.write(struct.pack('i', class_count))

for id, info in classes.items():    
    info['offset'] = meta.tell()
    
    meta.write(struct.pack('i', 0))
    
for id, info in classes.items():
    write_offset_to = info['offset']
    name_offset = info['name']
    subclasses = info['subclasses']
    subclass_count = len(subclasses)
    
    offset = meta.tell()
    meta.seek(write_offset_to)
    meta.write(struct.pack('i', offset))
    meta.seek(offset)
    
    meta.write(struct.pack('i', name_offset))
    meta.write(struct.pack('h', id))
    meta.write(struct.pack('h', subclass_count))
    
    for subclass_id, subclass_info in subclasses.items():
        subclass_info['offset'] = meta.tell()
        
        meta.write(struct.pack('i', 0))
    
    for subclass_id, subclass_info in subclasses.items():
        write_offset_to = subclass_info['offset']
        name_offset = subclass_info['name']
        progifs = subclass_info['progifs']
        progif_count = len(progifs)
        
        offset = meta.tell()
        meta.seek(write_offset_to)
        meta.write(struct.pack('i', offset))
        meta.seek(offset)
        
        meta.write(struct.pack('i', name_offset))
        meta.write(struct.pack('h', subclass_id))
        meta.write(struct.pack('h', progif_count))
        
        for progif_id, progif_info in progifs.items():
            progif_info['offset'] = meta.tell()
            
            meta.write(struct.pack('i', 0))
        
        for progif_id, progif_info in progifs.items():
            write_offset_to = progif_info['offset']
            name_offset = progif_info['name']
            
            offset = meta.tell()
            meta.seek(write_offset_to)
            meta.write(struct.pack('i', offset))
            meta.seek(offset)
            
            meta.write(struct.pack('i', name_offset))
            meta.write(struct.pack('h', progif_id))

offset = meta.tell()
meta.seek(0)
meta.write(struct.pack('i', offset))
meta.seek(offset)

f = open('out.bin', 'wb')
f.write(meta.getbuffer())
f.write(strings.getbuffer())