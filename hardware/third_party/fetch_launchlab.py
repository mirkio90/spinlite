"""Scarica i file hardware di LaunchLab usati da SpinLite in third_party/launchlab/.

I file di LaunchLab sono CC BY-SA (derivati dal BP Gear Port Connector di
Migbello, Thingiverse 6856080): servono in locale per stampare la base e per
le viste/render, ma NON vanno nel repo (la cartella e' nel .gitignore).

Tutti i link puntano allo stesso commit di LaunchLab, cosi' le quote su cui e'
disegnata la scatola SpinLite non cambiano.

Uso:  python hardware/third_party/fetch_launchlab.py
"""
import hashlib
import io
import pathlib
import shutil
import subprocess
import sys
import urllib.request
import zipfile

COMMIT = "87bb60d619c2127c9efd679d7505a144e52a30d8"
RAW = f"https://github.com/shadohead/launchlab/raw/{COMMIT}/site/public/hardware"

# (file remoto, cosa contiene)
FILES = [
    ("downloads/compact-base-v102.zip", "base QRE1113 v102: STL, STEP, licenza (da stampare)"),
    ("downloads/compact-base-v102.3mf", "base QRE1113 v102: progetto di stampa con i blocchi per le cerniere"),
    ("downloads/top-dock-v098.zip", "dock M5 v098: cartella preview/ con le parti in coordinate di montaggio"),
    ("downloads/standard-sensor-v102.zip", "supporto laterale TCRT5000 v102"),
    ("models/top-qre.glb", "modello del sito: base QRE montata (compresso meshopt)"),
    ("models/side-tcrt.glb", "modello del sito: supporto TCRT montato (compresso meshopt)"),
]

DEST = pathlib.Path(__file__).resolve().parent / "launchlab"


def fetch(url: str) -> bytes:
    with urllib.request.urlopen(url, timeout=60) as r:
        return r.read()


def main() -> int:
    DEST.mkdir(parents=True, exist_ok=True)
    lines = [f"LaunchLab, commit {COMMIT}", "Licenza: CC BY-SA (Migbello / LaunchLab). Non ridistribuire senza attribuzione.", ""]
    for rel, what in FILES:
        url = f"{RAW}/{rel}"
        name = rel.split("/")[-1]
        print(f"scarico {name} ...", end=" ", flush=True)
        data = fetch(url)
        (DEST / name).write_bytes(data)
        lines.append(f"{name}  sha256={hashlib.sha256(data).hexdigest()}  {what}")
        if name.endswith(".zip"):
            # ogni pacchetto ha gia' una sua cartella radice
            zipfile.ZipFile(io.BytesIO(data)).extractall(DEST)
        print(f"{len(data) // 1024} KB")
    # I .glb del sito sono compressi (EXT_meshopt_compression) e Blender non li
    # apre: li decomprimo con gltf-transform, se c'e' Node (npx).
    npx = shutil.which("npx")
    for name in ("top-qre.glb", "side-tcrt.glb"):
        raw = DEST / name.replace(".glb", "-raw.glb")
        if not npx:
            print(f"npx non trovato: {raw.name} non creato (serve solo per i render)")
            continue
        cmd = [npx, "-y", "@gltf-transform/cli@4", "copy", str(DEST / name), str(raw)]
        ok = subprocess.run(cmd, capture_output=True).returncode == 0
        print(f"decompresso {raw.name}" if ok else f"decompressione di {name} fallita")
    (DEST / "PROVENIENZA.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"fatto: {DEST}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
