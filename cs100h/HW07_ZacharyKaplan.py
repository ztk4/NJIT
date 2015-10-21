#!/usr/bin/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 7, Oct 9, 2015

#Problem 1:
#     My Answer: c
#Correct Answer: c

#Problem 2:
#     My Answer: b
#Correct Answer: b

#Problem 3:
#     My Answer: b
#Correct Answer: b

#Problem 4:
#     My Answer: c
#Correct Answer: c

#Problem 5:
#     My Answer: b
#Correct Answer: b

#Problem 6:
#     My Answer: b
#Correct Answer: b

#Problem 7:
#     My Answer: b
#Correct Answer: b

#Problem 8:
#     My Answer: a
#Correct Answer: a

#Problem 9:
#     My Answer: a
#Correct Answer: a

#Problem 10:
#     My Answer: b
#Correct Answer: b

#Problem 11:
#method dashes
def dashes(t, numDashes, dashLength):
    for i in range(numDashes):
        t.pd()
        t.fd(dashLength)
        t.pu()
        t.fd(dashLength)

#test code
def prob11():
    import turtle
    t = turtle.Turtle()
    s = turtle.Screen()
    dashes(t, 10, 20)
    s.bye()
    del turtle

#Problem 12:
#method shortest
def shortest(textList):
    return len(min(textList, key=len))

#test code
def prob12():
    l = 'I am the walrus'.split()
    print(shortest(l))

#Problem 13
#method nameHasLetter
def nameHasLetter(letter):
    name = input("What is your name? ")
    if letter in name:
        print(name, "your name contains at least one", letter)
        return True
    print(name, 'your name contains no', letter)
    return False

#test code
def prob13():
    print(nameHasLetter('a'))

#Main Routine
if __name__ == '__main__':
    prob11()
    prob12()
    prob13()
