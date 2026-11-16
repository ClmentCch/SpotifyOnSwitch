#!/usr/bin/env python3
"""Draws the Musictt logo once, so the icon can be regenerated anywhere.

The mark: a rounded square with a violet-to-cyan gradient, a white eighth note
and a play triangle cut into its stem.  Rendered at 4x and downsampled so the
edges are smooth at every size.
"""
from PIL import Image, ImageDraw

S = 4  # supersampling factor


def rounded_gradient(size, radius):
    w = size * S
    grad = Image.new("RGB", (w, w))
    px = grad.load()
    c0 = (124, 92, 255)
    c1 = (53, 214, 196)
    for y in range(w):
        t = y / (w - 1)
        # diagonal blend keeps the gradient alive in a square icon
        for x in range(0, w, 4):
            u = (x / (w - 1) + t) * 0.5
            col = (
                int(c0[0] + (c1[0] - c0[0]) * u),
                int(c0[1] + (c1[1] - c0[1]) * u),
                int(c0[2] + (c1[2] - c0[2]) * u),
            )
            for k in range(4):
                if x + k < w:
                    px[x + k, y] = col
    mask = Image.new("L", (w, w), 0)
    md = ImageDraw.Draw(mask)
    md.rounded_rectangle([0, 0, w - 1, w - 1], radius=radius * S, fill=255)
    out = Image.new("RGBA", (w, w), (0, 0, 0, 0))
    out.paste(grad, (0, 0), mask)
    return out


def note(img, size):
    d = ImageDraw.Draw(img)
    w = size * S
    # eighth note: head, stem, flag
    head_r = int(w * 0.105)
    hx, hy = int(w * 0.38), int(w * 0.70)
    d.ellipse([hx - head_r, hy - int(head_r * 0.78), hx + head_r, hy + int(head_r * 0.78)], fill=(255, 255, 255, 255))
    stem_x = int(w * 0.455)
    d.rounded_rectangle([stem_x, int(w * 0.24), stem_x + int(w * 0.062), int(w * 0.70)], radius=int(w * 0.031), fill=(255, 255, 255, 255))
    d.polygon(
        [
            (stem_x + int(w * 0.05), int(w * 0.235)),
            (int(w * 0.72), int(w * 0.33)),
            (stem_x + int(w * 0.05), int(w * 0.43)),
        ],
        fill=(255, 255, 255, 255),
    )
    # small play triangle inside the counter-space
    d.polygon(
        [
            (int(w * 0.60), int(w * 0.55)),
            (int(w * 0.60), int(w * 0.66)),
            (int(w * 0.685), int(w * 0.605)),
        ],
        fill=(124, 92, 255, 255),
    )


def main():
    big = rounded_gradient(1024, 224)
    note(big, 1024)
    big = big.resize((1024, 1024), Image.LANCZOS)
    big.save("assets/musictt.png")

    icon = rounded_gradient(256, 56)
    note(icon, 256)
    icon = icon.resize((256, 256), Image.LANCZOS)
    icon.convert("RGB").save("assets/icon.jpg", quality=94)
    print("wrote assets/musictt.png and assets/icon.jpg")


if __name__ == "__main__":
    main()
