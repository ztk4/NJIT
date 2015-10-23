#!/usr/bin/python3

''' Trapdoors & Catapults is a digital version of Chutes & Ladders.

    RULES:

    PLAYERS. There are two players. Humans are prompted for two names, but
    all subsequent action is handled by the program.

    BOARD. The board consists of spaces numbered 0 through 99.

    Ten of the spaces contain a catapult and 10 contain a trapdoor. No space
    contains both a catapult and a trapdoor.

    If a player lands on a catapult, the player is flung forward, possibly as
    far as but not including the winning space (99).

    If a player lands on a trapdoor, the player is dropped back, possibly as far
    as the starting space (0).

    The destination space the player is flung forward to by a catapult or dropped
    back to by a trapdoor never contains a catapult or trapdoor. However, any space
    may be a destination for multiple trapdoors and/or catapults.

    The spaces that contain trapdoors and catapults and the destination of each
    trapdoor and catapult is set at random when the game is set up.

    PLAY. The players begin on space 0. Players move by rolling a die numbered
    1-6. The first to land on space 99 wins.

    The program prints messages telling what happens to each player on each move.


    HOME ASSIGNMENT. COMPLETE THE GAME BY FILLING IN THE REQUESTED CODE FOR
    PROBLEMS 1-5. Read the game documentation and the specification for the
    code carefully. Follow the coding examples given, and make appropriate
    modifications. '''

import random

def getPlayerNames():
    print()
    player1 = input('Name of first player: ')
    player2 = input('Name of second player: ')

    ''' PROBLEM1: Return player1 and player2 '''
    return player1, player2


def boardSetup(boardSize, numTrapdoors, numCatapults):
    ''' Create and return two dictionaries -- one for trapdoors and one
        for catapults.

        In the trapdoor dictionary, each trapdoor location *loc* is a key
        and its value is the destination that the trapdoor drops the
        player back to. The key must be in the range 1-98, inclusive.
        The destination of the trapdoor (the value of the key) must be
        in the range 0 <= destination < e key.

        In the catapult dictionary, each key is a space containing a catapult.
        The key must be in the range 1-97. Its value is the space that the
        catapult flings the player forward to. The value must be greater than
        the key and <= 98. (Hint: each key must be unique
        and also, by the rules of the game, may not be a trapdoor. And a value,
        which is the destination, may not be a trapdoor or a catapult.)

        HINT ABOUT RULES: a space may not contain both a trapdoor and a catapult.
        A destination for a trapdoor or catapult may not be a trapdoor or a
        catapult. That is, if the player falls through a trapdoor or is catapulted,
        the player comes to rest in that position.

        HINT ABOUT IMPLEMENTATION: Use while loops to select the location and
        destination of a trapdoor or a catapult. Enter a trapdoor-destination pair
        or a catapult-destination pair into a dictionary only when they satisfy the
        rules of the game. '''
    trapdoors = {}
    catapults = {}

    ''' Select trapdoor-destination pairs at random '''
    while len(trapdoors) < numTrapdoors:
        trapdoorSpace = random.randint(1, boardSize-2)
        if trapdoorSpace in trapdoors:
            continue

        ''' Select a destination for this trapdoor. The destination
            must be >= 0 and less than the trapdoorSpace and must not be
            the location of a trapdoor'''
        destination = random.randint(0, trapdoorSpace-1)
        while destination in trapdoors:
            destination = random.randint(0, trapdoorSpace-1)

        ''' Enter the trapdoorSpace-destination pair into the dictionary '''
        trapdoors[trapdoorSpace] = destination

    ''' PROBLEM 2. Select catapult-destination pairs, following the pattern
        of the above code that selects trapdoor-destination pairs.

        BE SURE TO IMPLEMENT THESE CONDITIONS: No space may have both a trapdoor
        and a catapult. Neither the first nor the last nor the next to last space
        may have a catapult. The destination for a catapult may not be a catapult
        or a trapdoor. '''
    while len(catapults) < numCatapults:
        src = random.randint(1, boardSize-3)
        if src in trapdoors or src in catapults:
            continue

        dst = 0
        while dst in trapdoors or dst in catapults or not dst:
            dst = random.randint(src, boardSize-2)

        catapults[src] = dst

    ''' PROBLEM 3. Return the trapdoors catapults dictionaries '''
    return trapdoors, catapults

def rollDie():
    ''' Roll a six sided die and return the value of the roll '''
    return random.randint(1, 6)

def switchPlayer(currentPlayer, player1, player2):
    if currentPlayer == player1:
        return player2
    else:
        return player1

def playGame():
    ''' Get the names of the two players. Set up the board. Alternate play between
        the two players until one of them has won. On each move, print out the
        state of play, including the player, current position, move, and any
        encounter with a trapdoor or catapult. '''
    boardSize = 100
    numCatapults = 10
    numTrapdoors = 10

    ''' call getPlayerNames() to get names for each of the two players '''
    player1, player2 = getPlayerNames()

    ''' PROBLEM 4. Call boardSetup() to get the two dictionaries giving the
        space/destination pairs for trapdoors and catapults. Look at the
        parameters in the boardSetup function to determine what arguments
        need to be passed to it. '''
    trapdoors, catapults = boardSetup(boardSize, numTrapdoors, numCatapults)

    ''' Initialize the dictionary holding the position of each of the two
        players. Note: the keys are the names of the human players. '''
    position = {player1:0, player2:0}

    ''' Set the initial player '''
    currentPlayer = player1

    ''' Continue play until one player lands on the highest space, and wins '''
    while True:
        print()
        print('Player is', currentPlayer)
        print(currentPlayer, 'is on', position[currentPlayer])
        move = rollDie()
        print(currentPlayer, 'rolls a', move)

        targetPosition = position[currentPlayer] + move

        ''' OFF THE BOARD CASE: The roll of the die moves the player beyond
            the end of the board. Print out a message saying what happened. '''
        if targetPosition >= boardSize:
            print('Sorry', targetPosition, 'is off the board. No can do,', currentPlayer)

            ''' Switch to the other player '''
            currentPlayer = switchPlayer(currentPlayer, player1, player2)
            continue

        ''' ON THE BOARD CASE: The target position is on the board, so make the move '''
        position[currentPlayer] = targetPosition
        print(currentPlayer, 'moves to', position[currentPlayer])

        ''' PROBLEM 5. WE HAVE A WINNER CASE. If the current player is on the last
            space, print a WINS! message and break out of the game loop '''
        if targetPosition == boardSize-1:
            print(currentPlayer, 'WINS!')
            break

        ''' TRAPDOOR CASE: The current player has landed on a trapdoor. Move
            to the trapdoor destination and print out a message. '''
        if position[currentPlayer] in trapdoors:
            position[currentPlayer] = trapdoors[position[currentPlayer]]
            print('Trapdoor!', currentPlayer, 'falls to', position[currentPlayer])
            currentPlayer = switchPlayer(currentPlayer, player1, player2)
            continue

        if position[currentPlayer] in catapults:
            ''' PROBLEM 6. CATAPULT CASE. If the player has landed on a catapult,
            move to the catapult destination and print out a message. (Hint: follow
            the pattern in the trapdoor case. '''
            position[currentPlayer] = catapults[position[currentPlayer]]
            print('Catapult!', currentPlayer, 'is flung to', position[currentPlayer])
            currentPlayer = switchPlayer(currentPlayer, player1, player2)
            continue

        currentPlayer = switchPlayer(currentPlayer, player1, player2)

playGame()
