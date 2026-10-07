#!/usr/bin/env python3
"""Render *simulated* previews of the two applets (250x122 E-Ink layout).

The clock digits use the same integer arithmetic and segment geometry as ClockApplet.cpp.
Text lines use a generic sans font, so glyph shapes differ from the device's FreeSans bitmap fonts.
These images are illustrations, not photographs.   Usage:  python3 docs/render_preview.py [path/to/firmware/src/graphics/niche/Fonts]
"""
import os, re, sys, urllib.request
from PIL import Image, ImageDraw, ImageFont

W, H = 250, 122
PAPER, INK = (222, 221, 214), (18, 18, 18)
SEG_W, SEG_H = 16, 4
SEGMENTS = [[1,1,1,1,1,1,0],[0,1,1,0,0,0,0],[1,1,0,1,1,0,1],[1,1,1,1,0,0,1],[0,1,1,0,0,1,1],
            [1,0,1,1,0,1,1],[1,0,1,1,1,1,1],[1,1,1,0,0,1,0],[1,1,1,1,1,1,1],[1,1,1,1,0,1,1]]


def font(size):
    for p in ("/System/Library/Fonts/Supplemental/Arial.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
              "C:/Windows/Fonts/arial.ttf"):
        if os.path.exists(p):
            return ImageFont.truetype(p, size)
    return ImageFont.load_default()


def rect(d, x, y, w, h):
    d.rectangle([x, y, x + w - 1, y + h - 1], fill=INK)


def tri(d, x0, y0, x1, y1, x2, y2):
    d.polygon([(x0, y0), (x1, y1), (x2, y2)], fill=INK)


def hseg(d, x, y, w, h):
    half = h // 2
    rect(d, x, y, w, h); tri(d, x, y, x, y + h - 1, x - half, y + half); tri(d, x + w, y, x + w + half, y + half, x + w, y + h - 1)


def vseg(d, x, y, w, h):
    half = h // 2
    rect(d, x, y, h, w); tri(d, x + half, y - half, x + h - 1, y, x, y); tri(d, x, y + w, x + h - 1, y + w, x + half, y + w + half)


def digit(d, x, y, n, s):
    sw, sh = int(SEG_W * s), int(SEG_H * s); seg = SEGMENTS[n]
    x1, y1 = x + sh + 2, y; x2, y2 = x1 + sw + 2, y1 + sh + 2
    y3 = y2 + sw + 2 + sh + 2; y4 = y3 + sw + 2; y7 = y2 + sw + 2
    if seg[0]: hseg(d, x1, y1, sw, sh)
    if seg[1]: vseg(d, x2, y2, sw, sh)
    if seg[2]: vseg(d, x2, y3, sw, sh)
    if seg[3]: hseg(d, x1, y4, sw, sh)
    if seg[4]: vseg(d, x, y3, sw, sh)
    if seg[5]: vseg(d, x, y2, sw, sh)
    if seg[6]: hseg(d, x1, y7, sw, sh)


def clock(text="20:48", date="Thu 08.10.2026"):
    img = Image.new("RGB", (W, H), PAPER); d = ImageDraw.Draw(img)
    top_reserve, date_h = 18, 22
    area_h = H - date_h - 8 - top_reserve
    def width(s):
        sw, sh = int(SEG_W * s), int(SEG_H * s)
        tot = sum((sh + 6 + (int(4.5 * s) if s >= 2 else 0) + 5) if c == ":" else (sw + sh * 2 + 4 + 5) for c in text)
        return tot - 5 + sh // 2
    cell = lambda s: int(SEG_W * s) * 2 + int(SEG_H * s) * 3 + 8
    s = 0.5
    while s < 3.5 and width(s + 0.05) <= W - 12 and cell(s + 0.05) <= area_h:
        s += 0.05
    sw, sh = int(SEG_W * s), int(SEG_H * s)
    x = (W - width(s)) // 2 + sh // 2; y = top_reserve + (area_h - cell(s)) // 2 + 2
    for c in text:
        if c == ":":
            cs = sw * 2 + sh * 3 + 8; dx = x + int(4 * s)
            rect(d, dx, y + cs // 4, sh, sh); rect(d, dx, y + (cs // 4) * 3, sh, sh)
            x += sh + 6 + (int(4.5 * s) if s >= 2 else 0)
        else:
            digit(d, x, y, int(c), s); x += sw + sh * 2 + 4
        x += 5
    f = font(15); tw = d.textlength(date, font=f)
    d.text(((W - tw) / 2, H - 21), date, font=f, fill=INK)
    d.rectangle([W - 34, 3, W - 8, 13], outline=INK, width=1); d.rectangle([W - 33, 4, W - 14, 12], fill=INK)  # battery icon hint
    d.rectangle([W - 7, 6, W - 6, 10], fill=INK)
    return img


def status():
    img = Image.new("RGB", (W, H), PAPER); d = ImageDraw.Draw(img); f = font(14)
    lines = ["Batt: 62%  25.10V USB", "Nodes: 12 online / 187", "Airtime: 11.5%  TX 1.1%", "Uptime: 5h 12m", "Free heap: 182 KB"]
    for i, t in enumerate(lines):
        d.text((4, 3 + i * 21), t, font=f, fill=INK)
    d.rectangle([W - 34, 3, W - 8, 13], outline=INK, width=1); d.rectangle([W - 33, 4, W - 14, 12], fill=INK)
    d.rectangle([W - 7, 6, W - 6, 10], fill=INK)
    return img


def frame(img, scale=3):
    big = img.resize((W * scale, H * scale), Image.NEAREST)
    out = Image.new("RGB", (W * scale + 24, H * scale + 24), (40, 42, 46)); out.paste(big, (12, 12)); return out


# ---------------------------------------------------------------------------------------------
# Messages screen: uses the REAL InkHUD bitmap fonts (Adafruit GFX tables) so Cyrillic letters and
# emoji look exactly like on the device. Only the screen layout is simulated.
# ---------------------------------------------------------------------------------------------
FW_TAG = "v2.7.26.54e0d8d"
FONT_BASE = "https://raw.githubusercontent.com/meshtastic/firmware/%s/src/graphics/niche/Fonts/" % FW_TAG
EMOJI = {  # same mapping idea as InkHUD's AppletFont.cpp (emoji -> control-character glyph slots)
    "\U0001F44D": 0x01, "\U0001F60A": 0x03, "\U0001F602": 0x04, "\U0001F44B": 0x05, "☀": 0x06,
    "❤": 0x0B, "\U0001F389": 0x12, "\U0001F525": 0x16, "\U0001F914": 0x1D, "\U0001F44C": 0x1F,
}


def load_font(name, fonts_dir=None):
    fn = "%s.h" % name
    text = None
    if fonts_dir and os.path.exists(os.path.join(fonts_dir, fn)):
        text = open(os.path.join(fonts_dir, fn)).read()
    else:
        text = urllib.request.urlopen(FONT_BASE + fn, timeout=30).read().decode()
    body = re.search(r"Bitmaps\[\] PROGMEM = \{(.*?)\};", text, re.S).group(1)
    bitmaps = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", re.sub(r"/\*.*?\*/", "", body)))
    gl = re.search(r"Glyphs\[\] PROGMEM = \{(.*?)\};", text, re.S).group(1)
    glyphs = [tuple(int(v) for v in m) for m in re.findall(r"\{\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\}", gl)]
    tail = re.search(r"0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+),\s*(\d+)\s*\};\s*$", text.strip())
    return {"bitmaps": bitmaps, "glyphs": glyphs, "first": int(tail.group(1), 16), "yadv": int(tail.group(3))}


def encode(text):
    out = []
    for ch in text:
        if ch in EMOJI: out.append(EMOJI[ch])
        elif ch == "️": continue
        else: out.append(ch.encode("cp1251", "replace")[0])
    return out


def text_width(font, text):
    return sum(font["glyphs"][c - font["first"]][3] for c in encode(text))


def draw_text(img, font, x, baseline, text, ink=INK):
    px = img.load()
    for c in encode(text):
        off, w, h, adv, xo, yo = font["glyphs"][c - font["first"]]
        bit = 0
        for yy in range(h):
            for xx in range(w):
                byte = font["bitmaps"][off + bit // 8]
                if byte & (0x80 >> (bit % 8)):
                    X, Y = x + xo + xx, baseline + yo + yy
                    if 0 <= X < img.width and 0 <= Y < img.height: px[X, Y] = ink
                bit += 1
        x += adv


def messages(fonts_dir=None):
    big, mid, small = (load_font(n, fonts_dir) for n in ("FreeSans12pt_Win1251", "FreeSans9pt_Win1251", "FreeSans6pt_Win1251"))
    img = Image.new("RGB", (W, H), PAPER); d = ImageDraw.Draw(img)
    d.rectangle([0, 0, W - 1, 19], fill=INK)                       # header bar
    draw_text(img, mid, 5, 15, "Місцевий чат", ink=PAPER)           # "Local chat"
    d.rectangle([W - 34, 5, W - 8, 14], outline=PAPER, width=1); d.rectangle([W - 32, 7, W - 17, 12], fill=PAPER)
    rows = [("ODSA · 2 хв", "Привіт! Чути добре \U0001F44D"),
            ("KYIV · 5 хв", "Дякую, все працює ☀\U0001F60A"),
            ("LVIV · 9 хв", "Хто на зв'язку сьогодні? \U0001F525")]
    y = 31
    for who, msg in rows:
        draw_text(img, small, 5, y, who)
        draw_text(img, mid, 5, y + 17, msg)
        y += 33
    return img


if __name__ == "__main__":
    here = os.path.dirname(os.path.abspath(__file__)); os.makedirs(os.path.join(here, "img"), exist_ok=True)
    frame(clock()).save(os.path.join(here, "img", "clock.png"))
    frame(status()).save(os.path.join(here, "img", "status.png"))
    fonts_dir = sys.argv[1] if len(sys.argv) > 1 else None   # optional: path to src/graphics/niche/Fonts of a checkout
    frame(messages(fonts_dir)).save(os.path.join(here, "img", "messages.png"))
    print("saved docs/img/clock.png, status.png and messages.png")
