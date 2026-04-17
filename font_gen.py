import sys
import os
from PIL import Image, ImageFont, ImageDraw

def generate_full_font(char):
    # 尝试寻找更厚重的字体
    font_paths = [
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc", # 文泉驿正黑（比微米黑粗）
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc", # 粗体
    ]
#font_path = "/usr/share/fonts/windows/simsun.ttc"
    font_path = next((p for p in font_paths if os.path.exists(p)), None)
    
    # 1. 在一个较大的画布上渲染，以便裁剪
    temp_img = Image.new('1', (32, 32), 0)
    draw = ImageDraw.Draw(temp_img)
    try:
        # 使用较大的字号来获得更多细节
        font = ImageFont.truetype(font_path, 16)
    except:
        font = ImageFont.load_default()

    draw.text((0, 0), char, font=font, fill=1)

    # 2. 自动裁剪掉多余的白边
    bbox = temp_img.getbbox()
    if bbox:
        char_img = temp_img.crop(bbox)
        # 3. 强制拉伸到 16x16，使其撑满屏幕
        char_img = char_img.resize((14, 15), Image.NEAREST) # 留 1-2 像素间隙防止粘连
        final_img = Image.new('1', (16, 16), 0)
        final_img.paste(char_img, (1, 0)) # 居中放置
    else:
        final_img = temp_img.resize((16, 16))

    # 4. 按照你的驱动逻辑生成点阵 (纵向取模, LSB First)
    bytes_list = []
    for page in range(2): 
        for x in range(16):
            byte = 0
            for bit in range(8):
                y_pos = page * 8 + bit
                if final_img.getpixel((x, y_pos)):
                    byte |= (1 << bit) # LSB First
            bytes_list.append(f"0x{byte:02X}")
    
    print(f"\n/*-- 饱满型文字: {char} --*/")
    print(f"    {', '.join(bytes_list[:16])}, // 上")
    print(f"    {', '.join(bytes_list[16:])}, // 下")

if __name__ == "__main__":
    word = sys.argv[1] 
    generate_full_font(word)
