// SpinLite - viste di montaggio con tutti i componenti della BOM (solo
// immagini, non si stampa). OpenSCAD, millimetri. Licenza MIT.
//
// Include spinlite_top.scad e ne usa le quote: se cambi un parametro della
// scatola, componenti e fili si spostano con lei. I componenti sono blocchi
// semplificati con le misure tipiche dei parametri; la piedinatura della
// SuperMini e del TP4056 e' indicativa (controlla le serigrafie delle tue).
//
// La base LaunchLab (CC BY-SA) non e' nel repo. Per vederla, scarica il
// pacchetto top-dock-v098 di LaunchLab e metti in `launchlab_preview` il
// percorso della sua cartella preview/ (STL gia' in coordinate di montaggio).
// Senza, al suo posto c'e' un segnaposto schematico grigio.
//
// Immagini del repo (hardware/img/), da riga di comando:
//   openscad -o montato.png -D 'view="montato"' -D 'launchlab_preview="..."' spinlite_views.scad

include <spinlite_top.scad>
part = "none"; // l'ultima assegnazione vale: la scatola non viene disegnata da sola

// "montato", "interno", "sezione", "esploso"
view = "interno";
launchlab_preview = "";
show_launcher = true;

$fn = 24;

// ---------------------------------------------------------------------------
// Utilita'
// ---------------------------------------------------------------------------
module wire(pts, d = 0.9) {
    for (i = [0 : len(pts) - 2]) hull() {
        translate(pts[i]) sphere(d = d);
        translate(pts[i + 1]) sphere(d = d);
    }
}
module bx(p, s) translate(p) cube(s);

// ---------------------------------------------------------------------------
// Base LaunchLab (importata) o segnaposto
// ---------------------------------------------------------------------------
module launchlab_base() {
    if (launchlab_preview != "") {
        color("silver") for (f = ["01_QRE_open_holder_body", "08_QRE_latch_LEFT", "09_QRE_latch_RIGHT", "03_QRE_pressure_bar"])
            import(str(launchlab_preview, "/", f, ".stl"));
        color("dimgray") for (f = ["REF_M2x4_SENSOR_1", "REF_M2x4_SENSOR_2"])
            import(str(launchlab_preview, "/", f, ".stl"));
    } else {
        // segnaposto: piastra, rilievi dei fori, cerniere, clip
        color("gainsboro") {
            bx([-21.57, -16.48, 0], [43.14, 32.96, 7.4]);
            for (sx = [-1, 1], sy = [-1, 1]) translate([sx * pilot_x, sy * pilot_y, 7.3]) cylinder(d = 5.2, h = seat_z - 7.3);
            for (sx = [-1, 1]) bx([sx > 0 ? hinge_x0 : -hinge_x1, -hinge_y, 7.3], [hinge_x1 - hinge_x0, 2 * hinge_y, hinge_top - 7.3]);
            for (sx = [-1, 1]) bx([sx > 0 ? 16.1 : -29.9, -5.95, -3.46], [13.75, 11.9, 14]);
        }
    }
}

// Launcher: piano indicativo sotto la base
module launcher() {
    color("#4a4a4a", 0.35) bx([-45, -50, -8], [90, 105, 8]);
}

// ---------------------------------------------------------------------------
// Sensore QRE1113 su breakout, pettine a 90 gradi e spina Dupont.
// Posizioni dai riferimenti di montaggio di LaunchLab.
// ---------------------------------------------------------------------------
module qre_sensor() {
    color("firebrick") bx([-4.36, -14.22, 5.4], [9, 14, 1.0]);              // breakout
    color("black") bx([-1.21, -9.57, 3.1], [2.7, 3.4, 2.3]);                 // QRE1113, verso il basso
    color("tan") for (x = [-2.5, 1.5]) bx([x, -4, 6.4], [1.0, 2, 0.5]);     // resistenze
    color("black") bx([-3.55, -16.6, 7.1], [7.38, 2.5, 2.8]);                // pettine a 90 gradi
    color("#222") bx([-3.55, -28.5, 7.1], [7.38, 11.9, 2.8]);                // spina Dupont
}

// ---------------------------------------------------------------------------
// Componenti nella scatola (blocchi semplificati)
// ---------------------------------------------------------------------------
z_tp_top = z_floor + tp_pcb;
z_sm_top = z_shelf + sm_pcb;
// piazzole della SuperMini: lato A (x-) e lato B (x+), dal bordo USB
function sm_pad(i, side = -1) = [side * (sm_w / 2 - 1.1), sm_y1 - 1.6 - i * 2.54, z_sm_top + 0.3];
// lato A: 0 5V, 1 GND, 2 3V3, 3 GPIO4, 4 GPIO3 ... ; lato B: 5 GPIO7, 6 GPIO6
function tp_pad(x) = [x, tp_y0 + 1.2, z_tp_top + 0.3];  // OUT+ -7, B+ -3.5, B- 3.5, OUT- 7

module supermini() {
    color("#111") bx([-sm_w / 2, sm_y0, z_shelf], [sm_w, sm_l, sm_pcb]);
    color("silver") bx([-sm_usb_w / 2, sm_y1 + sm_usb_out - 7.3, z_sm_top], [sm_usb_w, 7.3, sm_usb_h]);
    color("#333") bx([-2.5, sm_y0 + 8, z_sm_top], [5, 5, 0.9]);              // ESP32-C3
    color("#666") bx([-1.5, sm_y0 + 0.8, z_sm_top], [3, 1.6, 1.1]);          // antenna
    color("whitesmoke") for (x = [sm_boot[0], -sm_boot[0]])                  // BOOT e RST
        translate([x - 1.5, sm_y1 - sm_boot[1] - 1.5, z_sm_top]) cube([3, 3, 1.5]);
    color("gold") for (side = [-1, 1], i = [0 : 7])
        translate(sm_pad(i, side) - [0.6, 0.6, 0.3]) cube([1.2, 1.2, 0.05]);
}

module tp4056() {
    color("royalblue") bx([-tp_w / 2, tp_y0, z_floor], [tp_w, tp_l, tp_pcb]);
    color("silver") bx([-tp_usb_w / 2, tp_y1 + tp_usb_out - 7.3, z_tp_top], [tp_usb_w, 7.3, tp_usb_h]);
    color("#222") { bx([-3, tp_y0 + 8, z_tp_top], [5, 4, 1.5]); bx([3, tp_y0 + 4, z_tp_top], [3, 3, 1.1]); }
    color("gold") for (x = [-7, -3.5, 3.5, 7]) translate(tp_pad(x) - [0.9, 0.9, 0.3]) cube([1.8, 1.8, 0.05]);
}

module lipo() {
    color("lightsteelblue") bx([-lipo_w / 2, lipo_y0 + fit / 2, z_floor + 0.5], [lipo_w, lipo_l, lipo_h]);
    color("khaki") bx([-lipo_w / 2, lipo_y0 + fit / 2, z_floor], [lipo_w, lipo_l, 0.5]); // gommapiuma
}

sw_pin = function(i) [in_x - sw_body[0] - 0.4, sw_y + (i - 1) * 2.5, z_floor + sw_body[2] / 2];
module switch_ss12() {
    color("#202020") bx([in_x - sw_body[0], sw_y - sw_body[1] / 2, z_floor], sw_body);
    color("white") bx([in_x - 0.1, sw_y - 0.75, z_floor + sw_body[2] / 2 - 0.75], [wall + 1.4, 1.5, 1.5]); // leva
    color("silver") for (i = [0 : 2]) translate(sw_pin(i)) rotate([0, 90, 0]) cylinder(d = 0.6, h = 0.8);
}

// partitore 100k/100k + 100 nF, saldati in linea sotto guaina termorestringente
div_p = [6, -14, z_floor + 0.5 + lipo_h + 0.8];
module divider() {
    color("tan") for (dy = [-1.2, 1.2]) translate(div_p + [-2, dy, 0]) rotate([0, 90, 0]) cylinder(d = 1.6, h = 3.2);
    color("orange") translate(div_p + [2.2, 0, 0]) cube([1.5, 1.2, 1.5], center = true);
    color("white", 0.35) translate(div_p + [-2.8, -2.3, -1.1]) cube([7.2, 4.6, 2.4]);
}

module oled_module() {
    if (oled) {
        color("navy") translate([-oled_pcb[0] / 2, oled_y - oled_pcb[1] / 2, z_in_top - 2.6]) cube([oled_pcb[0], oled_pcb[1], 1.2]);
        color("black") translate([-oled_win[0] / 2 - 2.2, oled_y - 15, z_in_top - 1.4]) cube([oled_win[0] + 4.4, 30, 1.4]);
        color("gold") for (i = [0 : 3]) translate([-3.81 + i * 2.54, oled_y - oled_pcb[1] / 2 + 1.3, z_in_top - 5.6]) cylinder(d = 0.6, h = 3);
    }
}

// ---------------------------------------------------------------------------
// Fili (percorsi indicativi; colori: rosso +, nero GND, giallo segnale)
// ---------------------------------------------------------------------------
module wiring() {
    zl = z_floor + 0.5 + lipo_h + 0.7;  // sopra la LiPo
    zh = z_sm_top + 1.6;                // sopra la SuperMini
    // sensore: dalla spina su per l'apertura, lungo il lato sinistro
    sens = [[2, "red"], [1, "black"], [4, "gold"]]; // 3V3, GND, GPIO3
    for (k = [0 : 2]) {
        x0 = -1.2 + 1.2 * k; xl = -12.6 + 0.9 * k; pad = sm_pad(sens[k][0]);
        color(sens[k][1]) wire([[x0, -28.5, 8.5], [x0, -30.3, 9.6], [x0, -30.6, 13], [x0, -30, zl - 2],
            [xl, -25, zl], [xl, bar_y0 - 1.5, zl], [xl, bar_y0 - 0.5, zh + 0.4 * k], [pad[0] - 0.8, pad[1], zh + 0.4 * k], pad]);
    }
    // LiPo -> TP4056 (B+, B-)
    color("red") wire([[-2, lipo_y1, z_floor + 3], [-3, lipo_y1 + 0.4, z_tp_top + 1.5], tp_pad(-3.5)]);
    color("black") wire([[2, lipo_y1, z_floor + 3], [3, lipo_y1 + 0.4, z_tp_top + 1.5], tp_pad(3.5)]);
    // OUT+ -> interruttore
    color("red") wire([tp_pad(-7), [-7, tp_y0 - 1, zl], [9, -2, zl], [11.4, sw_pin(0)[1], zl], sw_pin(0)]);
    // interruttore -> 5V della SuperMini, passando sopra la scheda
    p5 = sm_pad(0);
    color("red") wire([sw_pin(1), [11.4, sw_pin(1)[1], zl + 0.3], [11.4, bar_y0 - 1, zl + 0.3], [11.4, bar_y0, zh + 1.2],
        [0, p5[1] - 9, zh + 1.2], [p5[0] - 0.8, p5[1], zh], p5]);
    // OUT- -> GND della SuperMini
    pg = sm_pad(1);
    color("black") wire([tp_pad(7), [7, tp_y0 - 1, zl + 0.3], [-13.5, tp_y0 - 1, zl + 0.3], [-13.5, bar_y0 - 0.5, zh + 1.2],
        [pg[0] - 0.8, pg[1] - 0.5, zh + 1.2], pg]);
    // partitore: dal + dopo l'interruttore, a GND, a GPIO4
    p4 = sm_pad(3);
    color("red") wire([[11.4, sw_pin(1)[1] - 0.5, zl + 0.3], [10, -14, div_p[2]], div_p + [3.4, 0, 0]]);
    color("black") wire([div_p + [-2.6, -1.8, 0], [-13.5, -14, zl + 0.3], [-13.5, tp_y0 - 1.5, zl + 0.3]]);
    color("deepskyblue") wire([div_p + [-2.6, 1.2, 0], [-11.5, -10, zl + 0.6], [-11.5, bar_y0 - 1.5, zl + 0.6],
        [-11.5, bar_y0 - 0.5, zh + 0.8], [p4[0] - 0.8, p4[1], zh + 0.8], p4]);
    // OLED: SDA GPIO6, SCL GPIO7 (lato B), alimentazione 3V3 e GND (lato A)
    if (oled) {
        oy = oled_y - oled_pcb[1] / 2 + 1.3; oz = z_in_top - 5.6;
        o = [[sm_pad(6, 1), "green"], [sm_pad(5, 1), "purple"], [sm_pad(2), "red"], [sm_pad(1), "black"]];
        for (i = [0 : 3]) color(o[i][1])
            wire([[-3.81 + i * 2.54, oy, oz], [-3.81 + i * 2.54, oy - 2, oz - 1], [o[i][0][0] * 1.3, bar_y0 - 3, oz - 1],
                [o[i][0][0], o[i][0][1], zh + 1.8], o[i][0]]);
    }
}

// ---------------------------------------------------------------------------
// Viti M2x6 (4 sotto, 4 nel coperchio)
// ---------------------------------------------------------------------------
module screw_m2(l = 6) color("dimgray") { translate([0, 0, -l]) cylinder(d = 2, h = l); cylinder(d = 3.8, h = 1.3); }
module base_screws(dz = 0) for (sx = [-1, 1], sy = [-1, 1]) translate([sx * pilot_x, sy * pilot_y, screw_seat_z + dz]) screw_m2();
module lid_screws(dz = 0) for (sx = [-1, 1], y = [lid_post[1], lid_post[2]]) translate([sx * lid_post[0], y, z_in_top + lid_t + dz]) screw_m2();

module electronics() {
    tp4056(); lipo(); switch_ss12(); divider(); supermini(); oled_module(); wiring();
}

// ---------------------------------------------------------------------------
// Viste
// ---------------------------------------------------------------------------
module cut_half() intersection() { children(); translate([-60, -60, -20]) cube([60, 120, 100]); }

if (view == "montato") {
    if (show_launcher) launcher();
    launchlab_base(); qre_sensor();
    color("orange") box(); base_screws();
    color("lightblue") lid(); lid_screws();
} else if (view == "interno") {
    if (show_launcher) launcher();
    launchlab_base(); qre_sensor();
    color("orange") box(); base_screws();
    electronics();
} else if (view == "sezione") {
    if (show_launcher) launcher();
    cut_half() launchlab_base(); qre_sensor();
    color("orange") cut_half() box(); base_screws();
    color("lightblue", 0.8) cut_half() lid();
    electronics();
} else if (view == "esploso") {
    launchlab_base(); qre_sensor();
    translate([0, 0, 8]) base_screws();
    translate([0, 0, 22]) color("orange") box();
    translate([0, -18, 46]) { tp4056(); }
    translate([0, 0, 60]) { lipo(); divider(); }
    translate([10, 0, 60]) switch_ss12();
    translate([0, 0, 74]) supermini();
    translate([0, 0, 98]) { color("lightblue", 0.85) lid(); oled_module(); }
    translate([0, 0, 108]) lid_screws();
}
