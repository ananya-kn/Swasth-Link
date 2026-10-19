"""Parametric CAD for the SwasthLink handheld (CadQuery). python build_cad.py -> out/*.step, out/*.stl
Nominal dimensions: MEASURE your OLED, MAX30102 and MLX90614 modules and edit the numbers below before printing."""
import cadquery as cq, os

OUT = os.path.join(os.path.dirname(__file__), "out"); os.makedirs(OUT, exist_ok=True)
W, H, D, WALL = 72.0, 132.0, 30.0, 2.4
body = cq.Workplane("XY").box(W, H, D, centered=(True, True, False)).edges("|Z").fillet(10).faces(">Z").edges().fillet(3)
body = body.faces("<Z").shell(-WALL)                                    # open back for PCB + battery
front = body.faces(">Z").workplane(centerOption="CenterOfBoundBox")
body = front.center(0, 45).rect(27, 15).cutBlind(-WALL - 1)              # OLED window
body = body.faces(">Z").workplane(centerOption="CenterOfBoundBox").center(0, 12).circle(12).cutBlind(-1.4)   # finger well: shallow recess, leaves 1 mm skin
body = body.faces(">Z").workplane(centerOption="CenterOfBoundBox").center(0, 12).rect(9, 6).cutBlind(-WALL - 6)  # MAX30102 optical window
body = body.faces(">Z").workplane(centerOption="CenterOfBoundBox").center(0, -18).circle(4.2).cutBlind(-WALL - 1)  # MLX90614 IR window
body = body.faces(">Z").workplane(centerOption="CenterOfBoundBox").pushPoints([(-20, -48), (20, -48)]).circle(4.2).cutBlind(-WALL - 1)  # SOS + MEASURE
body = body.faces(">Z").workplane(centerOption="CenterOfBoundBox").center(0, -34).circle(2.6).cutBlind(-WALL - 1)  # status LED
body = body.faces(">Z").workplane(centerOption="CenterOfBoundBox").pushPoints([(-6, 62), (0, 62), (6, 62)]).circle(0.8).cutBlind(-WALL - 1)  # mic holes
body = body.faces("<Y").workplane(centerOption="CenterOfBoundBox").center(0, D / 2 - 5).rect(9.5, 3.6).cutBlind(-WALL - 2)  # USB-C slot
cover = cq.Workplane("XY").box(W - 2 * WALL - 0.4, H - 2 * WALL - 0.4, 2.0, centered=(True, True, False)).edges("|Z").fillet(7)
cover = cover.faces(">Z").workplane().pushPoints([(-W / 2 + 8, -H / 2 + 8), (W / 2 - 8, -H / 2 + 8), (-W / 2 + 8, H / 2 - 8), (W / 2 - 8, H / 2 - 8)]).hole(3.2)
bumper = cq.Workplane("XY").box(W + 8, H + 8, 18, centered=(True, True, False)).edges("|Z").fillet(12).cut(
    cq.Workplane("XY").box(W - 0.4, H - 0.4, 20, centered=(True, True, False)).edges("|Z").fillet(10))
parts = dict(handheld_body=body, back_cover=cover, tpu_bumper=bumper)
for n, p in parts.items():
    cq.exporters.export(p, os.path.join(OUT, n + ".step")); cq.exporters.export(p, os.path.join(OUT, n + ".stl"))
    bb = p.val().BoundingBox(); print(f"{n}: {bb.xlen:.1f} x {bb.ylen:.1f} x {bb.zlen:.1f} mm, volume {p.val().Volume()/1000:.1f} cm3, valid={p.val().isValid()}")
