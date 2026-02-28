import os
from PIL import Image

def process_image_color(img):
    img = img.convert("RGBA")
    data = img.get_flattened_data() if hasattr(img, 'get_flattened_data') else img.getdata()
    new_data = []
    
    for r, g, b, a in data:
        # Detect reddish pixels: Red dominantly higher than green and blue
        if a > 0 and r > 100 and r > sum([g, b]) * 0.7:
            # Change red to light blue (cyan-like MySQL color)
            # R goes down, G and B take the intensity
            new_r = min(255, int(r * 0.1 + g)) 
            new_g = min(255, int(r * 0.7 + g))
            new_b = min(255, int(r * 1.0 + b))
            new_data.append((new_r, new_g, new_b, a))
        else:
            new_data.append((r, g, b, a))
            
    img.putdata(new_data)
    return img

def convert_ico(filepath):
    print(f"Opening: {filepath}")
    
    # Backup original before modifying
    backup_path = filepath + ".bak"
    if not os.path.exists(backup_path):
        import shutil
        shutil.copy2(filepath, backup_path)
        print(f"Backed up to {backup_path}")

    img = Image.open(backup_path) # Read from backup to make sure we get original
    sizes = list(img.info.get('sizes', [img.size]))
    print(f"Detected sizes: {sizes}")
    
    frames = []
    for size in sizes:
        tmp_img = Image.open(backup_path)
        tmp_img.size = size
        tmp_img.load()
        
        new_frame = process_image_color(tmp_img)
        frames.append(new_frame)
    
    if len(frames) > 0:
        main_img = frames[0]
        append_imgs = frames[1:]
        
        main_img.save(filepath, format="ICO", sizes=sizes, append_images=append_imgs)
        print(f"Saved: {filepath}")

ico_paths = [
    r"d:\data\github\msqledit_msqlgrid\src\tools\mysql\msqlgrid\res\ogrid.ico",
    r"d:\data\github\msqledit_msqlgrid\src\tools\mysql\msqlgrid\res\msqlgridDoc.ico"
]

for p in ico_paths:
    if os.path.exists(p):
        convert_ico(p)
    else:
        print(f"Missing: {p}")
