"""Step 2: function detection via .pdata, call-graph BFS from DllMain (RVA 0x4ED740),
and zone classification by xrefs to strings / imports."""
import pefile, capstone, struct, json, collections

PE = '/workspace/neverwin.dll'
pe = pefile.PE(PE)
ib = pe.OPTIONAL_HEADER.ImageBase
raw = pe.__data__

def rva2off(rva):
    for s in pe.sections:
        if s.VirtualAddress <= rva < s.VirtualAddress + max(s.Misc_VirtualSize, s.SizeOfRawData):
            return s.PointerToRawData + (rva - s.VirtualAddress)
    return None

# ---------- 1. Function table from .pdata (RUNTIME_FUNCTION) ----------
pd = None
for s in pe.sections:
    if s.Name.startswith(b'.pdata'): pd = s
funcs = []          # list of start RVAs
frange = {}         # start_rva -> end_rva
rf_count = pd.SizeOfRawData // 12
for i in range(rf_count):
    begin, end, unwind = struct.unpack('<III', raw[pd.PointerToRawData+12*i : pd.PointerToRawData+12*(i+1)])
    if ib+begin >= ib and begin < end <= pe.OPTIONAL_HEADER.SizeOfImage:
        funcs.append(begin); frange[begin] = end
funcs = sorted(set(funcs))
fstarts = set(funcs)
print(f"Functions from .pdata: {len(funcs)}")

# IAT map slot VA -> name
iat_map = {}
for entry in pe.DIRECTORY_ENTRY_IMPORT:
    dll = entry.dll.decode()
    for imp in entry.imports:
        iat_map[imp.address] = f"{dll}!{imp.name.decode() if imp.name else 'ord%d'%imp.ordinal}"

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True

# ---------- 2. Build call graph edges (byte-level, no capstone detail needed) ----------
def parse_func(f):
    """Return (call_edges, rip_data_targets) using manual ModRM decoding."""
    off0, off1 = rva2off(f), rva2off(frange[f])
    code = raw[off0:off1]
    base_va = ib + f
    edges, data_refs = set(), []
    i = 0
    n = len(code)
    while i < n:
        b0 = code[i]
        # E8 rel32 near call
        if b0 == 0xE8 and i + 5 <= n:
            disp = int.from_bytes(code[i+1:i+5], 'little', signed=True)
            tgt = base_va + i + 5 + disp - ib
            if tgt in fstarts: edges.add(tgt)
            i += 5; continue
        # FF /2 rip-relative indirect call: 40-pref optional, FF 15 disp32
        if b0 in (0xFF,) and i + 1 < n and (code[i+1] & 0xC7) == 0x15:
            disp = int.from_bytes(code[i+2:i+6], 'little', signed=True)
            slot_va = base_va + i + 6 + disp
            api = iat_map.get(slot_va)
            if api: edges.add(('IAT', api))
            data_refs.append(slot_va)
            i += 6; continue
        # LEA/MOV rip-relative data ref: REX?(8D|8B) modrm mod=00 rm=101 disp32 (+opcode for 8D needs reg field)
        if b0 in (0x8D, 0x8B) and i + 5 < n:
            modrm = code[i+1]
            if (modrm >> 6) == 0 and ((modrm >> 3) & 7) != 4 and (modrm & 7) == 5:
                disp = int.from_bytes(code[i+2:i+6], 'little', signed=True)
                tgt = base_va + i + 6 + disp
                data_refs.append(tgt)
                i += 6; continue
        i += 1
    return edges, data_refs

calls_from, riprefs = {}, {}
for f in funcs:
    e, d = parse_func(f)
    calls_from[f] = e; riprefs[f] = d
print("Call graph built.")

# ---------- 3. BFS from DllMain roots ----------
START = 0x4ED740
ROOTS = [0x4ED740, 0x4EDC50, 0x4ED880]
seen, order = set(), []
q = collections.deque(ROOTS)
for r in ROOTS: seen.add(r)
while q:
    f = q.popleft(); order.append(f)
    for e in calls_from.get(f, ()):
        if not isinstance(e, tuple) and e not in seen:
            seen.add(e); q.append(e)
dq = collections.deque([(r,0) for r in ROOTS]); dd={r:0 for r in ROOTS}
while dq:
    f,d = dq.popleft()
    for e in calls_from.get(f, ()):
        if not isinstance(e, tuple) and e not in dd:
            dd[e]=d+1; dq.append((e,d+1))
print(f"BFS reachable functions from DllMain roots: {len(seen)}, max depth {max(dd.values())}")

# ---------- 4. Zone classification via string/IAT xrefs ----------
rdata = [s for s in pe.sections if s.Name.startswith(b'.rdata')]
text  = [s for s in pe.sections if s.Name.startswith(b'.text')][0]

ZONE_KEYS = {
 'Hook-Engine':      ['VirtualProtect','CreateThread','OpenThread','GetThreadContext','SetThreadContext',
                      'SuspendThread','ResumeThread','Toolhelp32','CreateFileMapping','MapViewOfFile',
                      'VirtualQuery','FlushInstructionCache','GetProcAddress','LoadLibrary'],
 'Lua-Runtime':      ['luaL_','lua_','luajit','lj_','lua_error','engine_simulate'],
 'Renderer':         ['D3D11','DXGI','D3DCOMPILER','IDXGISwapChain','Present','CreateSwapChain','imgui','ImGui'],
 'Network/License':  ['WinHttp','winhttp','neverwin.one','https://'],
 'Config/UI':        ['GetPrivateProfile','WritePrivateProfile','CreateDirectoryW','FindFirstFile',
                      'fopen','_wfopen','.cfg','configs','langs','scripts'],
}
def classify_str(s):
    low = s.lower()
    for z, keys in ZONE_KEYS.items():
        for k in keys:
            if k.lower() in low: return z
    return None

func_zone_votes = collections.defaultdict(collections.Counter)
for f in funcs:
    votes = func_zone_votes[f]
    # IAT API names from edges + data refs
    for e in calls_from[f]:
        if isinstance(e, tuple):
            z = classify_str(e[1])
            if z: votes[z] += 1
    for tgt in riprefs[f]:
        trva = tgt - ib
        if rdata and rdata[0].VirtualAddress <= trva < rdata[0].VirtualAddress + rdata[0].Misc_VirtualSize:
            o = rva2off(trva)
            chunk = raw[o:o+96]
            nul = chunk.find(b'\x00')
            s = chunk[:nul].decode('latin-1','ignore') if nul > 0 else ''
            if len(s) >= 6:
                z = classify_str(s)
                if z: votes[z] += 1
        api = iat_map.get(tgt)
        if api:
            z = classify_str(api)
            if z: votes[z] += 1

zones = collections.defaultdict(list)
for f, votes in func_zone_votes.items():
    if not votes: continue
    z, n = votes.most_common(1)[0]
    zones[z].append({'rva': hex(f), 'end_rva': hex(frange[f]), 'size': frange[f]-f,
                     'votes': dict(votes), 'reachable_from_dllmain_bfs': f in seen})

out = {'image_base': hex(ib), 'functions_total': len(funcs),
       'bfs_from_dllmain': {'root': hex(START), 'reachable_count': len(seen),
                            'sample_first50': [hex(x) for x in order[:50]]},
       'zones': {z: sorted(v, key=lambda d:int(d['rva'],16))[:40] for z,v in zones.items()},
       'zone_function_counts': {z: len(v) for z,v in zones.items()}}
json.dump(out, open('/workspace/analysis/callgraph_zones.json','w'), indent=1)
print("\nZone counts:", {z: len(v) for z,v in zones.items()})
print("Saved analysis/callgraph_zones.json")
