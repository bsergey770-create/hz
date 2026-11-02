import pefile, struct, json
pe = pefile.PE('/workspace/neverwin.dll')
ib = pe.OPTIONAL_HEADER.ImageBase
t = pe.get_data(pe.OPTIONAL_HEADER.DATA_DIRECTORY[9].VirtualAddress, 40)
start, end, idxva, cbva = struct.unpack('<QQQQ', t[0:32])
print(f"Start={start:#x} End={end:#x} IndexVA={idxva:#x} CallbacksArrayVA={cbva:#x}")
rva = cbva - ib
cbs = []
while True:
    v = struct.unpack('<Q', pe.get_data(rva + 8*len(cbs), 8))[0]
    if v == 0:
        break
    cbs.append(v - ib)
print("TLS callback RVAs:", [hex(c) for c in cbs] or "NONE (array empty; .fptable is zero-filled)")
json.dump({'tls': {'start_of_rawdata': hex(start), 'end_of_rawdata': hex(end),
                   'address_of_index': hex(idxva), 'callbacks_array_va': hex(cbva),
                   'callbacks_rva': [hex(c) for c in cbs]}},
          open('/workspace/analysis/tls_fixed.json', 'w'), indent=1)
