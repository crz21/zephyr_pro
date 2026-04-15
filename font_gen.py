import sys
import os
from PIL import Image, ImageFont, ImageDraw

def find_chinese_font():
    # 常见的 Ubuntu 中文字体路径列表
    paths = [
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",    # 文泉驿微米黑
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", # Noto Sans CJK
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/droid/DroidSansFallback.ttf",  # 旧版常用
    ]
    for p in paths:
        if os.path.exists(p):
            return p
    return None

def generate_font_hex(text):
    font_path = find_chinese_font()
    
    if not font_path:
        print("错误: 找不到中文字体文件！")
        print("请运行: sudo apt install fonts-wqy-microhei")
        return

    try:
        # 使用 14 像素的字号，在 16x16 的格子内最接近网站效果
        font = ImageFont.truetype(font_path, 14)
    except Exception as e:
        print(f"字体加载失败: {e}")
        return

    for char in text:
        img = Image.new('1', (16, 16), 0)
        draw = ImageDraw.Draw(img)
        
        # 针对 SSD1306 习惯，垂直偏移 -1 使文字居中偏上一点，防止切脚
        draw.text((0, -1), char, font=font, fill=1)

        print(f"\n/*-- 文字: {char} --*/")
        bytes_list = []
        # 逻辑：纵向取模，MSB First (对应你那个对的网站)
        for page in range(2): 
            for x in range(16):
                byte = 0
                for bit in range(8):
                    y_pos = page * 8 + bit
                    if img.getpixel((x, y_pos)):
                        # MSB First
                        byte |= (1 << (7 - bit))
                bytes_list.append(f"0x{byte:02X}")
        
        upper = ", ".join(bytes_list[:16])
        lower = ", ".join(bytes_list[16:])
        print(f"    {upper}, // 上半部")
        print(f"    {lower}, // 下半部")

if __name__ == "__main__":
    target_text = sys.argv[1] if len(sys.argv) > 1 else "组网中"
    generate_font_hex(target_text)
