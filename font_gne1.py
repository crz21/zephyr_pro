import sys
import os
from PIL import Image, ImageFont, ImageDraw

def find_chinese_font():
    paths = [
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
    ]
    for p in paths:
        if os.path.exists(p): return p
    return None

def generate_font_hex(text):
    font_path = find_chinese_font()
    if not font_path: return

    # 使用 12 像素字号，更符合你的 16x16 容器
    font = ImageFont.truetype(font_path, 12)

    for char in text:
        img = Image.new('1', (16, 16), 0)
        draw = ImageDraw.Draw(img)
        # 尝试坐标 (0, 0) 或者 (0, 1) 解决切头问题
        draw.text((0, 0), char, font=font, fill=1)

        bytes_list = []
        
        # 修改点 1：调整 Page 遍历顺序，确保先生成上半部，再生成下半部
        # 修改点 2：位序由 MSB 改为 LSB (或者相反)，以解决“头对底”
        for page in range(2): 
            for x in range(16):
                byte = 0
                for bit in range(8):
                    y_pos = page * 8 + bit
                    if img.getpixel((x, y_pos)):
                        # 改为 LSB First: 每一页的顶端像素对应 bit 0
                        # 如果之前是 (7-bit)，现在改为 (bit) 试试
                        byte |= (1 << bit) 
                bytes_list.append(f"0x{byte:02X}")
        
        print(f"\n/*-- 文字: {char} (已修正位序) --*/")
        upper = ", ".join(bytes_list[:16])
        lower = ", ".join(bytes_list[16:])
        print(f"    {upper}, // 上半部数据")
        print(f"    {lower}, // 下半部数据")

if __name__ == "__main__":
    target = sys.argv[1] if len(sys.argv) > 1 else "组网中"
    generate_font_hex(target)
