def introduce(s):
    p1, p2 = input('Person 1, what\'s your name? '),\
             input('Person 2, what\'s your name? ')

    print(p1, s, p2)
    print(p2, s, p1)

introduce('meet')
