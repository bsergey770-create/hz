#!/usr/bin/env python3
"""Step 3: extract embedded SVG icon blocks from the .nlart section of neverwin.dll."""
import os, re, sys, json
import pefile

DLL = "/workspace/neverwin.dll"
OUT = "/workspace/analysis/icons"

pe = pefile.PE(DLL, fast_load=True)
sec = next((s for s in pe.sections if s.Name.rstrip(b"\x00") == b".nlart"), None)
if sec is None:
    sys.exit("[!] .nlart section not found")

raw = sec.get_data()
print(f"[+] .nlart: VA={hex(pe.OPTIONAL_HEADER.ImageBase+sec.VirtualAddress)} "
      f"vsize={hex(sec.Misc_VirtualSize)} rawbytes={len(raw)}")

# Scan for <svg ... > ... </svg> text blocks (case-insensitive, tolerate NUL padding between blocks)
pat = re.compile(rb"<svg\b.*?</svg\s*>", re.DOTALL | re.IGNORECASE)
found = list(pat.finditer(raw))
print(f"[+] candidate <svg>...</svg> blocks: {len(found)}")

os.makedirs(OUT, exist_ok=True)
manifest = []
seen = {}
for i, m in enumerate(found):
    blob = m.group(0)
    # decode as utf-8 tolerantly; svg must be mostly ascii
    try:
        txt = blob.decode("utf-8")
    except UnicodeDecodeError:
        txt = blob.decode("utf-8", errors="replace")
    if "<path" not in txt and "<rect" not in txt and "<circle" not in txt and "<g" not in txt:
        continue  # skip degenerate/empty matches
    key = txt.strip()
    if key in seen:
        continue
    idx = len(manifest)
    # name hint: id= or first comment or viewBox
    mh = re.search(r'id="([^"]+)"', txt)
    hint = mh.group(1) if mh else f"icon_{idx:03d}"
    hint = re.sub(r"[^A-Za-z0-9_.-]", "_", hint)[:48]
    fname = f"{idx:03d}_{hint}.svg"
    with open(os.path.join(OUT, fname), "w", encoding="utf-8") as f:
        f.write(txt)
    off = m.start()
    va = pe.OPTIONAL_HEADER.ImageBase + sec.VirtualAddress + off
    manifest.append({"file": fname, "section_offset": hex(off), "va": hex(va),
                     "rva": hex(sec.VirtualAddress + off), "bytes": len(blob),
                     "name_hint": hint})
    seen[key] = fname

print(f"[+] unique icons written to {OUT}: {len(manifest)}")
with open(os.path.join(OUT, "manifest.json"), "w", encoding="utf-8") as f:
    json.dump({"section": ".nlart", "count": len(manifest), "icons": manifest}, f, indent=1)
for e in manifest[:20]:
    print("   ", e["file"], e["rva"], e["bytes"])
