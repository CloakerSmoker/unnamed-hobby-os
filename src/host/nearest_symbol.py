class HelloWorld (gdb.Command):
    """Greet the whole world."""

    def __init__ (self):
        super (HelloWorld, self).__init__ ("find-closest", gdb.COMMAND_DATA)

    def complete(self, text, word):
        return gdb.COMPLETE_EXPRESSION

    def invoke (self, arg, from_tty):
        arg = gdb.parse_and_eval(arg)
        goal = int(arg)

        binary = None

        for file in gdb.objfiles():
            if file.filename[-3:] == 'elf':
                binary = file.filename

        gdb.execute('shell dwarfdump -di ' + binary + ' > dwarfdump.tmp', from_tty=True)

        symbols = {}

        for line in open('dwarfdump.tmp', 'r'):
            if line[0] != '<':
                continue

            line = line.strip(' \t\r\n')

            fields = line.split(' ', 1)
            depth, offset, tag = fields.pop(0)[1:].replace('>', '').split('<')
            fields = fields[0]

            #print(depth, offset, tag, fields)

            levels_open = 0

            name = ''
            body = ''

            fields = list(fields)
            attrs = {}

            while len(fields):
                char = fields.pop(0)

                if char == '<':
                    if levels_open != 0:
                        body += '<'

                    levels_open += 1
                elif char == '>':
                    if levels_open != 1:
                        body += '>'

                    levels_open -= 1

                    if levels_open == 0:
                        attrs[name.strip(' \t')] = body.strip(' \t')
                        name = ''
                        body = ''
                elif levels_open == 0:
                    name += char
                else:
                    body += char
                
            if 'DW_AT_name' in attrs:
                symbols[attrs['DW_AT_name']] = {'depth': int(depth), 'tag': tag, **attrs}

        best = (99999999999, 'NONE')

        for name, symbol in symbols.items():
            if symbol['depth'] != 1:
                continue

            if symbol['tag'] != 'DW_TAG_variable' and symbol['tag'] != 'DW_TAG_subprogram':
                continue

            low_pc, high_pc = 0, 0

            if symbol['tag'] == 'DW_TAG_subprogram':
                low_pc = int(symbol['DW_AT_low_pc'], base=16)
                high_pc = int(symbol['DW_AT_high_pc'].split('highpc: ')[1].rstrip('>'), base=16)

                #print(symbol, low_pc, high_pc)
            elif symbol['tag'] == 'DW_TAG_variable' and not 'DW_AT_const_value' in symbol:
                #print(symbol)
                addr = int(symbol['DW_AT_location'].split('DW_OP_addr ')[1], base=16)
                size = int(symbol['DW_AT_byte_size'], base=16)

                low_pc = addr
                high_pc = addr + size
            
            #print(hex(low_pc), hex(high_pc), name)

            dist = goal - low_pc

            if dist >= 0 and dist < best[0]:
                best = (dist, name, symbol, low_pc, high_pc)

        dist, name, symbol, low_pc, high_pc = best

        print(f'{name} ({hex(low_pc)}) + {hex(dist)} = {hex(goal)}')

HelloWorld ()