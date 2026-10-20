// SpinLite - scatolina parametrica per l'elettronica (OpenSCAD, unita in mm).
// Licenza MIT, come il resto del progetto.
//
// Contiene: ESP32-C3 SuperMini, modulo TP4056, LiPo piccola, interruttore a slitta.
// Disposizione: la LiPo sta sul fondo; sopra, affiancate, SuperMini (a sinistra,
// con la USB-C sul lato corto sinistro) e TP4056 (a destra, USB-C sul lato corto
// destro). Interruttore sul lato anteriore, cavi del sensore sul lato posteriore.
// Pensata per stampa FDM SENZA supporti: scatola con l'apertura in alto,
// coperchio stampato a faccia in giu (con il bordo di incastro verso l'alto).

// ---- Cosa generare: "box", "lid" oppure "both" ----
part = "both";

// ---- Spessori e tolleranze ----
wall      = 1.6;  // spessore pareti
floor_t   = 1.6;  // spessore fondo
lid_t     = 1.6;  // spessore piastra del coperchio
lip_t     = 1.2;  // spessore del bordo di incastro del coperchio
lip_h     = 2.5;  // altezza del bordo di incastro
clearance = 0.2;  // gioco tra coperchio e scatola (regolalo sulla tua stampante)
fit       = 0.6;  // gioco totale aggiunto alle dimensioni dei componenti
gap       = 1.5;  // spazio tra SuperMini e TP4056

// ---- ESP32-C3 SuperMini (USB-C su un lato corto) ----
sm_l      = 22.5; // lunghezza (lungo X)
sm_w      = 18;   // larghezza
sm_h      = 3.5;  // altezza totale (PCB + componenti)
sm_usb_w  = 9.5;  // larghezza apertura USB-C
sm_usb_h  = 4.0;  // altezza apertura USB-C (parte dal piano del PCB)

// ---- TP4056 con protezione (USB-C su un lato corto) ----
tp_l      = 28;
tp_w      = 17;
tp_h      = 4;
tp_usb_w  = 9.5;
tp_usb_h  = 4.0;

// ---- LiPo (default: 300 mAh) ----
lipo_l    = 30;
lipo_w    = 20;
lipo_h    = 6;
foam      = 1;    // spazio per nastro biadesivo o gommapiuma sopra la LiPo

// ---- Interruttore a slitta (foro rettangolare sul lato anteriore) ----
sw_w      = 8.6;  // larghezza del foro (lungo X)
sw_h      = 4.2;  // altezza del foro
sw_x      = 0;    // posizione X del centro, relativa al centro scatola

// ---- Cavi verso il sensore (lato posteriore) ----
cable_w   = 6;    // larghezza del foro
cable_h   = 3.5;  // altezza del foro
cable_x   = -12;  // posizione X del centro, relativa al centro scatola

// ---- Finestrella OLED 128x32 (modulo ~38x12, area visibile ~23x6) ----
oled      = false; // true per aprire la finestrella e alzare la scatola
oled_win_w = 23.5;
oled_win_h = 6.5;
oled_extra_h = 3;  // altezza in piu per il modulo sotto il coperchio
oled_x    = -10;   // posizione X della finestrella, relativa al centro scatola
oled_y    = 0;     // posizione Y, relativa al centro scatola

// ---- Fissaggio al supporto del sensore ----
ear_w     = 9;    // larghezza delle linguette
ear_l     = 7;    // sporgenza delle linguette
ear_t     = 2.4;  // spessore delle linguette
screw_d   = 2.4;  // foro per vite M2
ear_slot  = 3.5;  // lunghezza asola per fascetta (0 = solo foro tondo)

$fn = 40;

// Sovrapposizione tra pezzi da unire. Due solidi che si toccano solo su una
// faccia possono restare separati per un errore di arrotondamento (es. 55,8
// calcolato come 55,79999): la linguetta o il bordo si staccano in stampa.
eps = 0.05;

// ---------------------------------------------------------------------------
// Dimensioni derivate
// ---------------------------------------------------------------------------
inner_x = max(sm_l + gap + tp_l, lipo_l) + fit;
inner_y = max(sm_w, tp_w, lipo_w) + fit;
inner_z = lipo_h + foam + max(sm_h, tp_h) + 1 + (oled ? oled_extra_h : 0);
outer_x = inner_x + 2 * wall;
outer_y = inner_y + 2 * wall;
box_h   = floor_t + inner_z;
z_board = floor_t + lipo_h + foam; // quota del piano delle schede

// Ritaglio rettangolare centrato in X e in Z; lo spessore e size[1] (lungo Y)
module cutout(size) {
    translate([-size[0] / 2, 0, -size[2] / 2]) cube(size);
}

// ---------------------------------------------------------------------------
// Scatola
// ---------------------------------------------------------------------------
module ear(side) { // side = -1 (sinistra) o +1 (destra)
    // la linguetta entra di eps nella parete, cosi' l'unione e' un pezzo solo
    x0 = side < 0 ? -ear_l : outer_x - eps;
    translate([x0, outer_y / 2 - ear_w / 2, 0]) difference() {
        cube([ear_l + eps, ear_w, ear_t]);
        // foro o asola per vite M2 / fascetta
        translate([ear_l / 2, ear_w / 2, -0.1]) hull() {
            translate([0, -ear_slot / 2, 0]) cylinder(d = screw_d, h = ear_t + 0.2);
            translate([0,  ear_slot / 2, 0]) cylinder(d = screw_d, h = ear_t + 0.2);
        }
    }
}

module box() {
    difference() {
        union() {
            cube([outer_x, outer_y, box_h]);
            ear(-1);
            ear(1);
        }
        // cavita interna
        translate([wall, wall, floor_t]) cube([inner_x, inner_y, inner_z + 0.01]);

        // USB-C della SuperMini: parete sinistra
        translate([-0.1, outer_y / 2 - sm_usb_w / 2, z_board - 0.5])
            cube([wall + 0.2, sm_usb_w, sm_usb_h + 1.2]);
        // USB-C del TP4056: parete destra
        translate([outer_x - wall - 0.1, outer_y / 2 - tp_usb_w / 2, z_board - 0.5])
            cube([wall + 0.2, tp_usb_w, tp_usb_h + 1.2]);

        // interruttore: parete anteriore (y = 0)
        translate([outer_x / 2 + sw_x, -0.1, floor_t + inner_z / 2])
            cutout([sw_w, wall + 0.2, sw_h]);

        // cavi sensore: parete posteriore (y = outer_y)
        translate([outer_x / 2 + cable_x, outer_y - wall - 0.1, z_board + 1])
            cutout([cable_w, wall + 0.2, cable_h]);
    }
}

// ---------------------------------------------------------------------------
// Coperchio: piastra + bordo che entra nella scatola con gioco `clearance`.
// Stampato con la piastra sul piatto e il bordo verso l'alto.
// ---------------------------------------------------------------------------
module lid() {
    difference() {
        union() {
            cube([outer_x, outer_y, lid_t]);
            // bordo di incastro: anello che sta dentro le pareti
            // (parte eps dentro la piastra: deve fondersi con lei)
            translate([wall + clearance, wall + clearance, lid_t - eps]) difference() {
                cube([inner_x - 2 * clearance, inner_y - 2 * clearance, lip_h + eps]);
                translate([lip_t, lip_t, -0.1])
                    cube([inner_x - 2 * clearance - 2 * lip_t,
                          inner_y - 2 * clearance - 2 * lip_t, lip_h + eps + 0.2]);
            }
        }
        if (oled) {
            translate([outer_x / 2 + oled_x - oled_win_w / 2,
                       outer_y / 2 + oled_y - oled_win_h / 2, -0.1])
                cube([oled_win_w, oled_win_h, lid_t + 0.2]);
        }
    }
}

// ---------------------------------------------------------------------------
// Uscita
// ---------------------------------------------------------------------------
if (part == "box") {
    box();
} else if (part == "lid") {
    lid();
} else {
    box();
    translate([0, outer_y + 6, 0]) lid();
}
