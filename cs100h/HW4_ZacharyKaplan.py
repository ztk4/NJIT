#!/bin/python3

##Zachary Kaplan
##CS 100 2015F Section H03
##HW 4, Sept 22, 2015

#Utility Methods

#Print List Item
#Takes any number of args and keyword args and passes them to print
#while also printing out (a) (b) ... (z) before each line
def printLi(*args, **kwargs):
	#global memory storage for the next letter (basically like static in c)
	global let

	#if a keyword argument 'let' exists, use its value to set let,
	#and then delete the entry
	if 'let' in kwargs:
		let = kwargs['let'];
		del kwargs['let'];

	#print the letter followed by the original args and kwargs passed in
	print('('+let+')', *args, **kwargs);

	#increment the letter
	let = chr(ord(let)+1);

def ex20():
    lst = ['January', 'February', 'March']

    for s in lst:
        print(s[:3])

    #for fun:
    #print(*[s[:3] for s in lst], sep='\n')

def ex21():
    lst = list(range(2, 10))

    for num in lst:
        if not num % 2:
            print(num)

    #for fun:
    #print(*[num for num in lst if not num % 2], sep='\n')

def ex22():
    lst = list(range(2, 10))

    for num in lst:
        if not (num*num % 8):
            print(num)

    #NOTE:
    #if n^2 is divisible by 8
    #then n^2 has at least 2^3 in its prime factorization
    #therefore n has at least 2^(ceil(1.5)) = 2^2 in its prime factorization
    #and so if n is divisible by 4, then n^2 is divisible by 8
    #ie: n**2 % 8 == 0 <==> n % 4 == 0

    #for fun
    #print(*[num for num in lst if not num % 4], sep='\n')

def ex23():
    printLi(*range(2), let='a')
    printLi(*range(1))
    printLi(*range(3, 7))
    printLi(*range(1, 2))
    printLi(*range(0, 4, 3))
    printLi(*range(5, 22, 4))

#Main Routine
if __name__ == '__main__':

	#grabs global dictionary (everything on file scope)
	glob = globals()

	#loops over each example to be executed
	for ex in range(20, 24):
		#prints the title
		print("Book Example %d:" % ex)

		#executes the proper method by constructing its name in a string,
		#and then using that as a key to the globals dictionary
		glob['ex%d' % ex]()

		#print a new line
		print()
