#!/usr/bin/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 1, Sept 12, 2015

import turtle as tr

#Exercise 1 from Word Doc
def num1():
	a, b, c = 3, 4, 5
	if a < b:
		print('OK')

	if c < b:
		print('OK')

	if a + b == c:
		print('OK')

	if a*a + b*b == c*c:
		print('OK')

#Exercise 2 from Word Doc
def num2():
	a, b, c = 3, 4, 5
	if a < b:
		print('OK')
	else:
		print('NOT OK')

	if c < b:
		print('OK')
	else:
		print('NOT OK')

	if a + b == c:
		print('OK')
	else:
		print('NOT OK')

	if a*a + b*b == c*c:
		print('OK')
	else:
		print('NOT OK')

#Exercise 3 from Word Doc
def num3():
	color = input('Enter a Color: ')
	line_w = int(input('Enter a Line Width: '))
	line_l = int(input('Enter a Line Length: '))
	shape = input('Enter a Shape (either Line, Triangle, or Square): ').lower()

	s = tr.Screen()
	t = tr.Turtle()

	t.color(color)
	t.width(line_w)

	if shape == 'line':
		t.fd(line_l)
	elif shape == 'triangle':
		for i in range(3):
			t.fd(line_l)
			t.rt(120)
	elif shape == 'square':
		for i in range(4):
			t.fd(line_l)
			t.rt(90)

	s.bye()

#Main Routine
if __name__ == '__main__':

	#grabs global dictionary (everything on file scope)
	glob = globals()

	#loops over each example to be executed
	for num in range(1, 4):
		#prints the title
		print("Word Doc Example %d:" % num)

		#executes the proper method by constructing its name in a string,
		#and then using that as a key to the globals dictionary
		glob['num%d' % num]()

		#print a new line
		print()
