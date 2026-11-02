"""Step 1 (fixed): correct IAT slot resolution for indirect calls around DllMain.
Fixes: FF /2 rip-relative call is 6 bytes; slot VA = insn_addr + insn_size + disp32.
Also: manual TLS directory parse (IMAGE_TLS_DIRECTORY64, Callbacks at field offset 24)."""
import pefile, capstone, struct, json

pe = pefile.PE('/workspace/neverwin.dll')
ib = pe.OPTIONAL_HEADER.ImageBase
raw = pe.__data__

def rva2off(rva):
    for s in pe.sections:
        if s.VirtualAddress <= rva < s.VirtualAddress + max(s.Misc_VirtualSize, s.SizeOfRawData):
            return s.PointerToRawData + (rva - s.VirtualAddress)
    raise ValueError(hex(rva))

iat_map = {}
for entry in pe.DIRECTORY_ENTRY_IMPORT:
    dll = entry.dll.decode()
    for imp in entry.imports:
        iat_map[imp.address] = f"{dll}!{imp.name.decode() if imp.name else 'ord%d'%imp.ordinal}"
print(f"IAT slots mapped: {len(iat_map)}")

tls_rva = pe.OPTIONAL_HEADER.DATA_DIRECTORY[9].VirtualAddress
t = pe.get_data(tls_rva, 40)
startVA, endVA, idxVA, cbVA = struct.unpack('<QQQQ', t[0:32])
print(f"TLS: StartOfData={startVA:#x} EndOfData={endVA:#x} CallbacksArrayVA={cbVA:#x}")
cbs = []
p = cbVA - ib
while True:
    v = struct.unpack('<Q', pe.get_data(p, 8))[0]
    if v == 0: break
    cbs.append(v - ib); p += 8
print("TLS callback RVAs:", [hex(c) for c in cbs] or "NONE (callbacks array empty)")

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)

def scan(start_rva, end_rva, label):
    code = raw[rva2off(start_rva):rva2off(end_rva)]
    found = []
    for ins in md.disasm(code, ib + start_rva):
        b = ins.bytes
        # FF /2 rip-relative indirect call: opcode FF, ModRM r/m=010 with mod=00 (disp32,SIB=none,reg=2)
        if len(b) >= 6 and b[-6] == 0xFF and (b[-5] & 0xC7) == 0x15:
            disp = int.from_bytes(b[-4:], 'little', signed=True)
            slot = ins.address + ins.size + disp
            kind = 'call'
            found.append({'insn_rva': hex(ins.address-ib), 'iat_slot_va': hex(slot),
                          'api': iat_map.get(slot, 'UNKNOWN'), 'kind': kind})
    print(f"\n== {label} [{start_rva:#x}..{end_rva:#x}] ==")
    for f in found:
        print(f"  @RVA {f['insn_rva']} -> slot {f['iat_slot_va']} : {f['api']}")
    return found

res = {}
res['attach_handler'] = scan(0x4EDC50, 0x4EE100, 'PROCESS_ATTACH handler')
res['dllmain_body']   = scan(0x4ED740, 0x4EDC50, 'DllMain body')
json.dump({'image_base': hex(ib), 'tls_callbacks_rva': [hex(c) for c in cbs], **res},
          open('/workspace/analysis/step1_iat.json','w'), indent=1)
print("\nSaved analysis/step1_iat.json")
