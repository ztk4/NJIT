#!/usr/bin/python

import re
from xml.dom.minidom import parse, parseString

def get_text(elm):
    if elm.nodeType == 4:
        return None
    elif elm.nodeType == 3:
        return elm.data
    else:
        return [get_text(e) for e in elm.childNodes if e.nodeType != 4 and e.nodeType != 3]

if __name__ == '__main__':
    dom = parse('bookstore.xml')
    print get_text(dom)
    print get_text(dom.getElementsByTagName('book')[2]);
