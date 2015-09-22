#!/usr/bin/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 0, Sept 9, 2015

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

#Integer Declarations
pennies = 100
nickels = 20
quarters = 4

#Float Declarations
penny = 0.01
nickel = 0.05
quarter = 0.25

#String Declarations
denomination1 = 'Penny'
denomination5 = 'Nickel'
denomination25 = 'Quarter'

#Text Book Exercises

#Wrong Exercise
#def ex12():
# 	printLi(sum(range(1, 8)), let='a');
# 	printLi((65 + 57 + 45)/3);
# 	printLi(2**20);
# 	printLi(4356 // 61);
# 	printLi(4356 % 61);

def ex12():
	s1, s2 = '-', '+'
	printLi(s1 + s2, let='a');
	printLi(s1 + s2 + s1);
	printLi(s2 + s1*2);
	printLi( (s2 + s1*2)*2 );
	printLi( (s2 + s1*2)*10 + s2 );
	printLi( (s2 + s1 + s2*3 + s1*2)*5 );

def ex13():
	#creates a list of all characters by looping from ord('a') to ord('z')+1,
	#then joins them on the empty string, creating 'abc...z'
	s = ''.join([chr(c) for c in range(ord('a'), ord('z')+1)]);

	#creates a list by mapping repr to all the characters to print
	#then passes the list as args to print using the unary * operator
	print(*map(repr,
			[s[0], s[2], s[25], s[24], s[16]] #characters to print (answer to question in textbook)
	));

def ex14():
	s = 'goodbye'
	printLi(s[0] == 'g', let='a')
	printLi(s[6] == 'g')
	printLi(s[:2] == 'ga')
	printLi(s[-2] == 'x')
	printLi(s[len(s)//2] == 'd')
	printLi(s[0] == s[-1])
	printLi(s[-4:] == 'tion')

def ex15():
	printLi(len('anachronistically') == len('counterintuitive') + 1, let='a');
	printLi('misinterpretation' < 'misrepresentation');
	printLi('e' not in 'floccinaucinihilipilification');
	printLi(len('counterrevolution') == len('counter') + len('resolution'));

def ex16():
	a, b = 6, 7
	c = (a + b)/2
	inventory = ['paper', 'staples', 'pencils']
	first, middle, last = 'John', 'Fitzgerald', 'Kennedy'

	#joins a list of all the names on space, creating the desired result
	fullname = ' '.join([first, middle, last])

#Main Routine
if __name__ == '__main__':

	#grabs global dictionary (everything on file scope)
	glob = globals()

	#loops over each example to be executed
	for ex in range(12, 17):
		#prints the title
		print("Example %d:" % ex)

		#executes the proper method by constructing its name in a string,
		#and then using that as a key to the globals dictionary
		glob['ex%d' % ex]()

		#print a new line
		print()
