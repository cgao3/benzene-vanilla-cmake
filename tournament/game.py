#----------------------------------------------------------------------------
# A game of Hex.
# Contains a list of moves and a result.
#----------------------------------------------------------------------------
class Game:
    
    def __init__(self):
        self._result = '?'
        self._elapsedBlack = 0.0
        self._elapsedWhite = 0.0
        self._moves = []

    def addMove(self, move):
        self._moves.append(move)

    def moveList(self):
        return self._moves

    def playedSwap(self):
        if (len(self._moves) >= 1 and self._moves[1] == 'swap-pieces'):
            return True
        return False

    def getLength(self):
        return len(self._moves)
    
    def setResult(self, result):
        self._result = result

    def getResult(self):
        return self._result

    def setElapsed(self, color, elapsed):
        if (color == "black"):
            self._elapsedBlack = elapsed
        elif (color == "white"):
            self._elapsedWhite = elapsed

    def getElapsed(self, color):
        if (color == "black"):
            return self._elapsedBlack
        elif (color == "white"):
            return self._elapsedWhite
