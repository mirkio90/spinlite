// SpinLite - immagini di montaggio della scatola piatta, passo per passo.
// OpenSCAD, licenza MIT. In ogni passo i pezzi gia' montati stanno al loro posto,
// quello nuovo e' staccato e una freccia magenta dice dove entra.
// Rigenerare le immagini: python hardware/render/render_montaggio.py
include <spinlite_flat.scad>
part = "none";
step = 1;   // 1..9
$fn = 40;

C_BODY = "wheat";
C_LID  = "lightblue";
C_TRAY = "#3a3a3a";
C_CLIP = "lightslategray";

// freccia magenta lungo un asse: p = punta, d = verso in cui punta, l = lunghezza
module arrow(p, d, l = 10) {
    rot = d == [0, 0, 1] ? [0, 0, 0] : d == [0, 0, -1] ? [180, 0, 0] :
          d == [1, 0, 0] ? [0, 90, 0] : d == [-1, 0, 0] ? [0, -90, 0] :
          d == [0, 1, 0] ? [-90, 0, 0] : [90, 0, 0];
    color("magenta") translate(p) rotate(rot) translate([0, 0, -l]) {
        cylinder(d = 1.4, h = l - 4);
        translate([0, 0, l - 4]) cylinder(d1 = 4, d2 = 0, h = 4);
    }
}

module tray_set()  { color(C_TRAY) tray(); qre(); tact(); }
module lid_set()   { color(C_LID) lid(); oled_module(); button(); }
module switch_screws() color("silver")
    for (s = [-1, 1]) translate([19, sf_y + s * sf_pitch / 2, sf_z]) rotate([0, 90, 0]) {
        cylinder(d = 3.8, h = 1.3);
        translate([0, 0, -6]) cylinder(d = 2, h = 6);
    }
module lid_screws() color("silver") post_positions() translate([0, 0, z_top]) {
    cylinder(d = 3.8, h = 1.3);
    translate([0, 0, -6]) cylinder(d = 2, h = 6);
}
module clips() { color(C_CLIP) { mounted_clip(0, false); mounted_clip(0, true); } }

// ciascun passo mostra quello che c'e' gia' piu' il pezzo nuovo, staccato.
// Le frecce stanno di lato al pezzo, dalla posizione staccata a quella di montaggio.
if (step == 1) {                      // QRE1113 e tasto nel vassoio
    color(C_TRAY) tray();
    translate([0, 0, 10]) { qre(); tact(); }
    arrow([-9.5, sensor_y, tray_top + 1], [0, 0, -1], 9);
    arrow([5.5, button_y, tray_top + 1], [0, 0, -1], 9);
} else if (step == 2) {               // vassoio nel corpo, da sotto
    color(C_BODY) body();
    translate([0, 0, -16]) tray_set();
    arrow([11, 9, body_z - 0.5], [0, 0, 1], 12);
} else if (step == 3) {               // SuperMini di traverso, USB-C nel fianco -X
    color(C_BODY) body(); tray_set();
    translate([0, 0, 14]) supermini();
    arrow([-6, sm_y1 + 3, z_sm + 2], [0, 0, -1], 11);
} else if (step == 4) {               // interruttore sul fianco +X, viti da fuori
    color(C_BODY) body(); tray_set(); supermini();
    translate([0, 0, 14]) switch_sf();
    arrow([13, sf_y + 11, sf_z + 4], [0, 0, -1], 10);
    translate([8, 0, 0]) switch_screws();
    arrow([27, sf_y, sf_z + 4], [-1, 0, 0], 7);
} else if (step == 5) {               // TP4056 e LiPo
    color(C_BODY) body(); tray_set(); supermini(); switch_sf(); switch_screws();
    translate([0, 0, 12]) tp4056();
    arrow([-13, (tp_y0 + tp_y1) / 2, z_floor + 3], [0, 0, -1], 10);
    translate([0, 0, 18]) lipo_pack();
    arrow([-12.5, lipo_y0 + 15, z_floor + 7], [0, 0, -1], 12);
} else if (step == 6) {               // tasto esterno sulla torretta
    color(C_BODY) body(); tray_set(); supermini(); switch_sf(); switch_screws(); tp4056(); lipo_pack();
    translate([0, 0, 10]) btn_tact();
    arrow([btn_x + 5.5, btn_y, z_btn_tact + 4], [0, 0, -1], 9);
} else if (step == 7) {               // OLED e cappuccio nel coperchio (coperchio capovolto)
    translate([0, 0, 2 * z_top]) mirror([0, 0, 1]) {
        color(C_LID) lid();
        translate([0, 0, -12]) { oled_module(); button(); }
    }
    arrow([9, oled_y, z_top + 5], [0, 0, -1], 9);
    arrow([btn_x - 5, btn_y, z_top + 5], [0, 0, -1], 9);
} else if (step == 8) {               // clip sul blocco della cerniera
    color(C_BODY) body(); tray_set(); supermini(); switch_sf(); switch_screws(); tp4056(); lipo_pack(); btn_tact();
    translate([10, 0, 6]) color(C_CLIP) mounted_clip(0, false);
    translate([-10, 0, 6]) color(C_CLIP) mounted_clip(0, true);
    arrow([pivot[0] + 2, clip_y - 7, pivot[1] + 3], [-1, 0, 0], 9);
    arrow([-pivot[0] - 2, clip_y - 7, pivot[1] + 3], [1, 0, 0], 9);
} else if (step == 9) {               // coperchio e 9 viti
    color(C_BODY) body(); tray_set(); supermini(); switch_sf(); switch_screws(); tp4056(); lipo_pack(); btn_tact(); clips();
    translate([0, 0, 22]) { lid_set(); lid_screws(); }
    arrow([23, 40, z_top + 1], [0, 0, -1], 20);
}
