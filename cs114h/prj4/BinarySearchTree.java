/*
 * Zachary Kaplan
 * ztk4 - 31353570
 * CS 114-H01
 * Project 5 - Binary Search Tree
 * MM/DD/YY
 */

import java.util.Iterator;

/**
 * Stores comparable in a sorted Binary Tree.
 * Avg Case O(logn) lookup
 * NOT BALANCED (can degenerate to a linked list)
 *
 * @author 		Zachary Kaplan
 * @version 	1.0
 */
public class BinarySearchTree<E extends Comparable<? super E>> extends BinaryTree<E> {
	
	public void insert(E data) {
		root = insert(data, root);
	}

	private Node<E> insert(E data, Node<E> root) {
		if(root == null) 
			return new Node<>(data);

		switch(data.compareTo(root.data)) {
		case -1:
		case 0:
			root.left = insert(data, root.left);
			break;
		default: //case 1
			root.right = insert(data, root.right);
		}

		return root;
	}

	public void remove(E data) {
		root = remove(data, root);
	}

	private Node<E> remove(E data, Node<E> root) {
		if(root == null)
			return root;

		switch(data.compareTo(root.data)) {
		case -1:
			root.left = remove(data, root.left);
			break;
		case 0:
			root.left = root.left == null ? null : remswp(root.left, root);
			break;
		default: //case 1
			root.right = remove(data, root.right);
		}

		return root;
	}

	private Node<E> remswp(Node<E> root, Node<E> swap) {
		if(root.right == null) {
			swap.data = root.data;
			return root.left;
		}
		
		root.right = remswp(root.right, swap);
		return root;
	}

	public boolean search(E data) {
		search(data, root);
	}

	private boolean search(E data, Node<E> root) {
		if(root == null)
			return false;

		switch(data.compareTo(root.data)) {
			case -1:
				return search(data, root.left);
			case 0:
				return true;
			default: //case 1
				return search(data, root.right);
		}
	}

	public Iterator<E> iterator() {
		return new Iterator<>() {
			
		};
	}

}
