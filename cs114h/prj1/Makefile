JC = javac

.PHONY: SortedList ISortedList

all: SortedList ISortedList

SortedList: SortedList.class

ISortedList: ISortedList.class

SortedList.class: SortedList.java List.java
	$(JC) SortedList.java List.java

ISortedList.class: ISortedList.java List.java
	$(JC) ISortedList.java List.java

clean:
	rm -f *.class
