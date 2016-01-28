def shortest(str_list):
    #return min(*(len(s) for s in str_list))
    return len(min(str_list, key=len))

def shortest_word(s):
    return len(min(s.split(), key=len))

beatleLine = "These are several tests!".split()

print(shortest(beatleLine))
