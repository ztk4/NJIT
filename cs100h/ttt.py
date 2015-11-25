#!/usr/bin/python3

import turtle as tr

turn = 1 #x's turn
board = [[0]*3 for i in range(3)]

#checks for win state
def checkWins():
    d1, d2 = board[0][0], board[0][2]
    for i in range(3):
        h, v = board[i][i], board[i][i]

        for j in range(3):
            if board[j][i] != h:
                h = 0
            if board[i][j] != v:
                v = 0

        if h:
            return h
        if v:
            return v

        if board[i][i] != d1:
            d1 = 0
        if board[i][2 - i] != d2:
            d2 = 0

    return d1 if d1 else d2

#grabs info about screen and resizes
def resize():
    global w, h, l, sl, woff, hoff

    t.reset()

    w, h = s.canvwidth, s.canvheight
    s.setworldcoordinates(0, 0, w, h)


    l = min(w, h) - 20
    woff, hoff = (w - l) / 2, (h - l) / 2
    sl = l/3

    drawBoard()
    s.update()

#fill canvas with board, leave small border
def drawBoard():
    t.pu()
    t.seth(0)
    t.goto(woff, hoff)
    t.seth(0)

    for i in range(3):
        t.pd()
        for j in range(3):
            for k in range(4):
                t.fd(sl)
                t.lt(90)
            t.fd(sl)

        t.pu()
        t.setx(woff)
        t.sety(t.ycor() + sl)

    for x in range(3):
        for y in range(3):
            if board[x][y] != 0:
                drawX(x, y) if board[x][y] > 0 else drawO(x, y)

#draw an x at a position
def drawX(x, y):
    x0, y0 = x*sl + woff + 5, y * sl + hoff + 5
    xf, yf = x0 + sl - 10, y0 + sl - 10

    t.pu()
    t.setpos(x0, y0)
    t.pd()
    t.setpos(xf, yf)

    t.pu()
    t.setpos(x0, yf)
    t.pd()
    t.setpos(xf, y0)

def drawO(x, y):
    t.pu()
    t.setpos(woff + x*sl + sl/2, hoff + y*sl + 5)

    t.pd()
    t.seth(0)
    t.circle(sl/2 - 5)

#makes move based on 0-8 input,
def makeMove(x, y):
    global turn

    if board[x][y] != 0:
        return #can't make move

    board[x][y] = turn
    drawX(x, y) if turn > 0 else drawO(x, y)

    if checkWins():
        t.pu()
        t.goto(w/2, h/2)
        t.pd()
        t.write(('X' if turn > 0 else 'O') + " wins!", align='center', font=('Arial', 20, 'normal'))
        s.onclick(lambda x, y: exit(0))

    turn = -turn
    s.update()

#process click event
def clicked(x, y):
    x = int((x - woff) // sl)
    y = int((y - hoff) // sl)

    if x < 0 or y < 0 or x >= 3 or y >= 3:
        return #no processing to be done

    makeMove(x, y)

def playgame():
    global t, s

    t = tr.Turtle()
    s = t.getscreen() #makes sure turtle exists
    t.speed(0)
    s.tracer(0)

    resize()
    s.onscreenclick(clicked)
    s._root.bind("<Configure>", lambda *args: resize(), True)

    tr.mainloop()

if __name__ == '__main__':
    playgame()
