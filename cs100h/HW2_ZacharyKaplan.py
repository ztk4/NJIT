#!/usr/bin/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 1, Sept 12, 2015

import turtle as tr

def cs2():
	#making a screen allows it to be closed between methods, allowing the screen to "refresh"
	s = tr.Screen()
	t = tr.Turtle()

	for i in range(4):
		t.fd(100)
		t.rt(90)

	#close screen
	s.bye()

def cs3():
	s = tr.Screen()
	t = tr.Turtle()

	t.rt(45)
	for i in range(4):
		t.fd(100)
		t.rt(90)

	#close screen
	s.bye()

def cs4():
	s = tr.Screen()
	t = tr.Turtle()

	#number of sides 5..8
	for i in range(5, 9):
		#degree per vertex
		angle = 360/i

		#draw a side
		for j in range(i):
			t.fd(100)
			t.rt(angle)

	#close screen
	s.bye()

def cs6():
	s = tr.Screen()
	t = tr.Turtle()

	#circle radii
	for r in (10, 20, 60, 80):
		#move to edge of circle (from center)
		t.pu()   #lift pen
		t.rt(90) #face along radius
		t.fd(r)  #move one radius
		t.lt(90) #face along circle

		t.pd()   #drop pen
		t.circle(r)

		#move to center of circle (from edge)
		t.pu()	 #lift pen
		t.lt(90) #face along radius
		t.fd(r)  #move one radius

	#close screen
	s.bye()

#Main Routine
if __name__ == '__main__':

	#grabs global dictionary (everything on file scope)
	glob = globals()

	#loops over each example to be executed
	for cs in (2, 3, 4, 6):
		#prints the title
		print("CS Example %d:" % cs)

		#executes the proper method by constructing its name in a string,
		#and then using that as a key to the globals dictionary
		glob['cs%d' % cs]()

		#print a new line
		print()
