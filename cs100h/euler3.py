from math import sqrt

def get_prime(n):
    prime = [True]*(n+1)
    for i in range(4, n+1, 2):
        prime[i] = False

    lim = int(sqrt(n) + 1)
    for i in range(3, lim, 2):
        if prime[i]:
            for j in range(i + i, n+1, i):
                prime[j] = False

    return prime

def prime(p):
    if p < 2:
        return False
    if p == 2:
        return True
    lim = int(sqrt(p) + 1)
    for i in range(3, lim, 2):
        if not p % i:
            return False
    return True

v = 600851475143

#prime = get_prime(v)

max_p = -1

lim = int(sqrt(v) + 1)

for p in range(3, lim, 2):
    if not v % p:
        n = v/p
        if prime(n):
            max_p = n
            break
        elif prime(p):
            max_p = p

print (max_p)
