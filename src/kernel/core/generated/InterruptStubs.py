out = open('src/kernel/core/generated/InterruptStubs.rlx', 'w')

error_code_stub = '''
define void OnInterrupt{}() asm {{
	call, @InterruptCodeSetup
	
	mov, rsi, {}
	call, @GenericInterrupt
	
	jmp, @InterruptCodeReturn
}}
'''

no_error_code_stub = '''
define void OnInterrupt{}() asm {{
	call, @InterruptSetup
	
	mov, rsi, {}
	call, @GenericInterrupt
	
	jmp, @InterruptReturn
}}
'''

single_stub_add_template = '\tInsertIDTEntry({}, &OnInterrupt{}, true)\n'
all_stubs_add_template = '''
define void AddHandlerStubs() {{
{}
}}
'''

has_error_code = [
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	True,
	False,
	True,
	True,
	True,
	True,
	True,
	False,
	False,
	True,
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	False,
	True,
	False,
]

adds = ''

index = 0

NUMBER_OF_STUBS = 256

for i in range(NUMBER_OF_STUBS):
	single_stub_template = no_error_code_stub

	if i in has_error_code and has_error_code[1]:
		single_stub_template = error_code_stub
	
	out.write(single_stub_template.format(index, index))
	adds += single_stub_add_template.format(index, index)
	
	index += 1

out.write(all_stubs_add_template.format(adds))

out.close()
