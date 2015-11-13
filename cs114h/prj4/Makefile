JC = javac

all: BinarySearchTree

.PHONY: BinarySearchTree

BinarySearchTree: BinarySearchTree.class

BinarySearchTree.class: BinarySearchTree.java BinaryTree.java
	$(JC) BinarySearchTree.java BinaryTree.java

clean:
	rm *.class
