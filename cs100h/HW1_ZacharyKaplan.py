#!/usr/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 1, Sept 12, 2015

#Utility Methods

#Print List
#Takes any number of args and keyword args and passes them to print
#while also printing out (a) (b) ... (z) before each line
def printLi(*args, **kwargs):
	#global memory storage for the next letter (basically like static in c)
	global let

	#if a keyword argument 'let' exists, use its value to set let, and then delete the entry
	if 'let' in kwargs:
		let = kwargs['let'];
		del kwargs['let'];

	#print the letter followed by the original args and kwargs passed in
	print('('+let+')', *args, **kwargs);

	#increment the letter
	let = chr(ord(let)+1);

def ex18():
	flowers = ['rose', 'bougainvillea', 'yucca', 'marigold', 'daylilly', 'lilly of the valley']
	printLi(flowers, let='a');
	printLi('potato' in flowers);
	thorny = flowers[:3]
	printLi(thorny);
	poisonous = flowers[-1:]
	printLi(poisonous);
	dangerous = thorny + poisonous
	printLi(dangerous);

#Wrong Exercise
#def ex19():
#	map(
#		lambda xy: printLi(xy[0]**2 + xy[1]**2 < 100),
#		[(0, 0), (10, 10), (6, -6), (-7, 8)]
#	)

#Wrong Exercise
#def ex21():
#	s = 'rgb'
#	print(s, 'goes to', s[::-1])

def ex19():
	answers = ['Y', 'N', 'N', 'Y', 'N', 'Y', 'Y', 'Y', 'N', 'N', 'N']

	numYes = answers.count('Y')
	printLi(numYes, let='a')

	numNo = len(answers) - numYes
	printLi(numNo)

	percentYes = numYes / len(answers) * 100
	#formats percent to round to hundreths and end with '%' character
	printLi("%.2f%%" % percentYes)

	answers.sort()
	printLi(answers)

	f = answers.index('Y')
	printLi(f)

def ex21():
	s, t = 'Kaplan', 'Zachary'
	print (t[0] + s[0])

#non-textbook assignment
def grade():
	grades = ['A', 'F', 'C', 'F', 'A', 'B', 'A']

	#list comprehension over tuple of possible grades
	freq = [grades.count(gr) for gr in ('A', 'B', 'C', 'D', 'F')]
	print (freq)

#Main Routine
if __name__ == '__main__':

	#grabs global dictionary (everything on file scope)
	glob = globals()

	#loops over each example to be executed
	for ex in (18, 19, 21):
		#prints the title
		print("Example %d:" % ex)

		#executes the proper method by constructing its name in a string,
		#and then using that as a key to the globals dictionary
		glob['ex%d' % ex]()

		#print a new line
		print()

	#Grading Example, not textbook
	print("Grading Example");
	grade();
