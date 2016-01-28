from turtle import Turtle, Screen
from copy import copy
from math import sqrt

def cb():
    pass

def tri_tree(t, l, d = True):
    '''
    t is the turtle
    l is the side length
    d is True if turning left
    '''
    if l < 3:
        return
    t.pd()
    t.speed(0)
    t.fd(l)

    for i in range(2):
        tri_tree(copy(t), l * .7, not d)
        t.lt(120 if d else -120)
        t.fd(l)
    t.pu()
    return

sqrt2 = sqrt(2)

def square_tree(t, l, c, inc, d = True):

    if l < 3:
        return

    t.pd()
    t.color(c)

    for i in range(4):
        t.fd(l)
        t.lt(90 if d else -90)

    n = l/sqrt2
    cor = t.xcor(), t.ycor()
    head = t.heading()
    col = (c[0], c[1] - inc, c[2] + inc)
    for b in (True, False):
        t.pu()
        t.setx(cor[0])
        t.sety(cor[1])
        t.seth(head)

        t.fd(l/2)
        t.lt(90 if d else -90)
        t.fd(3 * l/2)

        t.rt(135 if b else -135)
        square_tree(t, n, col, inc,b)

if __name__ == '__main__':
    s = Screen()
    t = Turtle()
    s.setworldcoordinates(-400, 0, 600, 1000)
    s.colormode(255)
    t.speed(0)
    t.ht()

    c = (0, 255, 0)
    inc = 20

    s.tracer(0,0)
    square_tree(t, 200, c, inc)
    s.update()
    while True:
        pass
