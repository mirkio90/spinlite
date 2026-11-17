"""Immagini di montaggio della scatola piatta, passo per passo (OpenSCAD).

Uso: python hardware/render/render_montaggio.py
Scrive hardware/img/montaggio/passo_N.png da enclosure/spinlite_flat_montaggio.scad.
Percorso di OpenSCAD: variabile OPENSCAD, altrimenti il default Windows.
"""
import os
import pathlib
import subprocess

HERE = pathlib.Path(__file__).resolve().parent
SCAD = HERE.parent / "enclosure" / "spinlite_flat_montaggio.scad"
OUT = HERE.parent / "img" / "montaggio"
OPENSCAD = os.environ.get("OPENSCAD", r"C:\Program Files\OpenSCAD\openscad.exe")

# passo: rotazione della camera (inclinazione, 0, azimut), cioe' da dove si guarda
VIEWS = {
    1: (40, 0, 30),     # vassoio dall'alto
    2: (125, 0, 30),    # corpo da sotto
    3: (55, 0, -120),   # dall'alto, lato -X con il foro della USB-C
    4: (65, 0, 60),     # dal fianco +X
    5: (50, 0, 30),
    6: (50, 0, -120),
    7: (35, 0, 30),     # coperchio capovolto, interno in alto
    8: (60, 0, 60),
    9: (55, 0, 25),
}


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    for step, (rx, ry, rz) in VIEWS.items():
        out = OUT / f"passo_{step}.png"
        subprocess.run([OPENSCAD, "-o", str(out), "-D", f"step={step}", "--imgsize=1400,1000",
                        "--colorscheme=Tomorrow", f"--camera=0,0,0,{rx},{ry},{rz},0", "--autocenter", "--viewall",
                        str(SCAD)], check=True, capture_output=True)
        print("scritto", out.name)


if __name__ == "__main__":
    main()
