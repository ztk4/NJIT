/*
 * Zachary Kaplan
 * ztk4 - 31353570
 * CS 114-H01
 * Project 2 - Kinght's Tour
 * 10/31/15
 */

import java.util.Iterator;
import java.lang.UnsupportedOperationException;

/**
 * Solves the Knight's Tour Problem.
 * Given a optional board size and starting position, finds a solution to the Knight's Tour problem.
 *
 * @author 		Zachary Kaplan
 * @version 	1.0
 */
public class Knight {
	
	/**
	 * Private helper class that stores Tile information in Object, allowing for iteration and simple utility methods.
	 * Instances of this class can iterate over adjacent square that a knight can travel to in a foreach loop.
	 * Additionally, this class offers some utility methods for marking and unmarking the board.
	 *
	 * @author Zachary Kaplan
	 * @version 1.0
	 */
	private static class Tile implements Iterable<Tile> {
		
		public Tile(int row, int col) {
			r = row;
			c = col;
		}
		
		public Iterator<Tile> iterator() {
			return new Iterator<Tile>() {
				
				/**
				 * Prepares the state of the iterator for the next object.
				 * Must be called after creation of object and after each call to next
				 * prior to any calls to next or hasNext
				 */
				private void prep() {
					int r = -1, c = -1;
					while(r < 0 || c < 0 || r >= size || c >= size) {
						if(pos >= roffs.length) {
							done = true;
							return;
						}
					 	r = row + roffs[pos];
						c = col + coffs[pos];
						++pos;
					}
					
					ready = true;
					curr = new Tile(r, c);
				}
	
				public Tile next() { //return Tile object and prep for next one
					if(!ready)
						prep();
					Tile tmp = curr;
					ready = false;
					return tmp;
				}

				public boolean hasNext() { //check if all 8 spaces have been exhausted
					if(!ready)
						prep();

					return !done;
				}

				public void remove() {
					throw new UnsupportedOperationException();
				}

				private int row = r,			//local storage for base r
							col = c,			//local storage for base c
							pos = 0;			//position in arrs, 0-7
				private Tile curr;				//current Tile in iteration
				private boolean ready = false,	//flag to indicate if prepared for next
								done = false;	//flag to indicate finished
			};
		}

		public void setNum(int num) {
			board[r][c] = num;
		}

		public int getNum() {
			return board[r][c];
		}
		
		public String toString() {
			return "(" + r + ", " + c + ")";
		}

		private int r, c; //local storage for row and col
	}
	
	/**
	 * Given the initial conditions, attempts to find a single tour using brute-force.
	 * 
	 * @param n		Move number (starts as 1)
	 * @param t		Tile object representing position of Knight
	 * @return 		true if a tour was found,
	 * 				false if no path was found
	 */
	private static boolean bruteForce(int n, Tile t) {
		t.setNum(n); //mark tile as visited

		if(n == max) //done, found a tour
			return true;
		//System.out.println(n);	
		for(Tile next : t) {
			if(next.getNum() == 0) { //if not visited
				if(bruteForce(n + 1, next)) //tour found later after this move
					return true;
			}
		}

		t.setNum(0); //can't move to any tiles, so undo this move
		return false;
	}
	
	private static void printBoard() {
		int width = 1, tmp = max;
		while((tmp /= 10) > 0) 
			++width;
		String format = "%" + width + "d "; //how is there no variable width in string formatting...

		for(int r = 0; r < size; ++r) {
			for(int c = 0; c < size; ++c) {
				System.out.print(String.format(format, board[r][c]));
			}
			System.out.println();
		}
	}

	/**
	 * Main Routine.
	 * Takes CLI input and runs the program
	 *
	 * @param args	String[] with CLI input	<br />
	 * 				args[0] - board size	<br />
	 * 				args[1] - starting row	<br />
	 * 				args[2] - starting col	<br />
	 * 				args[3] - b for brute-force, w for Warnsdorf
	 */
	public static void main(String[] args) {
		switch(args.length) {
			case 4:
				b = args[3].charAt(0) == 'b';
			case 3:
				irow = Integer.parseInt(args[1]);
				icol = Integer.parseInt(args[2]);
			case 1:
				size = Integer.parseInt(args[0]);
			case 0:
				break;
			default:
				System.err.println(help);
				System.exit(1);
		}

		max = size * size;
		board = new int[size][size];

		if(b) 
			bruteForce(1, new Tile(irow, icol));
		else
			;//TODO: Warnsdorf

		printBoard();
	}

	private static int 	size = 8,		//length of one side of square board
						irow = 0,		//initial row
		   				icol = 0,		//initial column
						max,			//number of square on board
						board[][];		//board, stores move numbers
	
	private static boolean b = true;	//if true, then brute force, else Warnsdorf

	//only used by iterator class, however static members cannot be inside of static inner classes
	private static final int[]	roffs = {2, 2, 1, 1, -1, -1, -2, -2},	//row offsets
								coffs = {1, -1, 2, -2, 2, -2, 1, -1};	//column offsets

	private static String help = "Usage: java Knight [board_size [starting_row starting_column]]";
}
