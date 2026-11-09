"""Render realistici di SpinLite montato sul launcher (Blender + OpenSCAD).

Prima scarica i file di LaunchLab:  python hardware/third_party/fetch_launchlab.py
Poi:                                python hardware/render/render.py [campioni] [opzioni, es. C]

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
    "a_switch": "switch_ss12();",
}
# tratto esterno dei 3 fili del sensore: dalla spina su per l'apertura del fondo
WIRE = ("x0 = -1.2 + 1.2 * {k}; xl = -12.6 + 0.9 * {k}; zl = z_floor + 0.5 + lipo_h + 0.7;"
        " wire([[x0, -28.5, 8.5], [x0, -30.3, 9.6], [x0, -30.6, 13], [x0, -30, zl - 2], [xl, -25, zl]], d = 1.1);")
for k in range(3):
    PARTS[f"a_wire{k}"] = WIRE.format(k=k)

# scatola piatta, da spinlite_flat.scad: le viti e la leva qui, gli altri pezzi sono moduli del file
PARTS_C = {
    "c_body": "body();",
    "c_tray": "tray();",
    "c_lid": "lid();",
    "c_clip_r": "mounted_clip(0, false);",
    "c_clip_l": "mounted_clip(0, true);",
    "c_elec": "supermini(); tp4056(); oled_module(); btn_tact(); button();",
    "c_switch": ("switch_sf();\n"
                 "translate([19 - wall - 0.5, sf_y - 2.5, sf_z - sf_lever[1] / 2]) cube([wall + sf_lever[2] + 0.5, sf_lever[0], sf_lever[1]]);"),
    "c_screws": ("post_positions() translate([0, 0, z_top]) cylinder(d = 3.8, h = 1.3);\n"
                 "for (s = [-1, 1]) translate([19, sf_y + s * sf_pitch / 2, sf_z]) rotate([0, 90, 0]) cylinder(d = 3.8, h = 1.3);"),
}

SHOTS = [  # opzione, vista, file
    ("A", "tre_quarti", "render_qre.png"),
    ("A", "lato", "render_qre_lato.png"),
    ("B", "tre_quarti_dx", "render_tcrt.png"),
    ("C", "c_tre_quarti", "render_piatta.png"),
    ("C", "c_lato", "render_piatta_lato.png"),
    ("C", "c_fronte", "render_piatta_fronte.png"),
    ("C", "c_clip", "render_piatta_clip.png"),
    ("L", "l_tre", "render_launcher.png"),
]


def scad(name: str, body: str, source: str = "spinlite_views.scad", setup: str = 'view = "none";') -> None:
    src = BUILD / f"{name}.scad"
    src.write_text(f'include <{(ENC / source).as_posix()}>\n{setup}\n$fn = 64;\n{body}\n')
    subprocess.run([OPENSCAD, "-o", str(BUILD / f"{name}.stl"), str(src)], check=True, capture_output=True)


def main() -> int:
    samples = sys.argv[1] if len(sys.argv) > 1 else "160"
    options = sys.argv[2] if len(sys.argv) > 2 else "ABCL"  # quali configurazioni: A, B, C, L (solo launcher)
    if set(options) & set("AB") and not (LL / "top-qre-raw.glb").exists():
        print("mancano i file LaunchLab: python hardware/third_party/fetch_launchlab.py")
        return 1
    BUILD.mkdir(exist_ok=True)
    if "A" in options or "B" in options:
        for name, body in PARTS.items():
            scad(name, body); print("esportato", name)
        for part in ("box", "lid"):
            subprocess.run([OPENSCAD, "-o", str(BUILD / f"b_{part}.stl"), "-D", f'part="{part}"', "-D", "$fn=64",
                            str(ENC / "spinlite_box.scad")], check=True, capture_output=True)
            print("esportato", f"b_{part}")
    if "C" in options:
        for name, body in PARTS_C.items():
            scad(name, body, "spinlite_flat.scad", 'part = "none";'); print("esportato", name)
    for opt, view, out in SHOTS:
        if opt not in options:
            continue
        print("render", out, "...", flush=True)
        subprocess.run([BLENDER, "-b", "--python", str(HERE / "scene.py"), "--", opt, str(BUILD), str(LL),
                        str(IMG / out), view, samples], check=True, capture_output=True)
    print("fatto")
    return 0


if __name__ == "__main__":
    sys.exit(main())
