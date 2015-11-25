import turtle
import string

triplets = (
    [0,1,2],[3,4,5],[6,7,8],  # horizontals
    [0,3,6],[1,4,7],[2,5,8],  # verticals
    [0,4,8],[6,4,2]           # diagonals
    )

cellSize = 100


def drawGrid(t, cellSize):
    ''' Draw two vertical and two horizontal lines '''

def labelCells():
    ''' Label the cells 0 through 8 '''

def drawX(t, cellNum):
    ''' Draw an 'X' in the specified cell with height and width equal
    to half the cell size.'''

def drawO(t, cellNum):
    ''' Draw an 'X' in the specified cell with height and width equal
    to half the cell size.'''

def getMove(player, board):
    ''' player is 'X' or 'O'. Return a int in range(9).'''


def isDraw(board):
    ''' Return True if the game is drawn (all triplets are blocked – contain  
    at least 1 'X' and 1 'O'), otherwise return False.''' 



def isXWin(board):
    ''' Return True if X has won -- some winTriplet contains three X's --  
    otherwise return False.'''


def isOWin(board):
    ''' Return True if O has won -- some winTriplet contains three O's --  
    otherwise return False.''' 


def gameOutcome(board):
    ''' Return 'X' if X wins, 'O' if O wins, 'draw' if no win is possible,
    and None otherwise.'''


def drawMove(player, t, cell):
    ''' Draw and 'X' or 'O' in the given cell, calling drawX or drawO'''

def playGame():
    ''' Set up the board, numbering the cells. Alternate play between X and O,
    checking for a win or draw after each move. Terminate with appropriate message
    when a win or draw is detected.'''
    cellSize = 100
    turtleWidth = 10
    gridColor = "black"
    xColor = "green"
    oColor = "blue"

    s = turtle.Screen()
    t = turtle.Turtle()
    t.hideturtle()
    t.width(turtleWidth)
    t.color(xColor)
    colors = {'X':'green', 'O':'blue'}

    board = ['-','-','-','-','-','-','-','-','-']
    player = 'X'
