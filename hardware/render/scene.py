# SpinLite - scena Blender (Cycles) con il dispositivo montato su un launcher.
# Si lancia da render.py:
#   blender -b --python scene.py -- <A|B> <build> <launchlab> <png> <vista> <campioni>
#
# A = scatola SpinLite sopra la base QRE1113 di LaunchLab (configurazione di riferimento)
# B = supporto laterale TCRT5000 di LaunchLab + scatola laterale fissata con fascette
#
# Il launcher e la trottola sono modellati a mano sulle foto del launcher a corda
# BX-28 (forma e proporzioni, non quote misurate). I pezzi LaunchLab sono CC BY-SA
# (Migbello / LaunchLab) e vengono letti da hardware/third_party/launchlab/.
import bpy, sys, math, os
from mathutils import Matrix, Vector

argv = sys.argv[sys.argv.index("--") + 1:]
OPT, BUILD, LL, OUT, VIEW = argv[:5]
OUT = os.path.abspath(OUT)  # Blender interpreta i percorsi relativi a modo suo
SAMPLES = int(argv[5]) if len(argv) > 5 else 160
MM = 0.001

bpy.ops.wm.read_factory_settings(use_empty=True)
sc = bpy.context.scene

# ---------------------------------------------------------------- materiali
def mat(name, color, rough=0.5, metal=0.0, coat=0.0, layers=False):
    m = bpy.data.materials.new(name); m.use_nodes = True
    nt = m.node_tree; b = nt.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*color, 1)
    b.inputs["Roughness"].default_value = rough
    b.inputs["Metallic"].default_value = metal
    b.inputs["Coat Weight"].default_value = coat
    if layers:  # righe dei layer FDM da 0,2 mm lungo Z dell'oggetto
        tc = nt.nodes.new("ShaderNodeTexCoord"); sep = nt.nodes.new("ShaderNodeSeparateXYZ")
        mul = nt.nodes.new("ShaderNodeMath"); mul.operation = "MULTIPLY"; mul.inputs[1].default_value = 2 * math.pi / 0.0002
        sn = nt.nodes.new("ShaderNodeMath"); sn.operation = "SINE"
        bump = nt.nodes.new("ShaderNodeBump"); bump.inputs["Strength"].default_value = 0.15; bump.inputs["Distance"].default_value = 0.00004
        nt.links.new(tc.outputs["Object"], sep.inputs[0]); nt.links.new(sep.outputs["Z"], mul.inputs[0])
        nt.links.new(mul.outputs[0], sn.inputs[0]); nt.links.new(sn.outputs[0], bump.inputs["Height"])
        nt.links.new(bump.outputs["Normal"], b.inputs["Normal"])
    return m

M = {
    "pla": mat("PLA arancione", (0.85, 0.11, 0.0), 0.45, layers=True),
    "petg": mat("PETG grigio", (0.30, 0.32, 0.35), 0.35, layers=True),
    "white": mat("ABS bianco", (0.80, 0.81, 0.82), 0.32, coat=0.3),
    "band": mat("Fascia nera", (0.012, 0.012, 0.014), 0.28, coat=0.4),
    "black": mat("Plastica nera", (0.015, 0.015, 0.015), 0.5),
    "metal": mat("Metallo", (0.80, 0.80, 0.82), 0.22, metal=1.0),
    "screw": mat("Viti", (0.30, 0.30, 0.32), 0.35, metal=1.0),
    "pcb_red": mat("PCB rosso", (0.50, 0.03, 0.02), 0.45),
    "pcb_blue": mat("PCB blu", (0.02, 0.09, 0.40), 0.45),
    "bey_metal": mat("Lama", (0.85, 0.86, 0.9), 0.18, metal=1.0),
    "bey_color": mat("Bey colore", (0.08, 0.22, 0.75), 0.3, coat=0.5),
    "w_red": mat("Filo rosso", (0.7, 0.02, 0.02), 0.5),
    "w_black": mat("Filo nero", (0.02, 0.02, 0.02), 0.5),
    "w_yellow": mat("Filo giallo", (0.85, 0.62, 0.02), 0.5),
    "string": mat("Corda", (0.05, 0.05, 0.05), 0.7),
    "floor": mat("Tavolo", (0.70, 0.71, 0.73), 0.55),
}

def finish(o, m):
    o.data.materials.clear(); o.data.materials.append(m)
    for p in o.data.polygons: p.use_smooth = True
    es = o.modifiers.new("spigoli", "EDGE_SPLIT"); es.split_angle = math.radians(35)

def stl(path, m, xf=Matrix.Identity(4)):
    bpy.ops.wm.stl_import(filepath=os.path.join(BUILD, path), global_scale=MM)
    o = bpy.context.selected_objects[0]
    o.matrix_world = xf @ o.matrix_world
    finish(o, m); return o

def glb(path, keep, xf_mm=Matrix.Identity(4)):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=os.path.join(LL, path))
    for o in [o for o in bpy.data.objects if o not in before]:
        if o.type != "MESH": continue
        if o.name not in keep:
            bpy.data.objects.remove(o, do_unlink=True); continue
        o.matrix_world = Matrix.Scale(MM, 4) @ xf_mm @ o.matrix_world
        finish(o, keep[o.name])

# ---------------------------------------------------------------- forme base (mm)
def smooth_mod(o, w, seg=5):
    b = o.modifiers.new("smusso", "BEVEL"); b.width = w * MM; b.segments = seg; b.limit_method = "ANGLE"
    for p in o.data.polygons: p.use_smooth = True
    o.modifiers.new("normali", "WEIGHTED_NORMAL")

def box(c, s, m, bev=0.0):
    bpy.ops.mesh.primitive_cube_add(size=1, location=[v * MM for v in c]); o = bpy.context.object
    o.scale = [v * MM for v in s]; bpy.ops.object.transform_apply(scale=True)
    o.data.materials.append(m)
    if bev: smooth_mod(o, bev)
    return o

def cyl(c, r, h, m, axis="Z", v=96, bev=0.0):
    rot = {"Z": (0, 0, 0), "X": (0, math.pi / 2, 0), "Y": (math.pi / 2, 0, 0)}[axis]
    bpy.ops.mesh.primitive_cylinder_add(radius=r * MM, depth=h * MM, location=[x * MM for x in c], rotation=rot, vertices=v)
    o = bpy.context.object; o.data.materials.append(m)
    if bev: smooth_mod(o, bev)
    else:
        for p in o.data.polygons: p.use_smooth = True
        o.modifiers.new("spigoli", "EDGE_SPLIT").split_angle = math.radians(40)
    return o

# ---------------------------------------------------------------- launcher a corda (dalle foto del BX-28)
# Sistema di LaunchLab: X di traverso, Y lungo il launcher, Z in alto.
# Cime dei fianchi bianchi a Z 0 (la base LaunchLab appoggia li'), fascia nera
# centrale incassata, corda verso +Y, innesti a gradini verso -Y.
Y0, Y1 = -46.0, 40.0
BEY = (0.0, -4.0)

def launcher():
    L = Y1 - Y0; yc = (Y0 + Y1) / 2
    for sx in (-1, 1):                                            # fianchi bianchi arrotondati
        box((sx * 19.5, yc, -15), (13, L, 30), M["white"], bev=5.5)
    box((0, yc, -16.5), (27, L - 2, 28), M["band"], bev=1.5)       # fascia nera incassata
    # testata lato corda e uscita della corda
    box((0, Y1 + 2.5, -15), (40, 7, 24), M["white"], bev=3)
    cyl((0, Y1 + 6.5, -13), 2.2, 2, M["black"], axis="Y")
    # impugnatura a T sulla corda
    cyl((0, Y1 + 16, -13), 0.45, 18, M["string"], axis="Y", v=12)
    box((0, Y1 + 27, -13), (7, 6, 7), M["black"], bev=1.2)
    cyl((0, Y1 + 32.5, -13), 2.6, 6, M["white"], axis="Y", bev=0.6)
    cyl((0, Y1 + 36, -13), 2.8, 32, M["white"], axis="X", bev=0.8)
    cyl((0, Y1 + 39.5, -13), 3.6, 3, M["white"], axis="Y", bev=0.6)
    # innesti a gradini sul lato opposto
    box((0, Y0 - 3.5, -13), (32, 8, 18), M["white"], bev=1.5)
    box((0, Y0 - 9, -12), (20, 5, 10), M["white"], bev=1.2)
    for sx in (-1, 1): box((sx * 11, Y0 - 3.5, -24.5), (6, 8, 5), M["white"], bev=1)
    # feritoie sui fianchi per le clip (coperte dalle clip quando montate)
    for sx in (-1, 1): box((sx * 26.2, -14, -6), (1.0, 8, 3.5), M["black"])
    # trottola agganciata sotto
    bx, by = BEY; top = -30.6
    cyl((bx, by, top - 3), 21, 6, M["bey_metal"], bev=0.8)
    for k in range(3):
        a = k * 2 * math.pi / 3 + 0.4
        cyl((bx + 19 * math.cos(a), by + 19 * math.sin(a), top - 3), 8.5, 6, M["bey_metal"], bev=0.8)
    cyl((bx, by, top - 9.5), 17, 7, M["bey_color"], bev=1.2)
    bpy.ops.mesh.primitive_cone_add(radius1=9 * MM, radius2=2.2 * MM, depth=11 * MM,
                                    location=(bx * MM, by * MM, (top - 18.5) * MM), rotation=(math.pi, 0, 0), vertices=64)
    o = bpy.context.object; o.data.materials.append(M["black"])
    for p in o.data.polygons: p.use_smooth = True
    return top - 24.0  # quota della punta

def zip_tie(y):
    # fascetta attorno a launcher e scatola laterale
    outer = box((7.5, y, -14.5), (70, 3.6, 33), M["black"])
    inner = box((7.5, y, -14.5), (67.6, 5, 30.6), M["black"])
    mod = outer.modifiers.new("buco", "BOOLEAN"); mod.object = inner; mod.operation = "DIFFERENCE"
    bpy.context.view_layer.objects.active = outer; bpy.ops.object.modifier_apply(modifier="buco")
    bpy.data.objects.remove(inner, do_unlink=True)
    smooth_mod(outer, 0.5, 2)

def cable(points, colors, r=0.55):
    for k, col in enumerate(colors):
        cu = bpy.data.curves.new("cavo", "CURVE"); cu.dimensions = "3D"; cu.bevel_depth = r * MM; cu.bevel_resolution = 3
        sp = cu.splines.new("BEZIER"); sp.bezier_points.add(len(points) - 1)
        off = Vector(((k - 1) * 1.2, 0, 0))
        for bp, p in zip(sp.bezier_points, points):
            bp.co = (Vector(p) + off) * MM; bp.handle_left_type = bp.handle_right_type = "AUTO"
        ob = bpy.data.objects.new("cavo", cu); ob.data.materials.append(M[col]); sc.collection.objects.link(ob)

# ---------------------------------------------------------------- scena
tip = launcher()
if OPT == "A":
    glb("top-qre-raw.glb", {
        "Mesh_0": M["petg"], "Mesh_1": M["petg"], "Mesh_2": M["petg"], "Mesh_8": M["petg"],
        "Mesh_3": M["pcb_red"], "Mesh_4": M["black"], "Mesh_5": M["black"], "Mesh_6": M["black"],
        "Mesh_7": M["black"], "Mesh_9": M["screw"], "Mesh_10": M["screw"]})
    stl("a_box.stl", M["pla"]); stl("a_lid.stl", M["pla"]); stl("a_screws.stl", M["screw"])
    stl("a_switch.stl", M["black"])  # interruttore: leva e viti si vedono dal fianco
    for k, w in enumerate(["w_red", "w_black", "w_yellow"]): stl("a_wire%d.stl" % k, M[w])
    target = Vector((0, -2, 2)) * MM
else:
    # supporto TCRT5000: dal sistema del modello del sito al nostro (+42; +4,33; -3,46 mm)
    glb("side-tcrt-raw.glb", {
        "Mesh_0": M["petg"], "Mesh_1": M["petg"], "Mesh_2": M["petg"],
        "Mesh_3": M["pcb_blue"], "Mesh_4": M["black"], "Mesh_5": M["black"], "Mesh_6": M["pcb_blue"],
        "Mesh_7": M["black"], "Mesh_8": M["screw"]}, Matrix.Translation((42.0, 4.33, -3.46)))
    # scatola laterale sul fianco +X: fondo contro il fianco, lunga lungo Y.
    # (lx, ly, lz) della scatola -> (x = 26 + lz, y = -31,9 + lx, z = -27 + ly)
    P = Matrix(((0, 0, 1, 0), (1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 0, 1)))
    place = Matrix.Translation((26.2 * MM, -31.9 * MM, -27 * MM)) @ P
    stl("b_box.stl", M["pla"], place)
    flip = Matrix.Translation((0, 23.8 * MM, 15.2 * MM)) @ Matrix.Rotation(math.pi, 4, "X")
    stl("b_lid.stl", M["pla"], place @ flip)
    for y in (-35.5, 27.5): zip_tie(y)
    # cavo dal pettine del TCRT al foro cavi della scatola
    cable([(0, 25.5, 15.8), (6, 33, 14), (24, 28, 8), (36, 6, 2), (35.6, -16, -2.6)], ["w_red", "w_black", "w_yellow"])
    target = Vector((10, -2, -2)) * MM

# ---------------------------------------------------------------- tavolo, luci, camera
bpy.ops.mesh.primitive_plane_add(size=30, location=(0, 0, (tip - 0.3) * MM))
bpy.context.object.data.materials.append(M["floor"])
w = bpy.data.worlds.new("mondo"); sc.world = w; w.use_nodes = True
w.node_tree.nodes["Background"].inputs["Color"].default_value = (0.86, 0.87, 0.90, 1)
w.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.10

def area(name, loc, power, size, color=(1, 1, 1)):
    l = bpy.data.lights.new(name, "AREA"); l.energy = power; l.size = size; l.color = color
    o = bpy.data.objects.new(name, l); o.location = loc; sc.collection.objects.link(o)
    o.rotation_euler = (target - Vector(loc)).to_track_quat("-Z", "Y").to_euler()

area("principale", (-0.30, -0.32, 0.42), 9, 0.40, (1.0, 0.96, 0.90))
area("riempimento", (0.40, -0.18, 0.22), 3, 0.55, (0.90, 0.95, 1.0))
area("controluce", (0.06, 0.45, 0.32), 6, 0.30)

views = {  # posizione della camera (m) e focale
    "tre_quarti": ((-0.150, -0.185, 0.135), 60),
    "tre_quarti_dx": ((0.175, -0.165, 0.130), 60),
    "lato": ((0.300, -0.065, 0.120), 60),
    "alto": ((-0.06, -0.10, 0.23), 60),
}
pos, lens = views[VIEW]
cam = bpy.data.cameras.new("cam"); cam.lens = lens
cam.dof.use_dof = True; cam.dof.aperture_fstop = 10
co = bpy.data.objects.new("cam", cam); sc.collection.objects.link(co); sc.camera = co
co.location = pos
d = target - co.location; co.rotation_euler = d.to_track_quat("-Z", "Y").to_euler(); cam.dof.focus_distance = d.length

sc.render.engine = "CYCLES"; sc.cycles.samples = SAMPLES; sc.cycles.use_denoising = True; sc.cycles.device = "CPU"
sc.render.resolution_x = 1600; sc.render.resolution_y = 1100
sc.view_settings.view_transform = "AgX"; sc.view_settings.look = "AgX - Punchy"
sc.render.filepath = OUT
bpy.ops.render.render(write_still=True)
print("SCRITTO", OUT)
