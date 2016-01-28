import turtle as tr

#dashes method (A)
def dashes(t, segments, length):
    for i in range(segments):
        t.pd()
        t.fd(length)
        t.pu()
        t.fd(length)

#testing maint routine (B)
if __name__ == '__main__':
    t = tr.Turtle()
    s = tr.Screen()

    dashes(t, 10, 20)
