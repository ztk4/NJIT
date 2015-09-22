def hello(name):
    return "Personalized message for %s." % name

def oddCount(l):
    c = 0
    for i in l:
        if i % 2:
            c += 1

    return c

#name = input("What's your name? ")
#print(hello(name))

l = [12, 21, 23, 43, 45, 56, 78, 89, 6, 43, 23]
print(oddCount(l))
