#!/usr/bin/env python3
# Copyright (c) 2026 dursuntokgoz. All rights reserved.
"""
Generates modern, high-resolution application screenshots for AndroCAD v28.0.0
showcasing bilingual Turkish/English localization, updated button iconography,
and complete CAD/CAE/CAM/PLM module capabilities.
"""

import os
import math
from PIL import Image, ImageDraw, ImageFont

OUTPUT_DIR = "/home/dursun/repo/AndroCad/docs/images"
os.makedirs(OUTPUT_DIR, exist_ok=True)

FONT_REG = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FONT_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

def get_font(size, bold=False):
    path = FONT_BOLD if bold else FONT_REG
    try:
        return ImageFont.truetype(path, size)
    except Exception:
        return ImageFont.load_default()

# Color Palette
BG_DARK = (20, 22, 24)
TOOLBAR_TOP = (30, 33, 36)
TOOLBAR_SUB = (37, 40, 44)
CHIP_BG = (47, 49, 54)
CHIP_BORDER = (65, 70, 78)
ACCENT_BLUE = (0, 122, 204)
HIGHLIGHT_CYAN = (0, 164, 228)
SUCCESS_GREEN = (76, 175, 80)
TEXT_WHITE = (255, 255, 255)
TEXT_MUTED = (160, 165, 175)
TEXT_DIM = (110, 115, 125)
BORDER_DARK = (43, 45, 49)
NEW_DESIGN_BG = (30, 58, 95)
NEW_DESIGN_TEXT = (144, 202, 249)

def draw_rounded_rect(draw, bbox, radius, fill=None, outline=None, width=1):
    x0, y0, x1, y1 = bbox
    draw.rounded_rectangle([x0, y0, x1, y1], radius=radius, fill=fill, outline=outline, width=width)

def draw_top_bar(draw, width, is_tr=True, active_tab="Solid"):
    # Top bar background
    draw.rectangle([0, 0, width, 44], fill=TOOLBAR_TOP)
    draw.line([0, 44, width, 44], fill=BORDER_DARK, width=1)

    # Logo & App Title
    draw_rounded_rect(draw, [12, 10, 36, 34], 4, fill=ACCENT_BLUE)
    draw.text((18, 11), "A", font=get_font(18, True), fill=TEXT_WHITE)
    draw.text((44, 12), "AndroCAD", font=get_font(16, True), fill=TEXT_WHITE)

    # Tabs
    tabs_en = ["Solid", "Sketch", "Surface", "Sheet Metal", "Mesh", "Simulation", "Manufacture (CAM)", "Additive (3D)", "Drawing", "Assemble", "Inspect", "File / IO"]
    tabs_tr = ["Katı Model", "Eskiz", "Yüzey", "Sac Metal", "Ağ (Mesh)", "Simülasyon", "İmalat (CAM)", "Katmanlı (3B)", "Teknik Resim", "Montaj", "İncele", "Dosya / G/Ç"]
    tabs = tabs_tr if is_tr else tabs_en

    cur_x = 150
    for idx, tab_name in enumerate(tabs):
        is_active = (tab_name == active_tab) or (is_tr and tabs_en[idx] == active_tab)
        color = ACCENT_BLUE if is_active else TEXT_MUTED
        font = get_font(12, is_active)
        w = draw.textlength(tab_name, font=font)
        if cur_x + w > width - 420:
            break
        draw.text((cur_x, 15), tab_name, font=font, fill=color)
        if is_active:
            draw.line([cur_x, 42, cur_x + w, 42], fill=ACCENT_BLUE, width=3)
        cur_x += int(w) + 18

    # Right side action buttons
    right_x = width - 12
    # New Design
    new_lbl = "✨ YENİ TASARIM" if is_tr else "✨ NEW DESIGN"
    w_new = int(draw.textlength(new_lbl, font=get_font(10, True))) + 16
    right_x -= w_new
    draw_rounded_rect(draw, [right_x, 9, right_x + w_new, 35], 6, fill=NEW_DESIGN_BG, outline=NEW_DESIGN_TEXT)
    draw.text((right_x + 8, 14), new_lbl, font=get_font(10, True), fill=NEW_DESIGN_TEXT)

    # Redo & Undo
    right_x -= 58
    draw_rounded_rect(draw, [right_x, 9, right_x + 52, 35], 6, fill=CHIP_BG, outline=CHIP_BORDER)
    draw.text((right_x + 7, 14), "↷ YİNELE" if is_tr else "↷ REDO", font=get_font(9), fill=TEXT_WHITE)

    right_x -= 68
    draw_rounded_rect(draw, [right_x, 9, right_x + 62, 35], 6, fill=CHIP_BG, outline=CHIP_BORDER)
    draw.text((right_x + 7, 14), "↶ GERİ AL" if is_tr else "↶ UNDO", font=get_font(9), fill=TEXT_WHITE)

    # Theme
    right_x -= 66
    draw_rounded_rect(draw, [right_x, 9, right_x + 60, 35], 6, fill=CHIP_BG, outline=CHIP_BORDER)
    draw.text((right_x + 7, 14), "🌙 KOYU" if is_tr else "🌙 DARK", font=get_font(9), fill=TEXT_WHITE)

    # Language Toggle Button (Modern highlighted)
    lang_lbl = "🇹🇷 TÜRKÇE" if is_tr else "🇬🇧 ENGLISH"
    w_lang = int(draw.textlength(lang_lbl, font=get_font(10, True))) + 16
    right_x -= (w_lang + 8)
    draw_rounded_rect(draw, [right_x, 9, right_x + w_lang, 35], 6, fill=(35, 45, 60), outline=(0, 164, 228))
    draw.text((right_x + 8, 14), lang_lbl, font=get_font(10, True), fill=(100, 210, 255))

def draw_sub_toolbar(draw, width, tools, is_tr=True, active_tool=None):
    draw.rectangle([0, 45, width, 84], fill=TOOLBAR_SUB)
    draw.line([0, 84, width, 84], fill=BORDER_DARK, width=1)

    cur_x = 12
    for tool_name, icon_symbol in tools:
        is_active = (tool_name == active_tool)
        bg = ACCENT_BLUE if is_active else CHIP_BG
        outline = (0, 164, 228) if is_active else CHIP_BORDER
        text_col = TEXT_WHITE if is_active else TEXT_WHITE

        lbl = f"{icon_symbol} {tool_name}"
        font = get_font(11, is_active)
        w = int(draw.textlength(lbl, font=font)) + 18
        if cur_x + w > width - 12:
            break
        draw_rounded_rect(draw, [cur_x, 50, cur_x + w, 78], 6, fill=bg, outline=outline)
        draw.text((cur_x + 9, 56), lbl, font=font, fill=text_col)
        cur_x += w + 6

def draw_browser_sidebar(draw, height, items, is_tr=True):
    sidebar_w = 230
    draw.rectangle([0, 85, sidebar_w, height - 30], fill=(22, 25, 28))
    draw.line([sidebar_w, 85, sidebar_w, height - 30], fill=BORDER_DARK, width=1)

    title = "MODEL TARAYICI" if is_tr else "MODEL BROWSER"
    draw.text((14, 98), title, font=get_font(11, True), fill=TEXT_MUTED)

    y = 124
    for icon, name, visible, is_selected in items:
        if is_selected:
            draw_rounded_rect(draw, [6, y - 4, sidebar_w - 6, y + 20], 4, fill=(30, 50, 75))
        
        # Visibility eye
        eye_col = (100, 200, 100) if visible else (120, 120, 120)
        draw.text((14, y), "👁" if visible else "⊘", font=get_font(10), fill=eye_col)

        # Icon & Name
        draw.text((36, y), icon, font=get_font(11), fill=TEXT_MUTED)
        name_col = (100, 210, 255) if is_selected else TEXT_WHITE
        draw.text((56, y), name, font=get_font(11, is_selected), fill=name_col)
        y += 26

    # Parametric Timeline section at bottom of sidebar
    draw.line([0, height - 150, sidebar_w, height - 150], fill=BORDER_DARK, width=1)
    tl_title = "ZAMAN ÇİZGİSİ (DAG)" if is_tr else "TIMELINE (DAG)"
    draw.text((14, height - 142), tl_title, font=get_font(10, True), fill=TEXT_MUTED)

    timeline_steps = [
        "1. Kutu (100x50x20)" if is_tr else "1. Box (100x50x20)",
        "2. Radyus (R=3.0 mm)" if is_tr else "2. Fillet (R=3.0 mm)",
        "3. M6 ISO Delik" if is_tr else "3. M6 ISO Hole",
        "4. Dairesel Dizi (4x)" if is_tr else "4. Circular Pattern (4x)"
    ]
    ty = height - 124
    for step in timeline_steps:
        draw.text((14, ty), f"▪ {step}", font=get_font(10), fill=(180, 200, 220))
        ty += 20

def draw_viewcube(draw, x, y, is_tr=True):
    # Viewcube container
    draw_rounded_rect(draw, [x, y, x + 110, y + 100], 8, fill=(30, 33, 38), outline=CHIP_BORDER)
    title = "BAKIŞ KÜPÜ" if is_tr else "VIEW CUBE"
    draw.text((x + 22, y + 6), title, font=get_font(9, True), fill=TEXT_MUTED)

    # Top button
    draw_rounded_rect(draw, [x + 35, y + 22, x + 75, y + 42], 4, fill=(45, 48, 54), outline=(70, 75, 85))
    draw.text((x + 44, y + 26), "ÜST" if is_tr else "TOP", font=get_font(9, True), fill=TEXT_WHITE)

    # Middle ring: L, FRONT/ÖN, R
    draw_rounded_rect(draw, [x + 8, y + 46, x + 32, y + 66], 4, fill=(45, 48, 54), outline=(70, 75, 85))
    draw.text((x + 13, y + 50), "SOL" if is_tr else "L", font=get_font(8, True), fill=TEXT_WHITE)

    draw_rounded_rect(draw, [x + 36, y + 46, x + 74, y + 66], 4, fill=(0, 122, 204), outline=(0, 164, 228))
    draw.text((x + 47, y + 50), "ÖN" if is_tr else "FRONT", font=get_font(8, True), fill=TEXT_WHITE)

    draw_rounded_rect(draw, [x + 78, y + 46, x + 102, y + 66], 4, fill=(45, 48, 54), outline=(70, 75, 85))
    draw.text((x + 83, y + 50), "SAĞ" if is_tr else "R", font=get_font(8, True), fill=TEXT_WHITE)

    # Bottom ring: BOTTOM/ALT, ISO/İZO
    draw_rounded_rect(draw, [x + 16, y + 70, x + 54, y + 90], 4, fill=(45, 48, 54), outline=(70, 75, 85))
    draw.text((x + 24, y + 74), "ALT" if is_tr else "BOT", font=get_font(8, True), fill=TEXT_WHITE)

    draw_rounded_rect(draw, [x + 58, y + 70, x + 94, y + 90], 4, fill=(45, 48, 54), outline=(70, 75, 85))
    draw.text((x + 67, y + 74), "İZO" if is_tr else "ISO", font=get_font(8, True), fill=(100, 210, 255))

def draw_bottom_nav(draw, width, height, is_tr=True, active_nav="Orbit"):
    # Center pill navigation
    pill_w = 360
    pill_x = (width - pill_w) // 2
    pill_y = height - 42
    draw_rounded_rect(draw, [pill_x, pill_y, pill_x + pill_w, pill_y + 30], 15, fill=(25, 28, 32), outline=CHIP_BORDER)

    nav_items = [
        ("🔄", "Yörünge" if is_tr else "Orbit"),
        ("✋", "Kaydır" if is_tr else "Pan"),
        ("⛶", "Sığdır" if is_tr else "Fit"),
        ("🧊", "Gölgeli+Kenar" if is_tr else "Shaded+Edges")
    ]

    nx = pill_x + 14
    for icon, label in nav_items:
        is_active = (label.startswith("Yörünge") or label.startswith("Orbit"))
        color = ACCENT_BLUE if is_active else TEXT_MUTED
        txt = f"{icon} {label}"
        font = get_font(11, is_active)
        draw.text((nx, pill_y + 7), txt, font=font, fill=color)
        nx += int(draw.textlength(txt, font=font)) + 16

    # Bottom status bar
    draw.rectangle([0, height - 20, width, height], fill=(16, 18, 20))
    status_txt = "Doğruluk: 0.000 mm • Çözücü: PlaneGCS (0 DoF) • OCCT 8.0.1 B-Rep • 60 FPS GL" if is_tr else "Precision: 0.000 mm • Solver: PlaneGCS (0 DoF) • OCCT 8.0.1 B-Rep • 60 FPS GL"
    draw.text((12, height - 16), status_txt, font=get_font(9), fill=TEXT_DIM)

def draw_3d_grid(draw, cx, cy, span_x=380, span_y=220):
    grid_col = (32, 36, 42)
    axis_x_col = (180, 50, 50)
    axis_y_col = (50, 180, 50)

    # Perspective grid lines
    for i in range(-6, 7):
        ox = i * 40
        # Line along U
        p0 = (cx + ox - span_x // 2, cy + span_y // 2 + ox // 4)
        p1 = (cx + ox + span_x // 2, cy - span_y // 2 + ox // 4)
        draw.line([p0, p1], fill=grid_col, width=1)
        # Line along V
        q0 = (cx - span_x // 2 + ox // 2, cy - span_y // 2 - ox // 4)
        q1 = (cx + span_x // 2 + ox // 2, cy + span_y // 2 - ox // 4)
        draw.line([q0, q1], fill=grid_col, width=1)

def draw_3d_bracket(draw, cx, cy):
    """Draws an isometric precision mechanical bracket with shaded faces & edges."""
    # Base block coordinates
    # Top face
    poly_top = [
        (cx - 140, cy - 40),
        (cx, cy - 110),
        (cx + 160, cy - 30),
        (cx + 20, cy + 40)
    ]
    draw.polygon(poly_top, fill=(55, 62, 72), outline=(100, 115, 135))

    # Front-left face
    poly_left = [
        (cx - 140, cy - 40),
        (cx + 20, cy + 40),
        (cx + 20, cy + 120),
        (cx - 140, cy + 40)
    ]
    draw.polygon(poly_left, fill=(40, 45, 52), outline=(100, 115, 135))

    # Front-right face
    poly_right = [
        (cx + 20, cy + 40),
        (cx + 160, cy - 30),
        (cx + 160, cy + 50),
        (cx + 20, cy + 120)
    ]
    draw.polygon(poly_right, fill=(30, 34, 40), outline=(100, 115, 135))

    # Vertical L-flange extrusion
    poly_flange_top = [
        (cx - 140, cy - 40),
        (cx - 60, cy - 80),
        (cx - 40, cy - 70),
        (cx - 120, cy - 30)
    ]
    draw.polygon(poly_flange_top, fill=(70, 78, 90), outline=(120, 135, 155))

    poly_flange_front = [
        (cx - 120, cy - 30),
        (cx - 40, cy - 70),
        (cx - 40, cy + 10),
        (cx - 120, cy + 50)
    ]
    draw.polygon(poly_flange_front, fill=(48, 54, 62), outline=(120, 135, 155))

    # Cylindrical Boss & Counterbore Holes (Ellipse representations)
    draw.ellipse([cx + 60, cy - 15, cx + 110, cy + 10], fill=(42, 47, 55), outline=(100, 115, 135))
    draw.ellipse([cx + 72, cy - 9, cx + 98, cy + 4], fill=(22, 24, 28), outline=(0, 164, 228))

    draw.ellipse([cx - 95, cy - 10, cx - 65, cy + 5], fill=(35, 40, 48), outline=(100, 115, 135))
    draw.ellipse([cx - 88, cy - 6, cx - 72, cy + 1], fill=(20, 22, 26), outline=(0, 164, 228))

    # Chamfer & Fillet highlight lines
    draw.line([(cx - 140, cy - 40), (cx + 20, cy + 40)], fill=(0, 164, 228), width=2)
    draw.line([(cx + 20, cy + 40), (cx + 160, cy - 30)], fill=(0, 164, 228), width=2)
    draw.line([(cx + 20, cy + 40), (cx + 20, cy + 120)], fill=(0, 164, 228), width=2)

    # 3D Origin Triad at corner
    ox, oy = cx - 180, cy + 140
    draw.line([ox, oy, ox + 35, oy + 10], fill=(220, 50, 50), width=3) # X Axis (Red)
    draw.text((ox + 40, oy + 5), "X", font=get_font(10, True), fill=(240, 80, 80))
    draw.line([ox, oy, ox - 20, oy - 20], fill=(50, 220, 50), width=3) # Y Axis (Green)
    draw.text((ox - 32, oy - 30), "Y", font=get_font(10, True), fill=(80, 240, 80))
    draw.line([ox, oy, ox, oy - 35], fill=(50, 120, 255), width=3)     # Z Axis (Blue)
    draw.text((ox - 4, oy - 48), "Z", font=get_font(10, True), fill=(100, 180, 255))

# =========================================================================
# Image 1: Tablet Workspace Dark - Turkish UI
# =========================================================================
def generate_tablet_turkish():
    w, h = 1376, 768
    im = Image.new("RGB", (w, h), BG_DARK)
    draw = ImageDraw.Draw(im)

    draw_top_bar(draw, w, is_tr=True, active_tab="Katı Model")
    solid_tools = [
        ("Kutu", "📦"),
        ("Silindir", "🛢️"),
        ("Küre", "⚪"),
        ("Torus", "🍩"),
        ("Koni", "🔺"),
        ("Ekstrüzyon", "⬆️"),
        ("Döndür", "🔄"),
        ("Radyus", "⌒"),
        ("Pah", "◢"),
        ("Delik", "🕳️"),
        ("Kabuk", "🐚"),
        ("Birleştir", "🔗"),
        ("Taşı/Döndür", "✥"),
        ("Dizi (Rect)", "⊞"),
        ("Aynalama", "🪞"),
        ("ISO Delik (M6)", "🔩")
    ]
    draw_sub_toolbar(draw, w, solid_tools, is_tr=True, active_tool="Kutu")

    browser_items = [
        ("⚙️", "Belge Ayarları", True, False),
        ("📐", "Orijin Düzlemleri", True, False),
        ("  ↳", "XY Düzlemi", True, False),
        ("  ↳", "XZ Düzlemi", True, False),
        ("  ↳", "YZ Düzlemi", True, False),
        ("🧱", "Katı Gövde 1 (Braket)", True, True),
        ("✏️", "Eskiz 1 (Taban Profili)", True, False),
        ("✏️", "Eskiz 2 (M6 Delik Ekseni)", True, False),
        ("🔩", "M6 ISO Civata Grubu", True, False)
    ]
    draw_browser_sidebar(draw, h, browser_items, is_tr=True)

    draw_3d_grid(draw, 780, 420)
    draw_3d_bracket(draw, 780, 400)
    draw_viewcube(draw, w - 128, 96, is_tr=True)
    draw_bottom_nav(draw, w, h, is_tr=True, active_nav="Yörünge")

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_tablet_turkish_dark.png"), "PNG")
    print("Generated androcad_v28_tablet_turkish_dark.png")

# =========================================================================
# Image 2: Tablet Workspace Dark - English UI
# =========================================================================
def generate_tablet_english():
    w, h = 1376, 768
    im = Image.new("RGB", (w, h), BG_DARK)
    draw = ImageDraw.Draw(im)

    draw_top_bar(draw, w, is_tr=False, active_tab="Solid")
    solid_tools = [
        ("Box", "📦"),
        ("Cylinder", "🛢️"),
        ("Sphere", "⚪"),
        ("Torus", "🍩"),
        ("Cone", "🔺"),
        ("Extrude", "⬆️"),
        ("Revolve", "🔄"),
        ("Fillet", "⌒"),
        ("Chamfer", "◢"),
        ("Hole", "🕳️"),
        ("Shell", "🐚"),
        ("Combine", "🔗"),
        ("Move/Rotate", "✥"),
        ("Rect Pattern", "⊞"),
        ("Mirror", "🪞"),
        ("ISO Hole (M6)", "🔩")
    ]
    draw_sub_toolbar(draw, w, solid_tools, is_tr=False, active_tool="Box")

    browser_items = [
        ("⚙️", "Document Settings", True, False),
        ("📐", "Origin Planes", True, False),
        ("  ↳", "XY Plane", True, False),
        ("  ↳", "XZ Plane", True, False),
        ("  ↳", "YZ Plane", True, False),
        ("🧱", "Solid Body 1 (Bracket)", True, True),
        ("✏️", "Sketch 1 (Base Profile)", True, False),
        ("✏️", "Sketch 2 (M6 Hole Center)", True, False),
        ("🔩", "M6 ISO Fastener Group", True, False)
    ]
    draw_browser_sidebar(draw, h, browser_items, is_tr=False)

    draw_3d_grid(draw, 780, 420)
    draw_3d_bracket(draw, 780, 400)
    draw_viewcube(draw, w - 128, 96, is_tr=False)
    draw_bottom_nav(draw, w, h, is_tr=False, active_nav="Orbit")

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_tablet_english_dark.png"), "PNG")
    print("Generated androcad_v28_tablet_english_dark.png")

# =========================================================================
# Image 3: 2D Parametric Sketcher & PlaneGCS Solver
# =========================================================================
def generate_sketcher_solver_ui():
    w, h = 1376, 768
    im = Image.new("RGB", (w, h), (18, 20, 23))
    draw = ImageDraw.Draw(im)

    draw_top_bar(draw, w, is_tr=True, active_tab="Eskiz")
    sketch_tools = [
        ("Dikdörtgen", "▭"),
        ("Çizgi", "╱"),
        ("Daire", "◯"),
        ("Çokgen", "⬡"),
        ("Kanal (Slot)", "⬯"),
        ("Öteleme", "⫘"),
        ("B-Spline", "〰"),
        ("Kenar İzdüşür", "◱"),
        ("Ölçülendir", "📏"),
        ("Kısıtlamalar", "🔒"),
        ("Eskizi Bitir ✓", "✓")
    ]
    draw_sub_toolbar(draw, w, sketch_tools, is_tr=True, active_tool="Ölçülendir")

    # 2D Grid
    grid_col = (28, 32, 38)
    for x in range(240, w, 30):
        draw.line([x, 85, x, h - 30], fill=grid_col, width=1)
    for y in range(85, h - 30, 30):
        draw.line([240, y, w, y], fill=grid_col, width=1)

    # Sketch origin
    so_x, so_y = 650, 420
    draw.line([so_x - 15, so_y, so_x + 15, so_y], fill=(220, 60, 60), width=2)
    draw.line([so_x, so_y - 15, so_x, so_y + 15], fill=(60, 220, 60), width=2)
    draw.ellipse([so_x - 3, so_y - 3, so_x + 3, so_y + 3], fill=(255, 255, 100))

    # Constrained profile (Fully Solved - Green contours)
    p0 = (so_x - 180, so_y - 120)
    p1 = (so_x + 180, so_y - 120)
    p2 = (so_x + 180, so_y + 120)
    p3 = (so_x - 60, so_y + 120)
    p4 = (so_x - 60, so_y + 30)
    p5 = (so_x - 180, so_y + 30)
    pts = [p0, p1, p2, p3, p4, p5]

    draw.polygon(pts, fill=(35, 75, 45, 100), outline=(76, 175, 80), width=3)
    for pt in pts:
        draw.rectangle([pt[0] - 4, pt[1] - 4, pt[0] + 4, pt[1] + 4], fill=(255, 255, 255), outline=(0, 0, 0))

    # Dimension lines & labels
    # Width 120.00 mm
    draw.line([p0[0], p0[1] - 30, p1[0], p1[1] - 30], fill=(0, 164, 228), width=2)
    draw.line([p0[0], p0[1] - 10, p0[0], p0[1] - 35], fill=(0, 164, 228), width=1)
    draw.line([p1[0], p1[1] - 10, p1[0], p1[1] - 35], fill=(0, 164, 228), width=1)
    draw_rounded_rect(draw, [so_x - 45, p0[1] - 42, so_x + 45, p0[1] - 18], 4, fill=(15, 30, 45), outline=(0, 164, 228))
    draw.text((so_x - 38, p0[1] - 37), "120.00 mm", font=get_font(11, True), fill=(100, 210, 255))

    # Height 80.00 mm
    draw.line([p1[0] + 30, p1[1], p1[0] + 30, p2[1]], fill=(0, 164, 228), width=2)
    draw.line([p1[0] + 10, p1[1], p1[0] + 35, p1[1]], fill=(0, 164, 228), width=1)
    draw.line([p2[0] + 10, p2[1], p2[0] + 35, p2[1]], fill=(0, 164, 228), width=1)
    draw_rounded_rect(draw, [p1[0] + 16, so_y - 12, p1[0] + 96, so_y + 12], 4, fill=(15, 30, 45), outline=(0, 164, 228))
    draw.text((p1[0] + 24, so_y - 7), "80.00 mm", font=get_font(11, True), fill=(100, 210, 255))

    # Solver status card top-left
    draw_rounded_rect(draw, [250, 96, 520, 150], 8, fill=(22, 45, 30), outline=(76, 175, 80))
    draw.text((262, 104), "✓ PlaneGCS Çözücü: TAM TANIMLI", font=get_font(12, True), fill=(120, 255, 130))
    draw.text((262, 126), "0 Serbestlik Derecesi (DoF) • 8 Geometrik Kısıt • Kalıntı: 1.2e-8", font=get_font(9), fill=(180, 230, 190))

    # Browser sidebar
    browser_items = [
        ("⚙️", "Eskiz Düzlemi (XY)", True, False),
        ("🔒", "Yatay Kısıt H1", True, False),
        ("🔒", "Dikey Kısıt V1", True, False),
        ("🔒", "Çakışık Kısıt C1", True, False),
        ("📏", "Ölçü D1 (120.00 mm)", True, False),
        ("📏", "Ölçü D2 (80.00 mm)", True, False),
        ("📏", "Ölçü D3 (30.00 mm)", True, False)
    ]
    draw_browser_sidebar(draw, h, browser_items, is_tr=True)
    draw_bottom_nav(draw, w, h, is_tr=True, active_nav="Sığdır")

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_sketcher_solver_ui.png"), "PNG")
    print("Generated androcad_v28_sketcher_solver_ui.png")

# =========================================================================
# Image 4: Simulation & FEA Stress Analysis
# =========================================================================
def generate_simulation_fea_ui():
    w, h = 1376, 768
    im = Image.new("RGB", (w, h), BG_DARK)
    draw = ImageDraw.Draw(im)

    draw_top_bar(draw, w, is_tr=True, active_tab="Simülasyon")
    fea_tools = [
        ("Kiriş FEA Çöz", "⚡"),
        ("Tet10 Kuadratik FEA", "▲"),
        ("Modal Analiz", "〰"),
        ("Sabit Mesnet", "⚓"),
        ("Yük Uygula (10 kN)", "⬇️"),
        ("Güvenlik Faktörü", "🛡️"),
        ("Deplasman Alanı", "📊")
    ]
    draw_sub_toolbar(draw, w, fea_tools, is_tr=True, active_tool="Tet10 Kuadratik FEA")

    draw_3d_grid(draw, 780, 420)

    # FEA Von Mises stress mesh heatmap on cantilever beam/bracket
    # Draw colored triangular elements representing quadratic Tet10 mesh
    triangles = [
        # Fixed root (High stress - Red / Orange)
        ([(600, 320), (660, 280), (660, 360)], (220, 50, 40)),
        ([(600, 320), (660, 360), (600, 400)], (240, 70, 30)),
        ([(600, 400), (660, 360), (660, 440)], (250, 100, 30)),
        ([(600, 400), (660, 440), (600, 480)], (255, 140, 20)),
        # Middle span (Medium stress - Yellow / Green)
        ([(660, 280), (740, 260), (740, 340)], (240, 200, 30)),
        ([(660, 280), (740, 340), (660, 360)], (230, 210, 40)),
        ([(660, 360), (740, 340), (740, 420)], (160, 220, 50)),
        ([(660, 360), (740, 420), (660, 440)], (120, 210, 60)),
        # Free tip (Low stress - Cyan / Deep Blue)
        ([(740, 260), (840, 250), (840, 330)], (60, 190, 210)),
        ([(740, 260), (840, 330), (740, 340)], (50, 160, 230)),
        ([(740, 340), (840, 330), (840, 410)], (40, 110, 220)),
        ([(740, 340), (840, 410), (740, 420)], (30, 70, 200)),
        ([(840, 250), (940, 250), (940, 330)], (30, 60, 190)),
        ([(840, 250), (940, 330), (840, 330)], (25, 50, 170)),
        ([(840, 330), (940, 330), (940, 410)], (20, 40, 150)),
        ([(840, 330), (940, 410), (840, 410)], (15, 30, 130)),
    ]
    for pts, col in triangles:
        draw.polygon(pts, fill=col, outline=(30, 35, 45), width=1)

    # Fixed support constraint markers at left
    for fy in range(300, 500, 30):
        draw.line([(585, fy), (600, fy)], fill=(220, 200, 50), width=3)
        draw.polygon([(575, fy - 6), (585, fy), (575, fy + 6)], fill=(220, 200, 50))

    # Applied Load Arrow at tip
    draw.line([(940, 190), (940, 245)], fill=(255, 60, 60), width=4)
    draw.polygon([(932, 235), (940, 252), (948, 235)], fill=(255, 60, 60))
    draw.text((915, 170), "F = 10 kN", font=get_font(11, True), fill=(255, 80, 80))

    # Stress Colorbar Legend
    cb_x, cb_y = w - 160, 120
    draw_rounded_rect(draw, [cb_x - 10, cb_y - 15, cb_x + 95, cb_y + 260], 8, fill=(25, 28, 32), outline=CHIP_BORDER)
    draw.text((cb_x, cb_y - 8), "von Mises (MPa)", font=get_font(9, True), fill=TEXT_WHITE)

    colors = [
        (230, 50, 40, "245.8 Max"),
        (250, 100, 30, "196.6"),
        (240, 200, 30, "147.5"),
        (120, 210, 60, "98.3"),
        (50, 160, 230, "49.2"),
        (20, 40, 150, "0.12 Min")
    ]
    for idx, (cr, cg, cb, val) in enumerate(colors):
        by = cb_y + 18 + idx * 38
        draw.rectangle([cb_x + 5, by, cb_x + 25, by + 34], fill=(cr, cg, cb))
        draw.text((cb_x + 32, by + 10), val, font=get_font(9), fill=TEXT_MUTED)

    # FEA Stats Overlay Bottom-Left
    draw_rounded_rect(draw, [250, h - 140, 640, h - 50], 8, fill=(20, 35, 45), outline=(0, 164, 228))
    draw.text((262, h - 132), "⚡ Tet10 Kuadratik FEA Çözümü Tamamlandı", font=get_font(11, True), fill=(100, 210, 255))
    draw.text((262, h - 110), "Maksimum Gerilme: 245.8 MPa • Akma Dayanımı: 350.0 MPa (St52)", font=get_font(9), fill=TEXT_WHITE)
    draw.text((262, h - 92), "Min Güvenlik Faktörü: 1.42 • Maks Deplasman: 0.184 mm • Mod 1: 142.6 Hz", font=get_font(9), fill=(180, 220, 240))

    # Browser sidebar
    browser_items = [
        ("⚡", "FEA Çözüm 1", True, True),
        ("⚓", "Sabit Mesnet (Taban)", True, False),
        ("⬇️", "Yük 1 (10,000 N)", True, False),
        ("🧱", "Katı: Yapısal Çelik", True, False),
        ("📊", "von Mises Gerilme", True, True),
        ("〰", "Deplasman Vektörü", True, False),
        ("🛡️", "Güvenlik Faktörü Dağılımı", True, False)
    ]
    draw_browser_sidebar(draw, h, browser_items, is_tr=True)
    draw_bottom_nav(draw, w, h, is_tr=True, active_nav="Yörünge")

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_simulation_fea_ui.png"), "PNG")
    print("Generated androcad_v28_simulation_fea_ui.png")

# =========================================================================
# Image 5: CAM G-Code & CNC Toolpath Generation
# =========================================================================
def generate_cam_gcode_ui():
    w, h = 1376, 768
    im = Image.new("RGB", (w, h), BG_DARK)
    draw = ImageDraw.Draw(im)

    draw_top_bar(draw, w, is_tr=True, active_tab="İmalat (CAM)")
    cam_tools = [
        ("G-Kodu Üret (GRBL)", "⚙️"),
        ("Haas G-Kodu", "🏭"),
        ("Fanuc G-Kodu", "🤖"),
        ("3B Uyarlanabilir Boşaltma", "🌀"),
        ("3B Scallop İnce İşleme", "✨"),
        ("Yüzey Temizleme", "▔"),
        ("2B Havuz Boşaltma", "🔲"),
        ("2B Profil Çevre", "⬚"),
        ("Gagalamalı Delme", "⛏️")
    ]
    draw_sub_toolbar(draw, w, cam_tools, is_tr=True, active_tool="3B Uyarlanabilir Boşaltma")

    draw_3d_grid(draw, 640, 420)
    draw_3d_bracket(draw, 640, 400)

    # CNC Adaptive Toolpaths (Cyan and Green spiral toolpath lines)
    for r in range(25, 120, 15):
        box = [640 - r, 360 - r // 2, 640 + r, 360 + r // 2]
        draw.arc(box, start=30, end=330, fill=(0, 220, 255), width=2)
    # Rapid travel moves (Yellow dashed lines)
    draw.line([(500, 260), (640, 300)], fill=(255, 220, 50), width=1)
    draw.line([(640, 300), (760, 280)], fill=(255, 220, 50), width=1)

    # G-Code Editor Panel on Right Side
    panel_w = 340
    panel_x = w - panel_w - 12
    draw_rounded_rect(draw, [panel_x, 96, panel_x + panel_w, h - 50], 8, fill=(18, 20, 24), outline=CHIP_BORDER)
    draw.text((panel_x + 14, 108), "CNC G-KODU ÇIKTISI (GRBL / FANUC)", font=get_font(10, True), fill=(100, 210, 255))

    gcode_lines = [
        "( AndroCAD v28 CAM Post-Processor )",
        "( Parça: Braket Taban - Malzeme: Al 6061-T6 )",
        "G21 (Metrik mm) G90 (Mutlak Koordinat)",
        "G17 G94 (XY Düzlemi, mm/dak İlerleme)",
        "T1 M06 (D=10mm Karbür Düz Freze)",
        "S12000 M03 (İş Mili 12,000 RPM CW)",
        "G54 (İş Sıfırı Seçimi)",
        "M08 (Soğutma Sıvısı Açık)",
        "G00 X0.000 Y0.000 Z15.000 (Hızlı Emniyet)",
        "G00 X25.400 Y18.250",
        "G01 Z-2.000 F800 (Dalma)",
        "G02 X35.400 Y28.250 I10.0 J0.0 F2400",
        "G01 X60.000 Y28.250 F3000",
        "G03 X70.000 Y38.250 I0.0 J10.0 F3000",
        "G01 Z-4.000 F800 (Katman 2 Boşaltma)",
        "G00 Z15.000 (Geri Çekilme)",
        "M09 (Soğutma Kapat)",
        "M05 (İş Mili Dur)",
        "G28 G91 Z0.0 (Referans Noktasına Dön)",
        "M30 (Program Sonu & Başa Dön)"
    ]
    gy = 136
    for line in gcode_lines:
        col = (120, 200, 140) if line.startswith("(") else (TEXT_WHITE if not line.startswith("G00") else (255, 210, 80))
        draw.text((panel_x + 14, gy), line, font=get_font(9), fill=col)
        gy += 18

    # Browser sidebar
    browser_items = [
        ("🏭", "CAM Operasyonları", True, False),
        ("🌀", "3B Uyarlanabilir Boşaltma", True, True),
        ("▔", "Yüzey Silme (Facing)", True, False),
        ("🔲", "2B Cep Boşaltma", True, False),
        ("⛏️", "M6 Delik Gagalamalı Delme", True, False),
        ("🔧", "Takım #1 (Ø10 Düz Freze)", True, False),
        ("🔧", "Takım #2 (Ø6 Karbür Matkap)", True, False)
    ]
    draw_browser_sidebar(draw, h, browser_items, is_tr=True)
    draw_bottom_nav(draw, w, h, is_tr=True, active_nav="Yörünge")

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_cam_gcode_ui.png"), "PNG")
    print("Generated androcad_v28_cam_gcode_ui.png")

# =========================================================================
# Image 6: Sheet Metal Unfold & Flat Pattern DXF
# =========================================================================
def generate_sheet_metal_ui():
    w, h = 1376, 768
    im = Image.new("RGB", (w, h), BG_DARK)
    draw = ImageDraw.Draw(im)

    draw_top_bar(draw, w, is_tr=True, active_tab="Sac Metal")
    sm_tools = [
        ("U-Braket", "⊔"),
        ("Açınım Deseni", "◫"),
        ("DXF Dışa Aktar", "💾"),
        ("Flanş Ekle", "⌍"),
        ("Büküm Payı (K-Faktör)", "📐"),
        ("Yırtılma Boşaltması", "✂️")
    ]
    draw_sub_toolbar(draw, w, sm_tools, is_tr=True, active_tool="Açınım Deseni")

    # Left side: 3D Formed Sheet Metal Body
    draw.text((320, 100), "3B BÜKÜLMÜŞ GÖVDE (FORMED STATE)", font=get_font(12, True), fill=(100, 210, 255))
    draw_3d_grid(draw, 450, 430, span_x=260, span_y=160)
    # Formed U-Bracket
    draw.polygon([(360, 380), (480, 320), (520, 340), (400, 400)], fill=(65, 75, 88), outline=(0, 164, 228), width=2)
    draw.polygon([(360, 380), (400, 400), (400, 460), (360, 440)], fill=(48, 56, 66), outline=(0, 164, 228), width=2)
    draw.polygon([(480, 320), (520, 340), (520, 400), (480, 380)], fill=(38, 44, 52), outline=(0, 164, 228), width=2)

    # Right side: 2D Unfolded Flat Pattern
    draw.text((850, 100), "2B AÇINIM DÜZLEMİ (UNFOLDED FLAT PATTERN - DXF)", font=get_font(12, True), fill=(120, 240, 140))
    # Unfolded rectangle with bend lines
    draw_rounded_rect(draw, [760, 160, 1260, 560], 4, fill=(24, 30, 36), outline=(76, 175, 80), width=2)
    # Flange tabs
    draw.rectangle([760, 260, 840, 460], fill=(30, 38, 46), outline=(76, 175, 80))
    draw.rectangle([1180, 260, 1260, 460], fill=(30, 38, 46), outline=(76, 175, 80))

    # Bend deduction lines (Magenta dashed)
    draw.line([(840, 160), (840, 560)], fill=(255, 64, 180), width=2)
    draw.line([(1180, 160), (1180, 560)], fill=(255, 64, 180), width=2)
    draw.text((848, 175), "BÜKÜM: 90.0° YUKARI R=2.0 mm (K=0.44)", font=get_font(9), fill=(255, 128, 220))
    draw.text((1000, 525), "BÜKÜM: 90.0° YUKARI R=2.0 mm", font=get_font(9), fill=(255, 128, 220))

    # Mounting holes on flat sheet
    draw.ellipse([920, 340, 960, 380], fill=(18, 20, 24), outline=(0, 164, 228), width=2)
    draw.ellipse([1060, 340, 1100, 380], fill=(18, 20, 24), outline=(0, 164, 228), width=2)

    # Sheet Metal Invariants Box
    draw_rounded_rect(draw, [760, 590, 1260, 670], 6, fill=(20, 35, 30), outline=(76, 175, 80))
    draw.text((774, 600), "📐 Analitik Açınım Parametreleri (DIN 6935)", font=get_font(11, True), fill=(120, 240, 140))
    draw.text((774, 622), "K-Faktörü: 0.440 • Sac Kalınlığı (T): 2.0 mm • Büküm Radyusu: 2.0 mm", font=get_font(9), fill=TEXT_WHITE)
    draw.text((774, 642), "Açınım Boyu: 246.85 mm • Yırtılma Boşaltması: 2.0x3.0 mm • Çıktı: DXF R14", font=get_font(9), fill=(180, 220, 200))

    browser_items = [
        ("⊔", "Sac Metal Gövde", True, True),
        ("◫", "Açınım Deseni", True, True),
        ("📐", "K-Faktör Kuralı (0.44)", True, False),
        ("⌍", "Flanş 1 (40 mm)", True, False),
        ("⌍", "Flanş 2 (40 mm)", True, False),
        ("✂️", "Köşe Yırtılma Boşaltması", True, False)
    ]
    draw_browser_sidebar(draw, h, browser_items, is_tr=True)
    draw_bottom_nav(draw, w, h, is_tr=True, active_nav="Sığdır")

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_sheet_metal_unfold_ui.png"), "PNG")
    print("Generated androcad_v28_sheet_metal_unfold_ui.png")

# =========================================================================
# Image 7: Mobile Phone Adaptive UI Layout (768x1376)
# =========================================================================
def generate_phone_ui():
    w, h = 768, 1376
    im = Image.new("RGB", (w, h), BG_DARK)
    draw = ImageDraw.Draw(im)

    # Phone status bar
    draw.rectangle([0, 0, w, 28], fill=(12, 14, 16))
    draw.text((16, 6), "12:30", font=get_font(10, True), fill=TEXT_WHITE)
    draw.text((w - 70, 6), "5G 98% 🔋", font=get_font(10), fill=TEXT_WHITE)

    # Top App Bar
    draw.rectangle([0, 28, w, 82], fill=TOOLBAR_TOP)
    draw_rounded_rect(draw, [14, 38, 44, 68], 6, fill=ACCENT_BLUE)
    draw.text((22, 40), "A", font=get_font(20, True), fill=TEXT_WHITE)
    draw.text((54, 42), "AndroCAD Mobile", font=get_font(17, True), fill=TEXT_WHITE)

    # Language Switcher on Phone
    draw_rounded_rect(draw, [w - 120, 38, w - 14, 70], 6, fill=(35, 45, 60), outline=(0, 164, 228))
    draw.text((w - 108, 46), "🇹🇷 TÜRKÇE", font=get_font(11, True), fill=(100, 210, 255))

    # Scrollable Tabs Row
    draw.rectangle([0, 83, w, 126], fill=TOOLBAR_SUB)
    tabs = ["Katı Model", "Eskiz", "Yüzey", "Sac Metal", "Simülasyon", "İmalat"]
    tx = 14
    for idx, tab in enumerate(tabs):
        is_act = (idx == 0)
        col = ACCENT_BLUE if is_act else TEXT_MUTED
        fnt = get_font(12, is_act)
        draw.text((tx, 95), tab, font=fnt, fill=col)
        if is_act:
            draw.line([tx, 123, tx + 65, 123], fill=ACCENT_BLUE, width=3)
        tx += int(draw.textlength(tab, font=fnt)) + 20

    # Quick Action Ribbon
    tools = [("Kutu", "📦"), ("Silindir", "🛢️"), ("Küre", "⚪"), ("Radyus", "⌒"), ("Pah", "◢"), ("Delik", "🕳️")]
    draw.rectangle([0, 127, w, 172], fill=(28, 30, 34))
    rx = 14
    for name, sym in tools:
        is_sel = (name == "Kutu")
        bg = ACCENT_BLUE if is_sel else CHIP_BG
        draw_rounded_rect(draw, [rx, 134, rx + 80, 165], 6, fill=bg, outline=CHIP_BORDER)
        draw.text((rx + 8, 142), f"{sym} {name}", font=get_font(10), fill=TEXT_WHITE)
        rx += 88

    # 3D Viewport on Phone
    draw_3d_grid(draw, w // 2, 450, span_x=340, span_y=200)
    draw_3d_bracket(draw, w // 2, 430)
    draw_viewcube(draw, w - 124, 185, is_tr=True)

    # Phone Bottom Sheet / Inspector Panel
    sheet_y = 680
    draw_rounded_rect(draw, [0, sheet_y, w, h], 20, fill=(28, 32, 38), outline=BORDER_DARK)
    # Drag handle
    draw_rounded_rect(draw, [w // 2 - 30, sheet_y + 10, w // 2 + 30, sheet_y + 16], 3, fill=(90, 95, 105))

    draw.text((24, sheet_y + 30), "PARAMETRİK PARÇA DENETLEYİCİSİ", font=get_font(14, True), fill=TEXT_WHITE)
    draw.text((24, sheet_y + 54), "Seçili Öğe: Katı Gövde 1 (Braket) • Hacim: 124,850 mm³", font=get_font(11), fill=TEXT_MUTED)

    # Sliders & Input Fields
    params = [
        ("Genişlik (X):", "100.0 mm", 0.7),
        ("Yükseklik (Y):", "50.0 mm", 0.5),
        ("Kalınlık (Z):", "20.0 mm", 0.35),
        ("Radyus:", "3.0 mm", 0.3)
    ]
    py = sheet_y + 90
    for label, val, slider_pct in params:
        draw.text((24, py), label, font=get_font(12, True), fill=TEXT_WHITE)
        draw.text((w - 120, py), val, font=get_font(12, True), fill=(100, 210, 255))
        # Slider track
        draw_rounded_rect(draw, [24, py + 24, w - 24, py + 32], 4, fill=(45, 50, 60))
        # Slider fill
        draw_rounded_rect(draw, [24, py + 24, int(24 + (w - 48) * slider_pct), py + 32], 4, fill=ACCENT_BLUE)
        # Slider knob
        kx = int(24 + (w - 48) * slider_pct)
        draw.ellipse([kx - 8, py + 20, kx + 8, py + 36], fill=TEXT_WHITE)
        py += 62

    # Bottom action buttons
    draw_rounded_rect(draw, [24, h - 85, w // 2 - 10, h - 35], 8, fill=CHIP_BG, outline=CHIP_BORDER)
    draw.text((w // 4 - 30, h - 65), "✕ İPTAL", font=get_font(12, True), fill=TEXT_MUTED)

    draw_rounded_rect(draw, [w // 2 + 10, h - 85, w - 24, h - 35], 8, fill=ACCENT_BLUE)
    draw.text((3 * w // 4 - 45, h - 65), "✓ UYGULA", font=get_font(12, True), fill=TEXT_WHITE)

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_phone_bilingual_ui.png"), "PNG")
    print("Generated androcad_v28_phone_bilingual_ui.png")

# =========================================================================
# Image 8: Bilingual Empty State Screen
# =========================================================================
def generate_empty_state_ui():
    w, h = 1376, 768
    im = Image.new("RGB", (w, h), (18, 20, 24))
    draw = ImageDraw.Draw(im)

    draw_top_bar(draw, w, is_tr=True, active_tab="Katı Model")

    # Center Modal Card
    card_w = 640
    card_h = 560
    cx = (w - card_w) // 2
    cy = 110
    draw_rounded_rect(draw, [cx, cy, cx + card_w, cy + card_h], 14, fill=(28, 32, 38), outline=CHIP_BORDER)

    draw.text((cx + 170, cy + 28), "AndroCAD'e Hoş Geldiniz", font=get_font(20, True), fill=TEXT_WHITE)
    draw.text((cx + 70, cy + 62), "Open CASCADE 8.0.1 ve PlaneGCS çözücülü mobil parametrik 3B CAD platformu.", font=get_font(11), fill=TEXT_MUTED)

    actions = [
        ("🎓 İnteraktif Rehberi Başlat", "Görünüm kontrolü, eskiz çizimi ve 3B ekstrüzyonu 6 basit adımda öğrenin.", (0, 122, 204), (20, 40, 65)),
        ("📦 Örnek Tasarımları Keşfet", "Hazır parametrik modelleri açın ve düzenleyin (L-Braket, Burç, Telefon Standı).", (76, 175, 80), (22, 45, 30)),
        ("📂 Dosya İçe Aktar (STEP / STL)", "SAF üzerinden cihazınızdan veya buluttan CAD parçalarını doğrudan yükleyin.", (230, 81, 0), (55, 30, 15))
    ]

    ay = cy + 110
    for title, desc, border_col, fill_col in actions:
        draw_rounded_rect(draw, [cx + 35, ay, cx + card_w - 35, ay + 72], 8, fill=fill_col, outline=border_col)
        draw.text((cx + 50, ay + 14), title, font=get_font(13, True), fill=border_col)
        draw.text((cx + 50, ay + 40), desc, font=get_font(10), fill=TEXT_MUTED)
        ay += 90

    # Action 4: Default solid block
    draw_rounded_rect(draw, [cx + 35, ay + 10, cx + card_w - 35, ay + 55], 8, fill=(35, 40, 48), outline=CHIP_BORDER)
    draw.text((cx + 120, ay + 24), "Varsayılan Katı Blokla Başla (100x50x20 mm) →", font=get_font(11, True), fill=(100, 210, 255))

    im.save(os.path.join(OUTPUT_DIR, "androcad_v28_empty_state_bilingual_ui.png"), "PNG")
    print("Generated androcad_v28_empty_state_bilingual_ui.png")

if __name__ == "__main__":
    print("Generating modern application screenshots for AndroCAD v28.0.0...")
    generate_tablet_turkish()
    generate_tablet_english()
    generate_sketcher_solver_ui()
    generate_simulation_fea_ui()
    generate_cam_gcode_ui()
    generate_sheet_metal_ui()
    generate_phone_ui()
    generate_empty_state_ui()
    print("All screenshots generated successfully.")
