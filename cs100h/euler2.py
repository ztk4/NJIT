a, b = 0, 1
s = 0

while b <= 4000000:
    a, b = b, a + b
    if not b&1:
        s += b

print(s)
