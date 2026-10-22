// SpinLite - scatola da avvitare SOPRA la base QRE1113 v102 di LaunchLab.
// OpenSCAD, millimetri. Licenza MIT.
//
// La base (CC BY-SA, Migbello / LaunchLab) NON e' in questo repo: si scarica e
// si stampa a parte (vedi hardware/README.md). Da lei prendiamo solo le quote
// d'interfaccia, misurate sui suoi STL:
//   - 4 fori pilota per M2 (diametro 1,7) su un rettangolo 29 x 24 mm, centri
//     a (+-14,5; +-12); i rilievi finiscono a Z 11,2;
//   - blocchi delle cerniere delle clip: X da +-10,2 a +-18,5, Y +-3,4, fino a
//     Z 13,28 (sono l'unica cosa sopra Z 11,2 oltre ai rilievi);
//   - spine Dupont a 90 gradi del QRE1113: X -3,6..3,8, Y -28,5..-14,1,
//     Z 7,1..9,9; i fili escono verso -Y;
//   - il dock M5 di LaunchLab occupa X +-17,3, Y -28,5..36,5 da Z 11,2: lo
//     usiamo come ingombro di riferimento (verificato da loro a CAD contro il
//     movimento delle clip). Usciamo di 4,5 mm verso -Y: i fili delle spine
//     escono orizzontali a Y -28,5 e devono avere spazio per curvare in su.
//
// Sistema di riferimento: quello degli STL di montaggio di LaunchLab. Origine
// al centro della base, Z verso l'alto, -Y lato connettore del sensore.
//
// Disposizione (due piani):
//   sotto: LiPo (verso -Y) e TP4056 (verso +Y, USB-C nella parete +Y);
//   sopra: SuperMini su due mensole della parete +Y e una traversa, USB-C
//          nella stessa parete, sopra quella del TP4056.
// Montaggio:
//   1. scatola vuota avvitata alla base con 4 x M2x6 (teste nel fondo);
//   2. TP4056 appoggiato al centro e fatto scorrere verso +Y sotto la
//      traversa, finche' la USB-C entra nel suo foro;
//   3. LiPo nel vano verso -Y (un velo di biadesivo o gommapiuma sotto);
//   4. fili del sensore su dall'apertura nel fondo; interruttore: SS12F15
//      avvitato da fuori alla parete +X (2 x M2 + dado), SS12D00 nella culla;
//   5. SuperMini: USB-C nel foro, poi si abbassa la coda sulla traversa;
//   6. coperchio con 4 x M2x6: una nervatura sulla USB-C e un limitatore
//      sulla coda tengono ferma la SuperMini.

// ---- Cosa generare: "box", "lid", "both" (affiancati), "assembly" ----
// ("components" e "lid_mounted" servono solo per i controlli)
part = "both";

// ---- Base LaunchLab, solo per controllo visivo (non stampata) ----
// Percorso di 01_QRE_open_holder_body.stl dalla cartella preview/ del pacchetto
// top-dock-v098 di LaunchLab (coordinate di montaggio). Vuoto = non mostrata.
base_stl = "";

// ---- Interfaccia con la base (misurata: non cambiare senza ricontrollare) ----
pilot_x   = 14.5;  // fori pilota a (+-pilot_x; +-pilot_y)
pilot_y   = 12;
seat_z    = 11.2;  // cima dei rilievi dei fori
hinge_x0  = 10.2;  // blocchi cerniere: |x| da hinge_x0 a hinge_x1
hinge_x1  = 18.5;
hinge_y   = 3.4;   // |y| <= hinge_y
hinge_top = 13.28; // quota massima delle cerniere
plug_x0   = -3.6;  // spine Dupont del sensore
plug_x1   = 3.8;
plug_y_end = -28.5; // da qui escono i fili

// ---- Ingombro della scatola ----
half_x    = 17.3;   // come il dock LaunchLab
y_min     = -33;    // 4,5 mm oltre il dock: spazio per la curva dei fili
y_max     = 36.5;
wall      = 1.6;
floor_t   = 1.6;
gap_hinge = 0.3;    // aria sopra le cerniere
eps       = 0.05;   // sovrapposizione tra solidi da unire

// ---- Viti ----
m2_pilot  = 1.7;   // foro per vite M2 autofilettante nella plastica
m2_clear  = 2.3;   // foro passante
m2_head   = 4.2;   // sede della testa (pan head ~3,8), solo dentro il fondo
m2_driver = 2.6;   // canale per il cacciavite sopra il fondo
screw_seat_z = 13.4; // piano d'appoggio della testa: M2x6 prende ~3,8 mm come nel dock
post_d    = 5.6;   // colonnine sopra i rilievi

// ---- Componenti (misura i tuoi e correggi) ----
fit       = 0.4;   // gioco totale sulle schede
// ESP32-C3 SuperMini
sm_l      = 22.5;  // lungo Y
sm_w      = 18;    // lungo X
sm_pcb    = 1.0;
sm_usb_w  = 9.0;   // ricettacolo USB-C
sm_usb_h  = 3.3;
sm_usb_out = 0.6;  // quanto la USB-C sporge oltre il bordo della scheda
sm_boot   = [-3.5, 4.5]; // pulsante BOOT: x dal centro, y dal bordo USB
sm_top_h  = 3.6;   // altezza dei componenti sopra il PCB (USB-C, pulsanti)
// TP4056 USB-C con protezione
tp_l      = 28;
tp_w      = 17.2;
tp_pcb    = 1.6;
tp_usb_w  = 9.0;
tp_usb_h  = 3.3;
tp_usb_out = 0.6;
// LiPo (default 502030: 5 x 20 x 30 mm, circa 250 mAh)
lipo_l    = 30;    // lungo Y
lipo_w    = 20;
lipo_h    = 5;
// Interruttore a slitta SS12D00 (corpo sdraiato, leva verso +X)
sw_body   = [3.7, 8.7, 3.6]; // x, y, z
sw_slot   = [5.0, 2.4];      // feritoia per la leva: lunghezza (y), altezza (z)
sw_y      = -12;             // centro dell'interruttore lungo Y

// Tipo di interruttore: "SS12F15" (flangia con 2 fori, avvitato alla parete +X
// sopra la LiPo) oppure "SS12D00" (piccolo, nella culla sul fondo; sw_* sopra)
switch_type = "SS12F15";
// SS12F15: misure dal disegno del venditore; corsa e fori stimati, da misurare
sf_plate  = [19.5, 6.1];      // flangia: lunghezza (y), altezza (z)
sf_body   = [10.5, 6.1, 5.3]; // corpo: y, z, profondita' dalla parete (x)
sf_pins   = 1.5;              // piedini accorciati a questa lunghezza
sf_lever  = [3.0, 2.5, 4.0];  // leva: y, z, sporgenza dalla flangia
sf_travel = 3.5;              // corsa della leva (stima)
sf_hole_pitch = 15;           // distanza tra i fori della flangia (stima)
sf_hole_d = 2.3;              // foro passante per vite M2
sf_y      = -9;               // centro lungo Y
// Fori USB-C: ricettacolo + incavo esterno per la spina
usb_hole_w = 9.8;
usb_hole_h = 3.9;
usb_pocket = [13, 0.8];      // larghezza dell'incavo esterno, profondita'
// OLED 128x32 opzionale nel coperchio
oled      = false;
oled_pcb  = [12.2, 38.2];    // x, y (modulo ~38 x 12)
oled_win  = [6.5, 23];       // area visibile, x, y
oled_y    = -9;              // centro lungo Y (sopra la LiPo)
oled_h    = 4;               // altezza in piu' della scatola

$fn = 40;

// ---------------------------------------------------------------------------
// Quote derivate
// ---------------------------------------------------------------------------
z_bot     = hinge_top + gap_hinge;        // fondo della scatola (sotto)
z_floor   = z_bot + floor_t;              // piano interno
in_x      = half_x - wall;                // interno: |x| <= in_x
in_y0     = y_min + wall;
in_y1     = y_max - wall;
// piano inferiore
bar_t     = 2;                            // spessore della traversa
// il TP4056 scorre sotto la traversa: serve aria sopra la sua USB-C
low_h     = max(lipo_h + 1, tp_pcb + tp_usb_h + 0.6 + bar_t);
z_shelf   = z_floor + low_h;              // appoggio della SuperMini
z_in_top  = z_shelf + sm_pcb + sm_top_h + 1.5 + (oled ? oled_h : 0);
lid_t     = 1.6;
lip_h     = 2;
lip_t     = 1.2;
clearance = 0.2;
// posizioni lungo Y
// bordi delle schede lungo Y: il PCB si ferma 0,2 mm prima della parete,
// solo la USB-C sporge dentro il foro
tp_y1     = in_y1 - 0.2;                  // bordo anteriore del TP4056
tp_y0     = tp_y1 - tp_l;                 // bordo posteriore
sm_y1     = in_y1 - 0.2;
sm_y0     = sm_y1 - sm_l;
lipo_y1   = tp_y0 - fit / 2 - 0.5;
lipo_y0   = lipo_y1 - lipo_l - fit;
cable_y0  = in_y0;                        // apertura per i fili del sensore
cable_y1  = lipo_y0 - 0.3;
bar_y0    = sm_y0 - fit / 2 - 0.8;        // traversa sotto la coda della SuperMini
bar_y1    = sm_y0 + 2.5;
z_tp_usb  = z_floor + tp_pcb + tp_usb_h / 2;
z_sm_usb  = z_shelf + sm_pcb + sm_usb_h / 2;
// colonnine del coperchio (angoli interni)
lid_post  = [in_x - 2.6, in_y1 - 2.6, in_y0 + 2.6];
// SS12F15: flangia appena sopra la LiPo (0,3 mm d'aria)
sf_z      = z_floor + 0.5 + lipo_h + 0.3 + sf_plate[1] / 2;
sf_slot   = [sf_lever[0] + sf_travel + 0.6, sf_lever[1] + 0.6]; // feritoia: y, z

assert(lipo_y0 > in_y0 + 3, "LiPo troppo lunga: non resta spazio per i fili del sensore");
assert(switch_type != "SS12F15" || sf_z + sf_plate[1] / 2 < z_in_top - 0.3, "SS12F15 non sta tra LiPo e coperchio");
assert(z_tp_usb + usb_hole_h / 2 + 1.2 < z_sm_usb - usb_hole_h / 2 + 0.01, "fori USB-C sovrapposti");

// ---------------------------------------------------------------------------
// Scatola
// ---------------------------------------------------------------------------
module rounded_box(x0, x1, y0, y1, z0, z1, r = 1.5) {
    hull() for (x = [x0 + r, x1 - r], y = [y0 + r, y1 - r])
        translate([x, y, z0]) cylinder(r = r, h = z1 - z0);
}

module shell() {
    difference() {
        rounded_box(-half_x, half_x, y_min, y_max, z_bot, z_in_top);
        translate([-in_x, in_y0, z_floor]) cube([2 * in_x, in_y1 - in_y0, z_in_top]);
    }
}

// Colonnine che scendono sui rilievi della base
module posts() {
    for (sx = [-1, 1], sy = [-1, 1])
        translate([sx * pilot_x, sy * pilot_y, seat_z])
            cylinder(d = post_d, h = z_floor - seat_z);
}

module post_holes() {
    for (sx = [-1, 1], sy = [-1, 1]) translate([sx * pilot_x, sy * pilot_y, 0]) {
        translate([0, 0, seat_z - 1]) cylinder(d = m2_clear, h = 30);
        // sede della testa: solo nello spessore del fondo, cosi' le pareti
        // restano piene; sopra passa solo il cacciavite
        translate([0, 0, screw_seat_z]) cylinder(d = m2_head, h = z_floor - screw_seat_z + 0.01);
        translate([0, 0, screw_seat_z]) cylinder(d = m2_driver, h = 30);
    }
}

// Mensole, traversa e guide
module supports() {
    // guide basse ai lati del TP4056
    for (s = [-1, 1])
        translate([s * (tp_w / 2 + fit / 2) + (s < 0 ? -1.2 : 0), tp_y0, z_floor - eps])
            cube([1.2, tp_l, 1.5]);
    // guide basse ai lati della LiPo
    for (s = [-1, 1])
        translate([s * (lipo_w / 2 + fit / 2) + (s < 0 ? -1.2 : 0), lipo_y0 + 4, z_floor - eps])
            cube([1.2, lipo_l - 8, 2]);
    // traversa: ponte da parete a parete, sotto la coda della SuperMini
    translate([-in_x - eps, bar_y0, z_shelf - bar_t]) cube([2 * in_x + 2 * eps, bar_y1 - bar_y0, bar_t]);
    // battuta dietro la SuperMini (non scivola verso -Y)
    translate([-sm_w / 2, bar_y0, z_shelf - eps]) cube([sm_w, 0.8, sm_pcb + 0.6]);
    // battute laterali sulla traversa
    for (s = [-1, 1])
        translate([s * (sm_w / 2 + fit / 2) + (s < 0 ? -1 : 0), bar_y0, z_shelf - eps])
            cube([1, bar_y1 - bar_y0, sm_pcb + 0.6]);
    // mensole a sbalzo sulla parete +Y (con raccordo a 45 gradi: niente supporti)
    for (s = [-1, 1]) {
        x0 = s > 0 ? sm_usb_w / 2 + 0.8 : -sm_w / 2 + 0.3;
        w  = sm_w / 2 - 0.3 - sm_usb_w / 2 - 0.8;
        // rotate([90,0,90]): x del poligono -> +Y, y -> Z, estrusione lungo X
        translate([x0, in_y1 + eps, z_shelf]) rotate([90, 0, 90])
            linear_extrude(w) polygon([[0, 0], [-2.5 - eps, 0], [0, -2.5]]);
    }
    // colonnine per le viti del coperchio
    for (sx = [-1, 1], y = [lid_post[1], lid_post[2]])
        translate([sx * lid_post[0], y, z_floor - eps]) cylinder(d = 5, h = z_in_top - z_floor + eps);
    // culla dell'interruttore SS12D00 contro la parete +X
    if (switch_type == "SS12D00")
    translate([in_x - sw_body[0] - 1.2, sw_y - sw_body[1] / 2 - 1.2, z_floor - eps])
        difference() {
            cube([sw_body[0] + 1.2 + eps, sw_body[1] + 2.4, 2.5]);
            translate([1.2, 1.2, -1]) cube([sw_body[0] + 1, sw_body[1], 5]);
        }
}

module cutouts() {
    // fili del sensore: dal fondo, sopra l'uscita delle spine
    translate([plug_x0 - 1.5, cable_y0, z_bot - 1]) cube([plug_x1 - plug_x0 + 3, cable_y1 - cable_y0, 5]);
    // USB-C del TP4056 e della SuperMini nella parete +Y
    for (z = [z_tp_usb, z_sm_usb])
        translate([-usb_hole_w / 2, in_y1 - 1, z - usb_hole_h / 2]) cube([usb_hole_w, wall + 2, usb_hole_h]);
    // incavo esterno: la spina arriva in battuta sul ricettacolo
    translate([-usb_pocket[0] / 2, y_max - usb_pocket[1], z_tp_usb - usb_hole_h / 2 - 1.2])
        cube([usb_pocket[0], usb_pocket[1] + 1, z_sm_usb - z_tp_usb + usb_hole_h + 2.4]);
    // feritoia della leva dell'interruttore, parete +X
    if (switch_type == "SS12D00") {
        translate([in_x - 1, sw_y - sw_slot[0] / 2, z_floor + sw_body[2] / 2 - sw_slot[1] / 2])
            cube([wall + 2, sw_slot[0], sw_slot[1]]);
    } else {
        translate([in_x - 1, sf_y - sf_slot[0] / 2, sf_z - sf_slot[1] / 2]) cube([wall + 2, sf_slot[0], sf_slot[1]]);
        // fori delle due viti M2 (da fuori, dado dentro)
        for (s = [-1, 1]) translate([in_x - 1, sf_y + s * sf_hole_pitch / 2, sf_z]) rotate([0, 90, 0])
            cylinder(d = sf_hole_d, h = wall + 2);
    }
    // fori pilota del coperchio
    for (sx = [-1, 1], y = [lid_post[1], lid_post[2]])
        translate([sx * lid_post[0], y, z_in_top - 7]) cylinder(d = m2_pilot, h = 8);
}

module box() {
    difference() {
        union() {
            shell();
            posts();
            supports();
        }
        cutouts();
        post_holes();
    }
}

// ---------------------------------------------------------------------------
// Coperchio (stampato capovolto: piastra sul piatto)
// ---------------------------------------------------------------------------
module lid() {
    difference() {
        union() {
            rounded_box(-half_x, half_x, y_min, y_max, z_in_top, z_in_top + lid_t);
            // bordo di centraggio dentro le pareti
            difference() {
                translate([-in_x + clearance, in_y0 + clearance, z_in_top - lip_h])
                    cube([2 * (in_x - clearance), in_y1 - in_y0 - 2 * clearance, lip_h + eps]);
                translate([-in_x + clearance + lip_t, in_y0 + clearance + lip_t, z_in_top - lip_h - 1])
                    cube([2 * (in_x - clearance - lip_t), in_y1 - in_y0 - 2 * (clearance + lip_t), lip_h + 2]);
                // intaglio sopra la USB-C della SuperMini, vicino alla parete +Y
                translate([-usb_hole_w / 2, in_y1 - lip_t - 2, z_in_top - lip_h - 1])
                    cube([usb_hole_w, lip_t + 3, lip_h + 1]);
                // intaglio sopra l'interruttore SS12F15 (la flangia arriva vicino al coperchio)
                if (switch_type == "SS12F15")
                    translate([in_x - lip_t - 2, sf_y - sf_plate[0] / 2 - 1, z_in_top - lip_h - 1])
                        cube([lip_t + 3, sf_plate[0] + 2, lip_h + 1]);
                // il bordo non tocca le colonnine
                for (sx = [-1, 1], y = [lid_post[1], lid_post[2]])
                    translate([sx * lid_post[0], y, z_in_top - lip_h - 1]) cylinder(d = 6, h = lip_h + 2);
            }
            // SuperMini: la USB-C nel foro blocca la parte anteriore. Una
            // nervatura preme sul guscio della USB-C (robusto); sopra la coda
            // un limitatore lascia 1,6 mm d'aria (antenna e chip) ma
            // impedisce alla scheda di sfilarsi. I bordi lunghi restano
            // liberi: li' ci sono le piazzole saldate.
            z_usb = z_shelf + sm_pcb + sm_usb_h + 0.2;
            translate([-1.5, sm_y1 - 5.5, z_usb]) cube([5, 5, z_in_top - z_usb + eps]);
            z_tail = z_shelf + sm_pcb + 1.6;
            translate([-3, sm_y0 + 0.5, z_tail]) cube([6, 2, z_in_top - z_tail + eps]);
        }
        // viti del coperchio
        for (sx = [-1, 1], y = [lid_post[1], lid_post[2]])
            translate([sx * lid_post[0], y, z_in_top - 1]) cylinder(d = m2_clear, h = lid_t + 2);
        // foro sopra il pulsante BOOT (si preme con uno stuzzicadenti)
        translate([sm_boot[0], sm_y1 - sm_boot[1], z_in_top - 1]) cylinder(d = 3.2, h = lid_t + 2);
        if (oled) {
            translate([-oled_win[0] / 2, oled_y - oled_win[1] / 2, z_in_top - 1])
                cube([oled_win[0], oled_win[1], lid_t + 2]);
        }
    }
    if (oled) {
        // cornice che centra il modulo OLED sotto la finestra
        translate([0, oled_y, z_in_top - 1.5]) difference() {
            translate([-oled_pcb[0] / 2 - 1.2, -oled_pcb[1] / 2 - 1.2, 0])
                cube([oled_pcb[0] + 2.4, oled_pcb[1] + 2.4, 1.5 + eps]);
            translate([-oled_pcb[0] / 2, -oled_pcb[1] / 2, -1])
                cube([oled_pcb[0], oled_pcb[1], 3]);
        }
    }
}

// ---------------------------------------------------------------------------
// Uscita
// ---------------------------------------------------------------------------
module base_ghost() {
    if (base_stl != "") %import(base_stl);
}

// Componenti come blocchi semplici, per controllare ingombri e montaggio.
// Le misure sono quelle dei parametri: correggile con le tue.
// Blocco [x0,y0,z0] + size, ristretto di `shr` per lato (per i controlli).
module blk(p, size, shr = 0) {
    translate(p + [shr, shr, shr]) cube(size - 2 * [shr, shr, shr]);
}

module components(which = "all", shr = 0) {
    // TP4056: PCB + USB-C sul bordo +Y
    if (which == "all" || which == "tp") color("blue") {
        blk([-tp_w / 2, tp_y0, z_floor], [tp_w, tp_l, tp_pcb], shr);
        blk([-tp_usb_w / 2, tp_y1 + tp_usb_out - 7.3, z_floor + tp_pcb], [tp_usb_w, 7.3, tp_usb_h], shr);
    }
    // LiPo (su 0,5 mm di gommapiuma)
    if (which == "all" || which == "lipo")
        color("silver") blk([-lipo_w / 2, lipo_y0 + fit / 2, z_floor + 0.5], [lipo_w, lipo_l, lipo_h], shr);
    // SuperMini: PCB, USB-C, componenti
    if (which == "all" || which == "sm") color("black") {
        blk([-sm_w / 2, sm_y0, z_shelf], [sm_w, sm_l, sm_pcb], shr);
        blk([-sm_usb_w / 2, sm_y1 + sm_usb_out - 7.3, z_shelf + sm_pcb], [sm_usb_w, 7.3, sm_usb_h], shr);
        blk([-sm_w / 2 + 1.5, sm_y0 + 2, z_shelf + sm_pcb], [sm_w - 3, sm_l - 10, 1.2], shr);
    }
    // interruttore
    if ((which == "all" || which == "sw") && switch_type == "SS12D00")
        color("green") blk([in_x - sw_body[0], sw_y - sw_body[1] / 2, z_floor], sw_body, shr);
    if ((which == "all" || which == "sw") && switch_type == "SS12F15") color("green") {
        blk([in_x - 0.5, sf_y - sf_plate[0] / 2, sf_z - sf_plate[1] / 2], [0.5, sf_plate[0], sf_plate[1]], shr);
        blk([in_x - sf_body[2], sf_y - sf_body[0] / 2, sf_z - sf_body[1] / 2], [sf_body[2], sf_body[0], sf_body[1]], shr);
        blk([in_x - sf_body[2] - sf_pins, sf_y - 3.5, sf_z - 0.6], [sf_pins, 7, 1.2], shr);
        for (s = [-1, 1]) // dadi M2 dentro
            blk([in_x - 0.5 - 1.6, sf_y + s * sf_hole_pitch / 2 - 2.2, sf_z - 2.2], [1.6, 4.4, 4.4], shr);
    }
}

if (part == "box") {
    box();
} else if (part == "lid") {
    // capovolto per la stampa
    translate([0, 0, z_in_top + lid_t]) rotate([180, 0, 0]) lid();
} else if (part == "assembly") {
    base_ghost();
    components();
    box();
    color("lightblue", 0.6) lid();
} else if (part == "components") {
    components();
} else if (part == "check_tp" || part == "check_lipo" || part == "check_sm" || part == "check_sw") {
    components(which = part == "check_tp" ? "tp" : part == "check_lipo" ? "lipo" : part == "check_sm" ? "sm" : "sw", shr = 0.1);
} else if (part == "none") {
    // niente: il file e' incluso da spinlite_views.scad
} else if (part == "lid_mounted") {
    lid();
} else {
    box();
    translate([2 * half_x + 8, 0, z_in_top + lid_t]) rotate([180, 0, 0]) lid();
}
