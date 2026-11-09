// SpinLite - corpo unico piatto che si aggancia direttamente al launcher.
// OpenSCAD, millimetri. Licenza MIT.
//
// Non serve la base LaunchLab: sensore e tasto di presenza stanno in un vassoio
// che entra nel fondo da sotto, le clip sono due pezzi stampati a parte. La geometria e' disegnata qui da zero.
// Dal progetto Beyblade-Dragoon-RPM di Luciandraw (modelli CC BY-NC-SA, non
// inclusi e non copiati) vengono solo idee e quote funzionali, cioe' la
// posizione reciproca di clip, sensore e pistoncino del launcher.
//
// Sistema di riferimento: X di traverso, Y lungo il launcher, Z verso l'alto.
//
// Disposizione lungo Y:
//   dietro  SuperMini (USB-C nella parete posteriore), interruttore sul fianco +X
//   centro  QRE1113 e tasto di presenza nel fondo, clip ai lati
//   davanti TP4056 di traverso (USB-C sul fianco +X), poi LiPo
// Montaggio: docs/scatola-piatta.md

// ---- Cosa generare: "body", "lid", "tray", "clips", "button", "plate" (tutti), "assembly",
//      "prova" (solo la zona centrale, per provare aggancio, sensore e tasto con poco filamento) ----
// ("mounted_lid", "mounted_clip" e "check_*" servono solo per i controlli)
part = "plate";
clip_angle = 0;     // per "mounted_clip": rotazione attorno al perno, in gradi

// ---- Interfaccia con il launcher (quote funzionali: non cambiare senza provare) ----
body_z    = 6.65;   // faccia inferiore del corpo
band_z    = 5.65;   // fascia centrale, entra nell'incavo del launcher
band_x    = 8.9;    // mezza larghezza della fascia
band_y    = [-1, 19];
tray_x    = 8.2;    // mezza larghezza della parte del vassoio dentro il corpo
tray_top  = 8.6;
sensor_y  = 5.44;   // centro del QRE1113
sensor_face_z = 5.19; // faccia del QRE1113
sensor_win = [4.6, 3.6]; // finestra nel fondo: x, y
button_y  = 13.05;  // centro del pistoncino
button_tip_z = 5.40; // punta del cappuccio del tasto
button_hole = 4.3;  // foro nel fondo per il cappuccio
hinge_y   = [9.3, 16.7];  // blocchi delle cerniere
hinge_x   = [16, 18.9];
hinge_top = 10.3;
clip_y    = 13.14;  // centro delle clip lungo Y
clip_gap  = 8.0;    // distanza interna tra i bracci della clip
clip_arm  = 2.0;    // spessore dei bracci
pivot     = [17.7, 11.7]; // asse delle clip: x, z

// ---- Pareti e viti ----
wall      = 1.6;
floor_t   = 1.0;
lid_t     = 1.6;
lip_h     = 1.1;
lip_t     = 1.2;
clearance = 0.2;
eps       = 0.05;
m2_pilot  = 1.7;
m2_clear  = 2.3;
post_d    = 5;
posts = [[14.5, -30], [14.5, -4], [13.0, 21.8], [15.8, 49.2], [13.2, 61]];

// ---- Componenti (misura i tuoi e correggi) ----
fit       = 0.4;
// QRE1113 su breakout: lato lungo di traverso, sensore al centro
qre_pcb   = [14, 9, 1.6];   // x, y, spessore
qre_h     = 1.8;            // altezza del sensore sotto il PCB
// Tasto SMD 6x6, sul fondo con il cappuccio verso il launcher
tact_body = [6.0, 6.0, 2.2];
tact_h    = 3.4;            // altezza totale, cappuccio compreso
tact_legs = 8.0;
// ESP32-C3 SuperMini
sm_l = 22.5; sm_w = 18; sm_pcb = 1.0;
sm_usb = [9.0, 3.3]; sm_usb_out = 0.6;
sm_rail = 1.0;              // aria sotto la scheda
sm_top_h = 3.6;
// TP4056 con USB-C
tp_l = 28; tp_w = 17.2; tp_pcb = 1.6;
tp_usb = [9.0, 3.3]; tp_usb_out = 0.6;
// LiPo 502030
lipo = [20, 30, 5];         // x, y, z
lipo_foam = 0.5;
// Interruttore SS12F15 (corsa e fori stimati, da misurare)
sf_plate = [19.5, 6.1]; sf_body = [10.5, 6.1, 5.3]; sf_pins = 1.5;
sf_lever = [3.0, 2.5, 4.0]; sf_travel = 3.5; sf_pitch = 15; sf_hole = 2.3;
sf_y     = -17;
usb_hole = [9.8, 3.9];
usb_pocket = [13, 0.8];

// OLED 128x32 da 0,91": sotto il coperchio, sopra il vassoio. Non alza il corpo.
oled      = true;
oled_pcb  = [12.2, 38.2, 1.2]; // x, y, spessore (misura il tuo)
oled_glass = [11.5, 30, 1.4];  // vetro: x, y, spessore
oled_win  = [6.5, 23];         // area visibile: x, y
oled_win_dy = 0;               // scostamento dell'area visibile dal centro del PCB (misura il tuo)
oled_y0   = -7.2;              // bordo del PCB dal lato dei pin, verso la SuperMini
oled_fit  = 0.3;
oled_ring = 2.4;               // altezza della cornice sotto il coperchio
// Tasto esterno, in parallelo al BOOT (GPIO9): stesso tasto SMD della presenza + cappuccio stampato
btn       = true;
btn_x     = -11;
btn_y     = 13;
btn_d     = 4.0;   // gambo del cappuccio
btn_hole  = 4.4;
btn_flange = 6.5;  // flangia sotto il coperchio: il cappuccio non puo' uscire
btn_out   = 1.0;   // quanto sporge dal coperchio
// Zone sottili del coperchio: fanno passare la luce dei LED con filamento chiaro
led_t     = 0.6;   // spessore residuo
led_zones = [[-8, -3, -29.5, -12.5], [3, 8, -29.5, -12.5], [-3, 3, -24.8, -12.5], // LED della SuperMini
             [-9, 10.5, 33, 45.5]];                                               // LED del TP4056
// Giochi e spezzone di prova
tray_gap  = 0.25;  // per lato, tra vassoio e apertura nel corpo
coupon_y  = [-6.5, 24.5];

$fn = 40;

// ---------------------------------------------------------------------------
// Quote derivate
// ---------------------------------------------------------------------------
z_floor   = body_z + floor_t;          // piano interno
z_plate   = band_z + 0.95;             // cima del fondo nella zona del sensore
z_lid     = 14.4;                      // appoggio del coperchio
tray_y0   = band_y[0] + 0.7;
tray_y1   = band_y[1] - 0.7;
z_top     = z_lid + lid_t;
tact_cap_l = tact_h - tact_body[2];
z_tact    = button_tip_z + tact_cap_l; // base del corpo del tasto
z_qre_pcb = sensor_face_z + qre_h;     // faccia inferiore del PCB del QRE

sm_y0  = -33 + wall + 0.2; sm_y1 = sm_y0 + sm_l;
z_sm   = z_floor + sm_rail;
z_sm_usb = z_sm + sm_pcb + sm_usb[1] / 2;

tp_y0  = 29.0; tp_y1 = tp_y0 + tp_w;
tp_x1  = 19 - wall - 0.2; tp_x0 = tp_x1 - tp_l;
z_tp_usb = z_floor + tp_pcb + tp_usb[1] / 2;

lipo_y0 = tp_y1 + 2.0; lipo_y1 = lipo_y0 + lipo[1];
sf_z   = z_floor + 0.2 + sf_plate[1] / 2;
sf_slot = [sf_lever[0] + sf_travel + 0.6, sf_lever[1] + 0.6];

oled_y    = oled_y0 + oled_pcb[1] / 2;
z_oled    = z_lid - oled_glass[2] - oled_pcb[2]; // faccia inferiore del PCB dell'OLED
z_btn     = z_lid - 1.0;                         // faccia inferiore della flangia del cappuccio
z_btn_tact = z_btn - tact_h;                     // base del tasto esterno

assert(!oled || z_oled >= z_tact + tact_body[2] + 2.5, "manca lo spazio per i fili sotto l'OLED");
assert(!oled || oled_y0 + oled_pcb[1] < tp_y0 + 3, "OLED troppo lungo: arriva sopra il chip del TP4056");
assert(!btn || btn_x + btn_flange / 2 <= -oled_pcb[0] / 2 - oled_fit / 2 - 1.2 - 0.2, "cappuccio contro la cornice dell'OLED");
assert(!btn || z_btn_tact > z_floor, "tasto esterno troppo alto");

assert(z_tact >= z_plate - 0.45, "cappuccio del tasto troppo corto: il fondo resta troppo sottile");
assert(z_tact <= z_plate + 0.01, "cappuccio del tasto troppo lungo: la punta supera la quota prevista");
assert(z_qre_pcb >= z_plate, "sensore troppo basso per il fondo");
assert(sensor_y + qre_pcb[1] / 2 <= button_y - tact_body[1] / 2, "PCB del QRE contro il tasto: misura il tuo");
assert(sf_z + sf_plate[1] / 2 < z_lid - 0.3, "interruttore troppo alto");
assert(lipo_y1 + 0.3 < 80.5 - wall, "LiPo troppo lunga");
assert(tp_x0 > -19 + wall, "TP4056 troppo lungo");

// ---------------------------------------------------------------------------
// Pianta
// ---------------------------------------------------------------------------
half = [[-33, 17], [-31, 19], [-1, 19], [1.5, 16], [21.5, 16], [26, 18.3], [30, 19], [50, 19], [56, 18.2], [62, 17.2], [68, 16], [74, 11.9], [80.5, 11.9]];

module outline() {
    r = [for (p = half) [p[1], p[0]]];
    l = [for (i = [len(half) - 1 : -1 : 0]) [-half[i][1], half[i][0]]];
    polygon(concat(r, l));
}

module prism(z0, z1) translate([0, 0, z0]) linear_extrude(z1 - z0) children();

// ---------------------------------------------------------------------------
// Corpo
// ---------------------------------------------------------------------------
module hinge_blocks() {
    for (s = [-1, 1]) translate([s > 0 ? hinge_x[0] - eps : -hinge_x[1], hinge_y[0], body_z])
        cube([hinge_x[1] - hinge_x[0] + eps, hinge_y[1] - hinge_y[0], hinge_top - body_z]);
}

module post_positions() for (s = [-1, 1], p = posts) translate([s * p[0], p[1], 0]) children();

module supports() {
    // binari e battute della SuperMini
    for (s = [-1, 1]) {
        translate([s * (sm_w / 2 - 0.5) - 0.5, sm_y0 + 1, z_floor - eps]) cube([1, sm_l - 2, sm_rail + eps]);
        translate([s > 0 ? sm_w / 2 + fit / 2 : -sm_w / 2 - fit / 2 - 1, sm_y0, z_floor - eps])
            cube([1, sm_l, sm_rail + sm_pcb + 0.5]);
    }
    translate([-sm_w / 2 - fit / 2, sm_y1 + fit / 2, z_floor - eps]) cube([sm_w + fit, 1, sm_rail + sm_pcb + 0.5]);
    // guide del TP4056
    for (y = [tp_y0 - fit / 2 - 1.2, tp_y1 + fit / 2])
        translate([tp_x0, y, z_floor - eps]) cube([tp_l - 8, 1.2, 1.6]);
    // guide della LiPo
    for (s = [-1, 1]) translate([s > 0 ? lipo[0] / 2 + fit / 2 : -lipo[0] / 2 - fit / 2 - 1.2, lipo_y0 + 4, z_floor - eps])
        cube([1.2, lipo[1] - 8, 2]);
    // colonnine del coperchio
    post_positions() translate([0, 0, z_floor - eps]) cylinder(d = post_d, h = z_lid - z_floor + eps);
    // torretta del tasto esterno: regge la spinta del dito, i piedini restano liberi lungo Y
    if (btn) translate([-14.5, btn_y - 2.5, z_floor - eps]) cube([5.9, 5, z_btn_tact - z_floor + eps]);
}

// Sponde che tengono in posizione PCB del sensore e tasto
module sensor_ribs() {
    h = tray_top;
    px = qre_pcb[0] / 2 + fit / 2;
    py = qre_pcb[1] / 2 + fit / 2;
    translate([0, sensor_y, 0]) {
        // appoggi sotto il PCB, ai due lati della finestra
        for (s = [-1, 1]) translate([s * (px - 1.5) - 1.5, -py + 0.5, z_plate - eps])
            cube([3, 2 * py - 1, z_qre_pcb - z_plate + eps]);
        // battute alle estremita' del lato lungo; quella su -X lascia uscire i fili
        translate([px, -3, z_plate - eps]) cube([0.8, 6, h - z_plate]);
        translate([-px - 0.8, 2, z_plate - eps]) cube([0.8, 2.5, h - z_plate]);
        translate([-px - 0.8, -4.5, z_plate - eps]) cube([0.8, 2.5, h - z_plate]);
        // sponda sul lato verso il retro
        translate([-5, -py - 0.8, z_plate - eps]) cube([10, 0.8, h - z_plate]);
    }
    translate([0, button_y, 0]) {
        tx = tact_body[0] / 2 + 0.15;
        ty = tact_body[1] / 2 + 0.15;
        // sponde del tasto: lasciano liberi i piedini sui lati X
        translate([-2.5, ty, z_plate - eps]) cube([5, 1, h - z_plate]);
        for (s = [-1, 1]) translate([s * tx + (s < 0 ? -1 : 0), -1, z_plate - eps]) cube([1, 2, h - z_plate]);
    }
}

module cutouts() {
    // apertura per il vassoio del sensore
    translate([-tray_x - tray_gap, tray_y0 - tray_gap, body_z - 1]) cube([2 * tray_x + 2 * tray_gap, tray_y1 - tray_y0 + 2 * tray_gap, floor_t + 2]);
    // USB-C SuperMini, parete posteriore
    translate([-usb_hole[0] / 2, -34, z_sm_usb - usb_hole[1] / 2]) cube([usb_hole[0], wall + 2, usb_hole[1]]);
    translate([-usb_pocket[0] / 2, -34, z_sm_usb - usb_hole[1] / 2 - 1.2]) cube([usb_pocket[0], usb_pocket[1] + 1, usb_hole[1] + 2.4]);
    // USB-C TP4056, fianco +X
    translate([19 - wall - 1, (tp_y0 + tp_y1) / 2 - usb_hole[0] / 2, z_tp_usb - usb_hole[1] / 2]) cube([wall + 2, usb_hole[0], usb_hole[1]]);
    translate([19 - usb_pocket[1], (tp_y0 + tp_y1) / 2 - usb_pocket[0] / 2, z_tp_usb - usb_hole[1] / 2 - 1.2]) cube([usb_pocket[1] + 1, usb_pocket[0], usb_hole[1] + 2.4]);
    // interruttore, fianco +X
    translate([19 - wall - 1, sf_y - sf_slot[0] / 2, sf_z - sf_slot[1] / 2]) cube([wall + 2, sf_slot[0], sf_slot[1]]);
    for (s = [-1, 1]) translate([19 - wall - 1, sf_y + s * sf_pitch / 2, sf_z]) rotate([0, 90, 0]) cylinder(d = sf_hole, h = wall + 2);
    // fori pilota del coperchio
    post_positions() translate([0, 0, z_lid - 7]) cylinder(d = m2_pilot, h = 8);
}

module body() {
    difference() {
        union() {
            difference() {
                prism(body_z, z_lid) outline();
                prism(z_floor, z_lid + 1) offset(delta = -wall) outline();
            }
            hinge_blocks();
            supports();
        }
        cutouts();
    }
}

// ---------------------------------------------------------------------------
// Vassoio del sensore: entra da sotto, la fascia resta fuori e sporge di 1 mm.
// A parte perche' il corpo si stampa sul fondo, che deve essere piano. La
// spinta del pistoncino lo preme contro il corpo.
// ---------------------------------------------------------------------------
module tray() {
    difference() {
        union() {
            difference() {
                union() {
                    translate([-band_x, band_y[0], band_z]) cube([2 * band_x, band_y[1] - band_y[0], body_z - band_z]);
                    translate([-tray_x, tray_y0, body_z - eps]) cube([2 * tray_x, tray_y1 - tray_y0, tray_top - body_z + eps]);
                }
                translate([-tray_x + 0.6, tray_y0 + 0.6, z_plate]) cube([2 * tray_x - 1.2, tray_y1 - tray_y0 - 1.2, 5]);
            }
            sensor_ribs();
        }
        translate([-sensor_win[0] / 2, sensor_y - sensor_win[1] / 2, band_z - 1]) cube([sensor_win[0], sensor_win[1], 3]);
        translate([0, button_y, band_z - 1]) cylinder(d = button_hole, h = 3);
        // sede del tasto se il cappuccio e' corto
        if (z_tact < z_plate)
            translate([-tact_body[0] / 2 - 0.15, button_y - tact_body[1] / 2 - 0.15, z_tact])
                cube([tact_body[0] + 0.3, tact_body[1] + 0.3, 2]);
    }
}

// ---------------------------------------------------------------------------
// Coperchio
// ---------------------------------------------------------------------------
module lid() {
    difference() {
        union() {
            prism(z_lid, z_top) outline();
            // alette sopra il perno delle clip: lo tengono nella sede
            for (s = [-1, 1]) translate([s > 0 ? 15 : -21, hinge_y[0], z_lid]) cube([6, hinge_y[1] - hinge_y[0], lid_t]);
            // cornice che centra l'OLED sotto la finestra
            if (oled) translate([0, oled_y, z_lid - oled_ring]) difference() {
                w = oled_pcb[0] + oled_fit; l = oled_pcb[1] + oled_fit;
                translate([-w / 2 - 1.2, -l / 2 - 1.2, 0]) cube([w + 2.4, l + 2.4, oled_ring + eps]);
                translate([-w / 2, -l / 2, -1]) cube([w, l, oled_ring + 2]);
            }
            // bordo di centraggio
            difference() {
                prism(z_lid - lip_h, z_lid + eps) offset(delta = -wall - clearance) outline();
                prism(z_lid - lip_h - 1, z_lid + 1) offset(delta = -wall - clearance - lip_t) outline();
                post_positions() translate([0, 0, z_lid - lip_h - 1]) cylinder(d = post_d + 1, h = lip_h + 2);
                // intaglio per la flangia del cappuccio
                if (btn) translate([btn_x - btn_flange / 2 - 1.5, btn_y - btn_flange / 2 - 0.8, z_lid - lip_h - 1]) cube([btn_flange + 1.5, btn_flange + 1.6, lip_h + 1]);
                // intagli sopra USB-C e interruttore
                translate([-usb_pocket[0] / 2, -34, z_lid - lip_h - 1]) cube([usb_pocket[0], 5, lip_h + 1]);
                translate([14, (tp_y0 + tp_y1) / 2 - usb_pocket[0] / 2, z_lid - lip_h - 1]) cube([6, usb_pocket[0], lip_h + 1]);
                translate([14, sf_y - sf_plate[0] / 2 - 1, z_lid - lip_h - 1]) cube([6, sf_plate[0] + 2, lip_h + 1]);
            }
            // sulla USB-C della SuperMini e sulla coda
            z_usb = z_sm + sm_pcb + sm_usb[1] + 0.2;
            translate([-2.5, sm_y0 + 1, z_usb]) cube([5, 5, z_lid - z_usb + eps]);
            z_tail = z_sm + sm_pcb + sm_top_h - 1.6;
            translate([-3, sm_y1 - 3, z_tail]) cube([6, 2, z_lid - z_tail + eps]);
            // sulla USB-C del TP4056
            z_tpu = z_floor + tp_pcb + tp_usb[1] + 0.2;
            translate([tp_x1 - 6, (tp_y0 + tp_y1) / 2 - 2.5, z_tpu]) cube([5, 5, z_lid - z_tpu + eps]);
        }
        post_positions() translate([0, 0, z_lid - 1]) cylinder(d = m2_clear, h = lid_t + 2);
        if (oled) translate([-oled_win[0] / 2, oled_y + oled_win_dy - oled_win[1] / 2, z_lid - 1]) cube([oled_win[0], oled_win[1], lid_t + 2]);
        if (btn) translate([btn_x, btn_y, z_lid - 1]) cylinder(d = btn_hole, h = lid_t + 2);
        // zone sottili, scavate dall'interno
        for (a = led_zones) translate([a[0], a[2], z_lid]) cube([a[1] - a[0], a[3] - a[2], lid_t - led_t]);
    }
}

// ---------------------------------------------------------------------------
// Clip (destra; la sinistra e' la speculare)
// ---------------------------------------------------------------------------
module clip_arm_profile() {
    polygon([[16.6, -0.3], [29.9, -0.3], [29.9, 1.0], [24.5, 3.2], [22, 5], [20.9, 8], [21.0, 11.5],
             [19.8, 13.4], [18.7, 13.65], [17.3, 13.1], [16.6, 11.8]]);
}

// Profilo del dente tra i bracci: si aggancia sotto il bordo del launcher
module clip_catch_profile() {
    polygon([[17.7, -0.3], [29.9, -0.3], [29.9, 0.7], [20.9, 0.7], [20.1, 1.6], [19.2, 2.0], [18.5, 2.0], [18.0, 1.5], [17.7, 0.7]]);
}

module extrude_y(y0, y1) translate([0, y1, 0]) rotate([90, 0, 0]) linear_extrude(y1 - y0) children();

module clip() {
    y0 = clip_y - clip_gap / 2;
    y1 = clip_y + clip_gap / 2;
    for (s = [0, 1]) extrude_y(s == 0 ? y0 - clip_arm : y1, s == 0 ? y0 : y1 + clip_arm) clip_arm_profile();
    extrude_y(y0 - eps, y1 + eps) clip_catch_profile();
    // perno
    hull() for (z = [11.5, 11.9]) translate([pivot[0], y1 + eps, z]) rotate([90, 0, 0]) cylinder(d = 2.0, h = y1 - y0 + 2 * eps);
}

// Clip montata, ruotata attorno al perno (segno positivo: dente verso l'esterno)
module mounted_clip(angle = 0, mirrored = false) {
    mirror([mirrored ? 1 : 0, 0, 0])
        translate([pivot[0], 0, pivot[1]]) rotate([0, -angle, 0]) translate([-pivot[0], 0, -pivot[1]]) clip();
}

// Coricata su un braccio: gli strati seguono il braccio, che lavora a flessione
module clip_for_print() {
    rotate([-90, 0, 0]) translate([-16.6, -clip_y - clip_gap / 2 - clip_arm, 0.3]) clip();
}

// ---------------------------------------------------------------------------
// Componenti (blocchi, per controlli e immagini)
// ---------------------------------------------------------------------------
module blk(p, size, shr = 0) translate(p + [shr, shr, shr]) cube(size - 2 * [shr, shr, shr]);

module qre(shr = 0) {
    color("firebrick") blk([-qre_pcb[0] / 2, sensor_y - qre_pcb[1] / 2, z_qre_pcb], qre_pcb, shr);
    color("black") blk([-1.7, sensor_y - 1.35, sensor_face_z], [3.4, 2.7, qre_h + eps], shr);
}

module tact(shr = 0) translate([0, button_y, 0]) {
    color("#202020") blk([-tact_body[0] / 2, -tact_body[1] / 2, z_tact], tact_body, shr);
    color("silver") blk([-1.5, -1.5, button_tip_z], [3, 3, tact_cap_l + eps], shr);
    color("gold") for (s = [-1, 1]) blk([-tact_legs / 2, s * 2.25 - 0.35, z_tact], [tact_legs, 0.7, 0.4], shr);
}

module supermini(shr = 0) {
    color("#111") blk([-sm_w / 2, sm_y0, z_sm], [sm_w, sm_l, sm_pcb], shr);
    color("silver") blk([-sm_usb[0] / 2, sm_y0 - sm_usb_out, z_sm + sm_pcb], [sm_usb[0], 7.3, sm_usb[1]], shr);
    color("#333") blk([-sm_w / 2 + 1.5, sm_y1 - 12, z_sm + sm_pcb], [sm_w - 3, 10, 1.2], shr);
}

module tp4056(shr = 0) {
    color("royalblue") blk([tp_x0, tp_y0, z_floor], [tp_l, tp_w, tp_pcb], shr);
    color("silver") blk([tp_x1 + tp_usb_out - 7.3, (tp_y0 + tp_y1) / 2 - tp_usb[0] / 2, z_floor + tp_pcb], [7.3, tp_usb[0], tp_usb[1]], shr);
    color("#222") blk([tp_x0 + 8, tp_y0 + 6, z_floor + tp_pcb], [5, 5, 1.6], shr);
}

// Tasto esterno: stesso tasto della presenza, girato col cappuccio verso l'alto e i piedini lungo Y
module btn_tact(shr = 0) translate([btn_x, btn_y, 0]) {
    color("#202020") blk([-tact_body[0] / 2, -tact_body[1] / 2, z_btn_tact], tact_body, shr);
    color("silver") blk([-1.5, -1.5, z_btn_tact + tact_body[2] - eps], [3, 3, tact_cap_l + eps], shr);
    color("gold") for (s = [-1, 1]) blk([s * 2.25 - 0.35, -tact_legs / 2, z_btn_tact], [0.7, tact_legs, 0.4], shr);
}

// Cappuccio stampato: la flangia resta sotto il coperchio e appoggia sul tasto
module button(shr = 0) translate([btn_x, btn_y, z_btn]) color("dimgray") {
    cylinder(d = btn_flange - 2 * shr, h = 0.8);
    cylinder(d = btn_d - 2 * shr, h = z_top + btn_out - z_btn);
}

module oled_module(shr = 0) translate([0, oled_y, 0]) {
    color("navy") blk([-oled_pcb[0] / 2, -oled_pcb[1] / 2, z_oled], oled_pcb, shr);
    color("black") blk([-oled_glass[0] / 2, -oled_glass[1] / 2, z_oled + oled_pcb[2] - eps], oled_glass, shr);
}

module lipo_pack(shr = 0) color("lightsteelblue") blk([-lipo[0] / 2, lipo_y0 + fit / 2, z_floor + lipo_foam], lipo, shr);

module switch_sf(shr = 0) color("#303030") {
    ix = 19 - wall;
    blk([ix - 0.5, sf_y - sf_plate[0] / 2, sf_z - sf_plate[1] / 2], [0.5, sf_plate[0], sf_plate[1]], shr);
    blk([ix - sf_body[2], sf_y - sf_body[0] / 2, sf_z - sf_body[1] / 2], [sf_body[2], sf_body[0], sf_body[1]], shr);
    blk([ix - sf_body[2] - sf_pins, sf_y - 3.5, sf_z - 0.6], [sf_pins, 7, 1.2], shr);
    for (s = [-1, 1]) blk([ix - 2.1, sf_y + s * sf_pitch / 2 - 2.2, sf_z - 2.2], [1.6, 4.4, 4.4], shr);
}

module components(shr = 0) {
    qre(shr); tact(shr); supermini(shr); tp4056(shr); lipo_pack(shr); switch_sf(shr);
    if (oled) oled_module(shr);
    if (btn) { btn_tact(shr); button(shr); }
}

// Tiene solo la zona centrale, tra le colonnine del coperchio
module coupon() intersection() {
    children();
    translate([-40, coupon_y[0], -5]) cube([80, coupon_y[1] - coupon_y[0], 40]);
}

// ---------------------------------------------------------------------------
// Uscita
// ---------------------------------------------------------------------------
if (part == "body") translate([0, 0, -body_z]) body();
else if (part == "mounted_body") body();
else if (part == "tray") translate([0, 0, -band_z]) tray();
else if (part == "mounted_tray") tray();
else if (part == "lid") translate([0, 0, z_top]) rotate([180, 0, 0]) lid();
else if (part == "clips") { clip_for_print(); translate([0, 20, 0]) mirror([1, 0, 0]) clip_for_print(); }
else if (part == "button") translate([-btn_x, -btn_y, -z_btn]) button();
else if (part == "prova") {
    translate([0, 0, -body_z]) coupon() body();
    translate([45, 0, z_top]) rotate([180, 0, 0]) coupon() lid();
    translate([70, 0, 0]) { clip_for_print(); translate([16, 20, 0]) mirror([1, 0, 0]) clip_for_print(); }
    translate([80, -40, -band_z]) tray();
    if (btn) translate([55, -40, 0]) translate([-btn_x, -btn_y, -z_btn]) button();
}
else if (part == "plate") {
    translate([0, 0, -body_z]) body();
    translate([45, 0, z_top]) rotate([180, 0, 0]) lid();
    translate([70, 0, 0]) { clip_for_print(); translate([16, 20, 0]) mirror([1, 0, 0]) clip_for_print(); }
    translate([80, -40, -band_z]) tray();
    if (btn) translate([80, -50, 0]) translate([-btn_x, -btn_y, -z_btn]) button();  // fuori dall'ingombro del coperchio
}
else if (part == "assembly") {
    color("orange") body();
    color("gold") tray();
    components();
    mounted_clip(0, false);
    mounted_clip(0, true);
    color("lightblue", 0.5) lid();
}
else if (part == "mounted_lid") lid();
else if (part == "mounted_clip") mounted_clip(clip_angle, false);
else if (part == "check_qre") qre(0.1);
else if (part == "check_tact") tact(0.1);
else if (part == "check_sm") supermini(0.1);
else if (part == "check_tp") tp4056(0.1);
else if (part == "check_lipo") lipo_pack(0.1);
else if (part == "check_sw") switch_sf(0.1);
else if (part == "check_oled") oled_module(0.1);
else if (part == "check_btn") btn_tact(0.1);
else if (part == "check_cap") button(0.1);
