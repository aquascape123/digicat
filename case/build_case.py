"""
DigiCat case generator (parametric) for the ESP32-2432S024R "CYD 2.4" board.

    pip install manifold3d trimesh numpy
    python build_case.py            # writes the STL files into ./stl

Coordinates: X = across the screen, Y = along the board (+Y = USB-C end, top),
Z = depth, 0 is the front face, growing towards the back.

Parts:
  body      - cat-shaped shell with ears, printed face down
  back      - back plate with PCB clamping posts, printed outer face down
  tail      - tail-shaped stand, screwed to the back plate, printed on its side
  fit_test  - first 7 mm of the body: print this first to check the window
"""
import math
import os

import numpy as np
from manifold3d import CrossSection, JoinType, Manifold

# ---------------------------------------------------------------------------
# Board (measure yours if something does not fit and adjust here)
# ---------------------------------------------------------------------------
POCKET_W = 44.2          # PCB 43 x 74.5 plus clearance
POCKET_L = 75.6
HOLE_DX = 36.7           # mounting hole spacing
HOLE_DY = 68.75
LCD_STACK = 4.0          # PCB front face -> display glass front face
PCB_T = 1.6
BACK_CLEAR = 7.0         # tallest part on the back of the PCB + margin

WIN_W = 39.5             # screen opening (active area is 36.7 x 49.0)
WIN_L = 53.0
WIN_Y = 1.5              # opening centre relative to the PCB centre

USB_W, USB_H = 17.0, 8.0  # cable opening between the ears
USB_X = 0.0

LIGHT_X, LIGHT_Y = 9.0, -33.5   # thin skin in front of the RGB LED / LDR
LIGHT_W, LIGHT_L = 14.0, 7.0

RESET_HOLES = [(-16.5, -22.7), (-16.5, -27.2)]  # BOOT / RST buttons on the back

# ---------------------------------------------------------------------------
# Case
# ---------------------------------------------------------------------------
WALL = 2.2
FRONT = 2.0              # front wall thickness
PLATE = 2.0              # back plate thickness
SEAT_WALL = 1.6          # rim around the back plate
FIT = 0.15               # back plate clearance per side
CHIN = 11.0              # material under the board (gives the cat a chin)
CORNER_R = 7.0
EAR_H = 14.0             # ear height above the head
EAR_BASE = 15.0          # ear width along the top edge
TILT = 16.0              # screen leans back this many degrees
PILOT_D = 1.7            # M2 self-tapping screws
CLEAR_D = 2.4
CSK_D = 4.2
TAIL_T = 8.0             # tail thickness
TAIL_REACH = 62.0        # tail foot distance from the front edge, along the desk

SEG = 64

D = FRONT + LCD_STACK + PCB_T + BACK_CLEAR + PLATE   # total depth
PCB_BACK = FRONT + LCD_STACK + PCB_T
W = POCKET_W + 2 * WALL
Y_TOP = POCKET_L / 2 + WALL
Y_BOT = -(POCKET_L / 2 + CHIN)

EAR_SCREWS = [(s * 17.5, Y_TOP + 2.5) for s in (-1, 1)]
CHIN_SCREWS = [(s * 16.0, -POCKET_L / 2 - 4.5) for s in (-1, 1)]
POSTS = [(sx * HOLE_DX / 2, sy * HOLE_DY / 2) for sx in (-1, 1) for sy in (-1, 1)]
TAIL_ROOT = (-14.0, 6.0)   # y range of the tail flange on the back plate


def rrect(w, h, r, cx=0.0, cy=0.0):
    r = min(r, w / 2 - 0.01, h / 2 - 0.01)
    return CrossSection.square((w - 2 * r, h - 2 * r), center=True).offset(r, JoinType.Round, circular_segments=SEG).translate((cx, cy))


def circle(r, cx=0.0, cy=0.0):
    return CrossSection.circle(r, SEG).translate((cx, cy))


def prism(cs, z0, z1):
    return Manifold.extrude(cs, z1 - z0).translate((0, 0, z0))


def cyl(r, z0, z1, x=0.0, y=0.0):
    return Manifold.cylinder(z1 - z0, r, r, SEG).translate((x, y, z0))


def silhouette():
    head = rrect(W, Y_TOP - Y_BOT, CORNER_R, 0, (Y_TOP + Y_BOT) / 2)
    ears = []
    for s in (-1, 1):
        tri = [(s * W / 2, Y_TOP - 12), (s * (W / 2 - EAR_BASE), Y_TOP + 0.5), (s * (W / 2 - 4.5), Y_TOP + EAR_H)]
        ears.append(CrossSection([tri if s < 0 else tri[::-1]]))  # counter-clockwise
    shape = head + ears[0] + ears[1]
    # round convex corners (ear tips) and concave ones (ear roots)
    shape = shape.offset(-2.5, JoinType.Round, circular_segments=SEG).offset(2.5, JoinType.Round, circular_segments=SEG)
    shape = shape.offset(2.0, JoinType.Round, circular_segments=SEG).offset(-2.0, JoinType.Round, circular_segments=SEG)
    return shape


def chamfered(cs, depth, front_chamfer=1.0, back_round=1.0, steps=5):
    """Extrude with a 45 degree chamfer on the front (bed) edge and a rounded back edge."""
    def inset(d):
        return cs.offset(-d, JoinType.Round, circular_segments=SEG) if d > 0.01 else cs

    c, r = front_chamfer, back_round
    parts = []
    for i in range(steps):
        za, zb = c * i / steps, c * (i + 1) / steps
        parts.append(prism(inset(c - zb + c / steps / 2), za, zb + 0.01))
    parts.append(prism(cs, c, depth - r + 0.01))
    for i in range(steps):
        a0, a1 = i / steps * math.pi / 2, (i + 1) / steps * math.pi / 2
        za, zb = depth - r + r * math.sin(a0), depth - r + r * math.sin(a1)
        parts.append(prism(inset(r * (1 - math.cos((a0 + a1) / 2))), za, zb + (0.01 if i < steps - 1 else 0)))
    return Manifold.batch_boolean(parts, _union())


def _union():
    from manifold3d import OpType
    return OpType.Add


def desk_cut(m):
    """Remove everything below the desk when the case leans back by TILT degrees."""
    t = math.radians(TILT)
    n = (0.0, math.cos(t), -math.sin(t))          # points up, away from the desk
    offset = Y_BOT * math.cos(t)                   # plane passes through (y=Y_BOT, z=0)
    return m.trim_by_plane(n, offset)


def paw(cx, cy):
    toes = [circle(1.35, cx + dx, cy + dy) for dx, dy in [(-3.2, 2.6), (-1.1, 4.1), (1.1, 4.1), (3.2, 2.6)]]
    pad = CrossSection.circle(1, SEG).scale((2.6, 2.1)).translate((cx, cy - 0.4))
    return CrossSection.batch_boolean([pad] + toes, _union())


def inner_ears():
    ears = []
    for s in (-1, 1):
        tri = [(s * (W / 2 - 3.5), Y_TOP - 2.5), (s * (W / 2 - EAR_BASE + 3.5), Y_TOP + 1.5), (s * (W / 2 - 5.5), Y_TOP + EAR_H - 4.5)]
        ears.append(CrossSection([tri if s < 0 else tri[::-1]]).offset(-0.8, JoinType.Round, circular_segments=SEG)
                    .offset(0.8, JoinType.Round, circular_segments=SEG))
    return ears[0] + ears[1]


def build_body():
    sil = silhouette()
    body = chamfered(sil, D, front_chamfer=1.2, back_round=1.0)

    cuts = []
    # board pocket, open to the back
    cuts.append(prism(rrect(POCKET_W, POCKET_L, 1.0), FRONT, D + 1))
    # seat for the back plate
    cuts.append(prism(sil.offset(-SEAT_WALL, JoinType.Round, circular_segments=SEG), D - PLATE, D + 1))
    # screen opening with a 45 degree bevel towards the front
    bevel = 1.2
    win = rrect(WIN_W, WIN_L, 1.5)
    big = rrect(WIN_W + 2 * bevel, WIN_L + 2 * bevel, 1.5 + bevel)
    cuts.append(Manifold.extrude(big, bevel, scale_top=(WIN_W / (WIN_W + 2 * bevel), WIN_L / (WIN_L + 2 * bevel)))
                .translate((0, WIN_Y, -0.001)))
    cuts.append(prism(win.translate((0, WIN_Y)), bevel - 0.01, FRONT + 0.5))
    # thin skin in front of the LED so it glows through
    cuts.append(prism(rrect(LIGHT_W, LIGHT_L, 2.0, LIGHT_X, LIGHT_Y), 0.6, FRONT + 0.5))
    # USB-C cable opening between the ears
    usb_z = PCB_BACK + 1.7
    slot = rrect(USB_W, USB_H, USB_H / 2 - 0.01)
    usb = Manifold.extrude(slot, 30).rotate((-90, 0, 0)).translate((USB_X, POCKET_L / 2 - 1, usb_z))
    cuts.append(usb)
    # screw pilots
    for x, y in EAR_SCREWS + CHIN_SCREWS:
        cuts.append(cyl(PILOT_D / 2, D - PLATE - 8, D + 1, x, y))
    # paw prints on the chin and inner ears: 0.6 mm deep, fill them with a second colour
    deco = paw(-11, -42.5) + paw(11, -42.5) + inner_ears()
    cuts.append(prism(deco, -0.1, 0.6))

    body = body - Manifold.batch_boolean(cuts, _union())
    return desk_cut(body)


def build_back():
    sil = silhouette().offset(-SEAT_WALL - FIT, JoinType.Round, circular_segments=SEG)
    plate = prism(sil, D - PLATE, D)
    adds = []
    for x, y in POSTS:
        adds.append(cyl(2.0, PCB_BACK, D - PLATE + 0.01, x, y))   # presses the PCB forward
        adds.append(cyl(1.1, PCB_BACK - 1.4, PCB_BACK + 0.01, x, y))  # locating pin into the hole
    plate = Manifold.batch_boolean([plate] + adds, _union())

    cuts = []
    for x, y in EAR_SCREWS + CHIN_SCREWS:
        cuts.append(cyl(CLEAR_D / 2, D - PLATE - 1, D + 1, x, y))
        cuts.append(Manifold.cylinder(1.0, CLEAR_D / 2, CSK_D / 2, SEG).translate((x, y, D - 1.0 + 0.001)))
    for x, y in RESET_HOLES:
        cuts.append(cyl(1.0, D - PLATE - 1, D + 1, x, y))
    for y in tail_screw_ys():
        cuts.append(cyl(CLEAR_D / 2, D - PLATE - 1, D + 1, 0, y))
    # USB opening continues through the plate edge
    usb_z = PCB_BACK + 1.7
    slot = rrect(USB_W, USB_H, USB_H / 2 - 0.01)
    cuts.append(Manifold.extrude(slot, 30).rotate((-90, 0, 0)).translate((USB_X, POCKET_L / 2 - 1, usb_z)))
    plate = plate - Manifold.batch_boolean(cuts, _union())
    return desk_cut(plate)


def tail_screw_ys():
    y0, y1 = TAIL_ROOT
    return [y0 + 5, y1 - 5]


def build_tail():
    t = math.radians(TILT)
    y0, y1 = TAIL_ROOT
    r0, r1 = 4.5, 3.2

    # Path in the (y, z) plane: from the back plate down to the desk, then curling up
    foot_s = TAIL_REACH
    foot = (Y_BOT + foot_s * math.sin(t) + r1 * math.cos(t), foot_s * math.cos(t) - r1 * math.sin(t))
    p0 = ((y0 + y1) / 2, D + 2.0)
    c0 = (p0[0] - 4, D + 26)
    pts = []
    n = 24
    for i in range(n + 1):
        k = i / n
        u = 1 - k
        pts.append((u * u * p0[0] + 2 * u * k * c0[0] + k * k * foot[0],
                    u * u * p0[1] + 2 * u * k * c0[1] + k * k * foot[1],
                    r0 + (r1 - r0) * k))
    # curl: an arc rising from the foot, bending back towards the case
    cy, cz = foot[0] + 7.5 * math.cos(t), foot[1] - 7.5 * math.sin(t)
    start = math.atan2(foot[1] - cz, foot[0] - cy)
    for i in range(1, 14):
        a = start - i * math.radians(17)
        pts.append((cy + 7.5 * math.cos(a), cz + 7.5 * math.sin(a), r1 * (1 - 0.25 * i / 13)))

    # 2D stroke in (u=-z, v=y), extruded along the tail thickness
    blobs = [CrossSection.circle(r, SEG).translate((-z, y)) for y, z, r in pts]
    stroke = CrossSection.batch_hull([blobs[0], blobs[1]])
    for a, b in zip(blobs[1:], blobs[2:]):
        stroke = stroke + CrossSection.batch_hull([a, b])
    # flange lying on the back plate
    flange = CrossSection.square((3.0, y1 - y0)).translate((-(D + 3.0), y0))
    root = CrossSection.batch_hull([flange, blobs[0], blobs[2]])
    shape = stroke + root + flange
    tail = Manifold.extrude(shape, TAIL_T).rotate((0, 90, 0)).translate((-TAIL_T / 2, 0, 0))

    cuts = [cyl(PILOT_D / 2, D - 0.1, D + 9, 0, y) for y in tail_screw_ys()]
    tail = tail - Manifold.batch_boolean(cuts, _union())
    # don't let it poke into the back plate, flatten the foot on the desk
    tail = tail.trim_by_plane((0, 0, 1), D)
    return desk_cut(tail)


def to_trimesh(m):
    import trimesh
    mesh = m.to_mesh64()
    # manifold output is already clean; trimesh's vertex merging would only break it
    return trimesh.Trimesh(vertices=np.asarray(mesh.vert_properties)[:, :3], faces=np.asarray(mesh.tri_verts), process=False)


def lay_flat(m, normal_down):
    """Rotate so the given direction points to the bed, then sit on z=0."""
    import trimesh
    tm = to_trimesh(m)
    tm.apply_transform(trimesh.geometry.align_vectors(normal_down, [0, 0, -1]))
    tm.apply_translation(-tm.bounds[0] * [0, 0, 1] - [tm.centroid[0], tm.centroid[1], 0])
    return tm


def main():
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "stl")
    os.makedirs(out, exist_ok=True)
    body, back, tail = build_body(), build_back(), build_tail()
    fit = body.trim_by_plane((0, 0, -1), -(FRONT + LCD_STACK + 0.6))

    parts = {
        "digicat_body": lay_flat(body, [0, 0, -1]),       # front face on the bed
        "digicat_back": lay_flat(back, [0, 0, 1]),        # outer face on the bed
        "digicat_tail": lay_flat(tail, [1, 0, 0]),        # on its side
        "digicat_fit_test": lay_flat(fit, [0, 0, -1]),
    }
    for name, tm in parts.items():
        tm.export(os.path.join(out, name + ".stl"))
        print(f"{name:18s} {tm.extents.round(1)} mm  watertight={tm.is_watertight}  {tm.volume / 1000:.1f} cm3")
    return body, back, tail


if __name__ == "__main__":
    main()
