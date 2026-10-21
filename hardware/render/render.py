"""Render realistici di SpinLite montato sul launcher (Blender + OpenSCAD).

Prima scarica i file di LaunchLab:  python hardware/third_party/fetch_launchlab.py
Poi:                                python hardware/render/render.py [campioni]

Esporta i pezzi SpinLite da OpenSCAD in hardware/render/build/ (ignorata da git)
e scrive le immagini in hardware/img/render_*.png.
Percorsi dei programmi: variabili OPENSCAD e BLENDER, altrimenti i default Windows.
"""
import glob
import os
import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent
HW = HERE.parent
ENC = HW / "enclosure"
LL = HW / "third_party" / "launchlab"
BUILD = HERE / "build"
IMG = HW / "img"

OPENSCAD = os.environ.get("OPENSCAD", r"C:\Program Files\OpenSCAD\openscad.exe")
BLENDER = os.environ.get("BLENDER") or next(iter(sorted(glob.glob(r"C:\Program Files\Blender Foundation\Blender *\blender.exe"))), "blender")

# pezzi in coordinate di montaggio, da spinlite_views.scad (che include spinlite_top.scad)
PARTS = {
    "a_box": "box();",
    "a_lid": "lid();",
    "a_screws": "base_screws(); lid_screws();",
}
# tratto esterno dei 3 fili del sensore: dalla spina su per l'apertura del fondo
WIRE = ("x0 = -1.2 + 1.2 * {k}; xl = -12.6 + 0.9 * {k}; zl = z_floor + 0.5 + lipo_h + 0.7;"
        " wire([[x0, -28.5, 8.5], [x0, -30.3, 9.6], [x0, -30.6, 13], [x0, -30, zl - 2], [xl, -25, zl]], d = 1.1);")
for k in range(3):
    PARTS[f"a_wire{k}"] = WIRE.format(k=k)

SHOTS = [  # opzione, vista, file
    ("A", "tre_quarti", "render_qre.png"),
    ("A", "lato", "render_qre_lato.png"),
    ("B", "tre_quarti_dx", "render_tcrt.png"),
]


def scad(name: str, body: str) -> None:
    src = BUILD / f"{name}.scad"
    src.write_text(f'include <{(ENC / "spinlite_views.scad").as_posix()}>\nview = "none";\n$fn = 64;\n{body}\n')
    subprocess.run([OPENSCAD, "-o", str(BUILD / f"{name}.stl"), str(src)], check=True, capture_output=True)


def main() -> int:
    samples = sys.argv[1] if len(sys.argv) > 1 else "160"
    if not (LL / "top-qre-raw.glb").exists():
        print("mancano i file LaunchLab: python hardware/third_party/fetch_launchlab.py")
        return 1
    BUILD.mkdir(exist_ok=True)
    for name, body in PARTS.items():
        scad(name, body); print("esportato", name)
    for part in ("box", "lid"):
        subprocess.run([OPENSCAD, "-o", str(BUILD / f"b_{part}.stl"), "-D", f'part="{part}"', "-D", "$fn=64",
                        str(ENC / "spinlite_box.scad")], check=True, capture_output=True)
        print("esportato", f"b_{part}")
    for opt, view, out in SHOTS:
        print("render", out, "...", flush=True)
        subprocess.run([BLENDER, "-b", "--python", str(HERE / "scene.py"), "--", opt, str(BUILD), str(LL),
                        str(IMG / out), view, samples], check=True, capture_output=True)
    print("fatto")
    return 0


if __name__ == "__main__":
    sys.exit(main())
