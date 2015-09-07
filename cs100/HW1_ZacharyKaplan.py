##Zachary Kaplan
##CS 100 2015F Section H01
##HW 1, Sept 9, 2015

#Utility Methods
def printLi(*args, **kwargs):
	global let
	if 'let' in kwargs:
		let = kwargs['let'];
		del kwargs['let'];
	print('('+let+')', *args, **kwargs);
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


def ex19():
	map(
		lambda xy: printLi(xy[0]**2 + xy[1]**2 < 100),
		[(0, 0), (10, 10), (6, -6), (-7, 8)]
	)


def ex21():
	s = 'rgb'
	print(s, 'goes to', s[::-1])


def grade():
	grades = ['A', 'F', 'C', 'F', 'A', 'B', 'A']
	freq = [grades.count(gr) for gr in ('A', 'B', 'C', 'D', 'F')]
	print (freq)

#Main Routine
if __name__ == '__main__':
	glob = globals()
	for ex in (18, 19, 21):
		print("Example %d:" % ex)
		glob['ex%d' % ex]()
		print()
	print("Grading Example");
	grade();