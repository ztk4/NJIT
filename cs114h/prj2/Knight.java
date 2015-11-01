/*
 * Zachary Kaplan
 * ztk4 - 31353570
 * CS 114-H01
 * Project 2 - Kinght's Tour
 * 10/31/15
 */

/**
 * Solves the Knight's Tour Problem.
 * Given a optional board size and starting position, finds a solution to the Knight's Tour problem.
 *
 * @author 		Zachary Kaplan
 * @version 	1.0
 */
public class Knight {
	
	/**
	 * Given the initial conditions, attempts to find a single tour using brute-force.
	 * 
	 * @param n		Move number (starts as 1)
	 * @param r		Row number
	 * @param c		Column Number
	 * @return 		true if a tour was found,
	 * 				false if no path was found
	 */
	private static boolean bruteForce(int n, int r, int c) {
		board[r][c] = n; //mark tile as visited

		if(n == max) //done, found a tour
			return true;

		for(int i = 0; i < roffs.length; ++i) {
			int rp = r + roffs[i], cp = c + coffs[i]; //row prime and column prime

			if(rp >= 0 && cp >= 0 && rp < size && cp < size && board[rp][cp] == 0) { //if not visited
				if(bruteForce(n + 1, rp, cp)) //tour found later after this move
					return true;
			}
		}

		board[r][c] = 0; //can't move to any tiles, so undo this move
		return false;
	}

	/**
	 * Given the initial conditions, attempts to find a single tour using Warnsdorf's Hueristic.
	 *
	 * @param n		Move number (starts at 1)
	 * @param r		Row number
	 * @param c		Column number
	 * @return		true if a tour was found,
	 * 				false if no path was found
	 */
	private static boolean warnsdorf(int n, int r, int c) {
		board[r][c] = n; //mark tile as visited

		if(n == max) //done, found a tour
			return true;

		int best = 9,			 					//best can be at worst 8, so 9 is essentially weight infinity
			rp[] = new int[8], cp[] = new int[8],	//row prime and column prime (move candidates)
			access[] = new int[8];					//access for move

		for(int i = 0; i < roffs.length; ++i) {
			rp[i] = r + roffs[i];
			cp[i] = c + coffs[i]; //row prime and column prime

			if(rp[i] >= 0 && cp[i] >= 0 && rp[i] < size && cp[i] < size && board[rp[i]][cp[i]] == 0) { //if not visited
				for(int j = 0; j < roffs.length; ++j) {
					int rpp = rp[i] + roffs[j],
						cpp = cp[i] + coffs[j]; //row prime prime and column prime prime

					if(rpp >= 0 && cpp >= 0 && rpp < size && cpp < size && board[rpp][cpp] == 0) //if not visited
						++access[i];						
				}
				if(access[i] < best)
						best = access[i]; //keep track of best
			} else
				access[i] = -1; //invalid move (best will never be -1)
		}
		
		//recursive backtracking for naive tie handling (I don't think this is necessary, but just in case)
		for(int i = 0; i < access.length; ++i)
			if(access[i] == best && warnsdorf(n + 1, rp[i], cp[i]))
					return true;

		board[r][c] = 0;
		return false; //no solution
	}
	/**
	 * Prints out board to console.
	 */
	private static void printBoard() {
		int width = 1, tmp = max;
		while((tmp /= 10) > 0) 
			++width;
		String format = "%" + width + "d "; //how is there no variable width in string formatting... (I mean I guess this would be faster in java)

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
				if(size < 1) {
					System.err.println("Size must be greater than 0");
					System.exit(1);
				} else if(irow < 0 || icol < 0 || irow >= size || icol >= size) {
					System.err.println("Initial position must be on the board");
					System.exit(1);
				}
			case 0:
				break;
			default:
				System.err.println(help);
				System.exit(1);
		}

		max = size * size;
		board = new int[size][size];
		boolean ret;

		ret = b ? bruteForce(1, irow, icol) : warnsdorf(1, irow, icol);
		
		if(ret)
			printBoard();
		else 
			System.out.println("No Solution Exists");
	}

	private static int 	size = 8,		//length of one side of square board
						irow = 0,		//initial row
		   				icol = 0,		//initial column
						max,			//number of squares on board
						board[][];		//board, stores move numbers
	
	private static boolean b = false;	//if true, then brute force, else Warnsdorf

	//private static final int[]	roffs = {2, 2, 1, 1, -1, -1, -2, -2},	//row offsets
	//							coffs = {1, -1, 2, -2, 2, -2, 1, -1};	//column offsets
	private static final int[]	roffs = {2, 1, -1, -2, -2, -1, 1, 2},	//row offsets
								coffs = {1, 2, 2, 1, -1, -2, -2, -1};	//column offsets

	private static String help = "Usage: java Knight [board_size [starting_row starting_column [brute_force]]";
}
