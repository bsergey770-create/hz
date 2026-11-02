#!/usr/bin/env python3
"""Map every IAT slot actually called from .text -> API name + calling functions."""
import re, json, struct, bisect
import pefile

data = open('/workspace/neverwin.dll', 'rb').read()
pdata_off, pdata_size = 0xc9d400, 0x3189c
funcs = []
for i in range(pdata_size // 12):
    b, e = struct.unpack_from('<II', data, pdata_off + i * 12)
    funcs.append((b, e))
funcs.sort()
starts = [f[0] for f in funcs]

def func_of(rva):
    j = bisect.bisect_right(starts, rva) - 1
    if 0 <= j and funcs[j][0] <= rva < funcs[j][1]:
        return funcs[j]
    return None

txt_off, txt_va, txt_len = 0x400, 0x1000, 0x5694fc
text = data[txt_off:txt_off + txt_len]
sites = {}
for m in re.finditer(rb'\xff\x15', text):
    s = m.start()
    disp = struct.unpack('<i', text[s + 2:s + 6])[0]
    tgt = txt_va + s + 6 + disp          # FF /2 rip-rel call: size 6
    if 0x56b000 <= tgt < 0x56c500:       # IAT lives at RVA 0x56b000+ (first .rdata page area)
        sites.setdefault(tgt, []).append(txt_va + s)

pe = pefile.PE('/workspace/neverwin.dll')   # full parse -> DIRECTORY_ENTRY_IMPORT built
slot_api = {}
for ent in pe.DIRECTORY_ENTRY_IMPORT:
    dll = ent.dll.decode()
    for imp in ent.imports:
        if imp.address:
            slot_api[imp.address - 0x180000000] = f"{dll}!{imp.name.decode() if imp.name else 'ord%d' % imp.ordinal}"

out = {}
for slot, ss in sorted(sites.items()):
    fs = sorted({hex(func_of(x)[0]) for x in ss if func_of(x)})
    out[hex(slot)] = {'api': slot_api.get(slot, '?'), 'call_sites': [hex(x) for x in ss], 'funcs': fs}
json.dump(out, open('/workspace/analysis/iat_usage.json', 'w'), indent=1)
interesting = ['VirtualProtect', 'VirtualAlloc', 'OpenThread', 'GetThreadContext', 'SetThreadContext',
               'SuspendThread', 'ResumeThread', 'WinHttpOpen', 'WinHttpConnect', 'WinHttpOpenRequest',
               'WinHttpSendRequest', 'LoadLibraryA', 'LoadLibraryW', 'GetProcAddress']
for slot, rec in out.items():
    if any(k in rec['api'] for k in interesting):
        print(rec['api'], slot, 'sites:', len(rec['call_sites']), 'funcs:', rec['funcs'][:12])
