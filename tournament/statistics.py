from math import sqrt

# Tracks simple statistics
class Statistics:
    def __init__(self):
        self._sum = 0.0
        self._sumSq = 0.0
        self._count = 0
        self._min = 1e100;   # TODO: use proper numerical limits
        self._max = -1e100;  # TODO: use proper numerical limits

    def mean(self):
        if (self._count == 0):
            return 0.0
        return self._sum / self._count

    def count(self):
        return self._count

    def min(self):
        return self._min

    def max(self):
        return self._max

    def sum(self):
        return self._sum

    def variance(self):
        if (self._count == 0):
            return 0.0
        return (self._sumSq / self._count) - self.mean() * self.mean()

    def stddev(self):
        if (self._count == 0):
            return 0.0
        return sqrt(self.variance())

    def stderror(self):
        if (self._count == 0):
            return 0.0
        return self.stddev() / sqrt(self._count)

    def add(self, value):
        self._count = self._count+1
        self._sum = self._sum + value
        self._sumSq = self._sumSq + (value * value)
        if (value < self._min):
            self._min = value
        if (value > self._max):
            self._max = value

    # outputs string representation
    def dump(self):
        return "%.1f (+-%.1f) min=%.1f max=%.1f deviation=%.1f" % \
               (self.mean(), self.stderror(), self.min(),
                self.max(), self.stddev())
        
        
        
        

