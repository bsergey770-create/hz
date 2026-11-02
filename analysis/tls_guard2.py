import pefile, struct
pe = pefile.PE('/workspace/neverwin.dll')
base = pe.OPTIONAL_HEADER.ImageBase

print("=== TLS ===")
tls_rva = pe.OPTIONAL_HEADER.DATA_DIRECTORY[9].VirtualAddress
b = pe.get_data(tls_rva, 0x18)
start, end = struct.unpack('<QQ', b[0:16])
cb_va = struct.unpack('<Q', b[16:24])[0]
print(f"StartVA=0x{start:x} EndVA=0x{end:x} CallbacksVA=0x{cb_va:x}")
n = 0
while True:
    cb = struct.unpack('<Q', pe.get_data(cb_va - base + n*8, 8))[0]
    if cb == 0: break
    print(f"TLS callback[{n}] RVA=0x{cb - base:x}")
    n += 1

lc_rva = pe.OPTIONAL_HEADER.DATA_DIRECTORY[10].VirtualAddress
data = pe.get_data(lc_rva, 0x140)
def u32(o): return struct.unpack('<I', data[o:o+4])[0]
def u64(o): return struct.unpack('<Q', data[o:o+8])[0]
print("\n=== LOAD CONFIG ===")
print(f"GuardCFCheck=0x{u64(0x58):x} GuardCFDispatch=0x{u64(0x60):x}")
print(f"CFFunctionTable=0x{u64(0x68):x} Count={u64(0x70)}")
gf = u32(0x78); print(f"GuardFlags=0x{gf:x} CFG={bool(gf&0x100)} Fptr={bool(gf&0x400)} HotPatch={bool(gf&1)}")
print(f"SecurityCookie VA=0x{u64(0x98):x}")
print(f"SEHTable RVA=0x{u32(0xa0):x} count={u32(0xa4)}")
print(f"GuardIAT RVA=0x{u32(0xa8):x} size=0x{u32(0xac):x}")
