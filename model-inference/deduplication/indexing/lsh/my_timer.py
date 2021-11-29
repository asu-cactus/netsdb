import time

class Timer(object):
    """A convenient class to measure the running time of a program

    """

    def __init__(self):
        self.start = 0
        self.end = 0
    
    def tic(self):
        """Tic the start time

        """
        self.start = time.perf_counter()
    
    def toc(self):
        """Toc the end time and return the running time

        Returns:
            float: running time (ms)
        """
        self.end = time.perf_counter()
        return (self.end - self.start)*1000