import random
import string

# 出力ファイル名
output_file = "random_chars.txt"

# 使用するASCII文字（32～126）
usable_chars = ''.join(chr(i) for i in range(32, 127))

# 文字列の個数（1文字ずつで400万個）
num_chars = 4_000_000

# ランダム文字列を生成
random_string = ''.join(random.choices(usable_chars, k=num_chars))

# ファイルに書き込む
with open(output_file, 'w', encoding='ascii') as f:
    f.write(random_string)
