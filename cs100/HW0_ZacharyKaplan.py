##Zachary Kaplan
##CS 100 2015F Section H01
##HW 0, Sept 9, 2015

#Utility Methods
def printLi(*args, **kwargs):
	global let
	if 'let' in kwargs:
		let = kwargs['let'];
		del kwargs['let'];
	print('('+let+')', *args, **kwargs);
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

def ex12():
	printLi(sum(range(1, 8)), let='a');
	printLi((65 + 57 + 45)/3);
	printLi(2**20);
	printLi(4356 // 61);
	printLi(4356 % 61);

def ex13():
	s1, s2 = '-', '+'
	printLi(s1 + s2, let='a');
	printLi(s1 + s2 + s1);
	printLi(s2 + s1*2);
	printLi( (s2 + s1*2)*2 );
	printLi( (s2 + s1*2)*10 + s2);
	printLi( (s2 + s1 + s2*3 + s1*2)*5);
	
def ex14():
	s = ''.join([chr(c) for c in range(ord('a'), ord('z')+1)]); #alphabet
	print(*map(repr, 
			[s[0], s[2], s[25], s[24], s[16]]
	));

def ex15():
	s = 'goodbye'
	printLi(s[0] == 'g', let='a')
	printLi(s[6] == 'g')
	printLi(s[:2] == 'ga')
	printLi(s[-2] == 'x')
	printLi(s[len(s)//2] == 'd')
	printLi(s[0] == s[-1])
	printLi(s[-4:] == 'tion')

def ex16():
	a, b = 6, 7
	c = (a + b)/2
	inventory = ['paper', 'staples', 'pencils']
	first, middle, last = 'John', 'Fitzgerald', 'Kennedy'
	fullname = ' '.join([first, middle, last])

#Main Routine
if __name__ == '__main__':
	glob = globals()
	for ex in range(12, 17):
		print("Example %d:" % ex)
		glob['ex%d' % ex]()
		print()