# Structure Images

Place structure images here. They will be automatically loaded at game startup and converted into structure blueprints.

## How it works

- Each **pixel** in the image becomes one **cell** (particle) in the structure grid.
- **Transparent pixels** (alpha < 10) become empty space.
- Images are **auto-cropped** to the bounding box of non-transparent content. So a 1800x900px image with a 30x50px drawing becomes a 30x50 cell structure.
- The filename (without extension) becomes the structure name.

## Color mapping

Pixel colors are matched to particle types:

| Color (RGB)     | Particle Type |
| --------------- | ------------- |
| (194, 178, 128) | Sand          |
| (30, 144, 255)  | Water         |
| (50, 50, 50)    | Smoke         |
| (128, 128, 128) | Stone         |
| (60, 255, 73)   | Uranium       |

Colors within a distance of 50 units from these values are matched automatically.

**Any other color** will create a solid (stone-like) particle with that exact color from the image. This lets you use any color you want for custom-looking structures.

## Supported formats

- `.png` (recommended - supports transparency)
- `.jpg` / `.jpeg`
- `.bmp`
- `.tga`

## Tips

- Use **PNG with transparency** for best results (transparent = empty space).
- Draw at 1:1 scale — each pixel = one particle cell.
- Use the exact RGB values above if you want specific particle types.
- Use any other color for decorative solid blocks.
