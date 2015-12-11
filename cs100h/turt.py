#!/usr/bin/python3

import turtle as tr
from multiprocessing import Pool

t1, t2, t3, t4 = [tr.Turtle() for i in range(4)]

t2.rt(90)
t3.rt(180)
t4.rt(270)

def move(t, d):
    return t, d

p = Pool(4)

for t in (t1, t2, t3, t4):
    p.apply_async(move, (t, 100), callback=lambda arg: print(arg))

p.close()
p.join()
