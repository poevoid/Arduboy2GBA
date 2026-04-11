import os

def _pages_for_height(h):
    return (h + 7) // 8

def estimate_bitmap_metadata(bitmaps):
    metadata = []

    for bmp in bitmaps:
        name = bmp["name"]
        values = [int(x, 0) for x in bmp["data"]]
        metadata.append({
            "name": name,
            "byte_count": len(values),
        })

    return metadata

def convert_bitmaps(bitmaps, out_dir):
    """
    Arduboy bitmaps and sprites are now consumed directly by the runtime in
    their native PROGMEM layout, so we do not expand them into raw .bin files.

    We keep this function so the rest of the toolchain still has a bitmap step,
    but it now just writes a small metadata report for inspection.
    """
    os.makedirs(out_dir, exist_ok=True)

    metadata = estimate_bitmap_metadata(bitmaps)
    report_path = os.path.join(out_dir, "bitmap_report.txt")

    with open(report_path, "w", encoding="utf-8") as f:
        for item in metadata:
            f.write(f'{item["name"]}: {item["byte_count"]} bytes\n')

    return metadata
