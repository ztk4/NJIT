#!/bin/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 4, Sept 25, 2015

#Problem 1:
#     My Answer: d
#Correct Answer: d

#Problem 2:
#     My Answer: a
#Correct Answer: a

#Problem 3:
#     My Answer: c
#Correct Answer: c

#Problem 4:
#     My Answer: e
#Correct Answer: e

#Problem 5:
#     My Answer: c
#Correct Answer: c

#Problem 11:
#method drawTick (part a)
def drawTick(t, tickLen):
    t.pd()
    t.rt(90)

    t.fd(tickLen/2) #half tick up
    t.bk(tickLen)   #all the way down
    t.fd(tickLen/2) #half tick up to original position

    t.lt(90)

#method drawTicks (part b)
def drawTicks(t, tickLen, numTicks, distance):
    for i in range(numTicks):
        drawTick(t, tickLen)
        t.pu()
        t.fd(distance)

#test code
def prob11():
    import turtle
    s = turtle.Screen()
    turt = turtle.Turtle()
    drawTicks(turt, 5, 10, 15)

#Main Routine
if __name__ == '__main__':
    prob11()
