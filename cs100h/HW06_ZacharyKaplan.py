#!/usr/bin/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 5.2, Oct 2, 2015

#Problem 6:
#     My Answer: a
#Correct Answer: a

#Problem 7:
#     My Answer: e
#Correct Answer: e

#Problem 8:
#     My Answer: d
#Correct Answer: d

#Problem 9:
#     My Answer: b
#Correct Answer: b

#Problem 10:
#     My Answer: d
#Correct Answer: d

#Problem 12:
#method beginsWith
def beginsWith(letter, strList):
    count = 0
    for s in strList:
        if s[:1] == letter: #never raises an error because if s is '', s[:1] is ''
            count += 1
    return count

#test code
def prob12():
    eliza = 'the rain in spain falls mainly on the plain'.split()
    firstLetter = 't'
    print(beginsWith(firstLetter, eliza))

#Problem 13
#method greeting
def greeting(greetStr):
    name, day = input('What is your name? '),\
                input('What day is today? ')
    print(greetStr, day, name)

    print('Your name has', end=' ')
    comp = 'than' #default
    if len(name) == len(day):
        print('the same', end=' ')
        comp = 'as' #if same, can't say then, instead as
    elif len(name) < len(day):
        print('a fewer', end=' ')
    else:
        print('a greater', end=' ')
    print('number of characters', comp, 'today!')

#test code
def prob13():
    greeting('Happy')

#Main Routine
if __name__ == '__main__':
    prob12()
    prob13()
