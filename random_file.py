import random
import string

output_file = "100MB_random_chars.txt"

usable_chars = ''.join(chr(i) for i in range(32, 127))

num_chars = 103532273

random_string = ''.join(random.choices(usable_chars, k=num_chars))

with open(output_file, 'w', encoding='ascii') as f:
    f.write(random_string)
