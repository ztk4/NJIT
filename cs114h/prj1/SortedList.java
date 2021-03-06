/*
 * Zachary Kaplan
 * ztk4 - 31353570
 * CS 114-H01
 * Porject 1 - SortedList
 * 10/9/15
 */

import java.util.Iterator;
import java.lang.IndexOutOfBoundsException;
import java.lang.UnsupportedOperationException;
import java.lang.StringBuilder;

import java.util.Random; //testing only

/**
 * Allows insertion and deletion of elements while maintaining ascending 
 * sorted order. Also allows for the retrieval of elements by index, and 
 * checking if an element is or is not in a list.
 *
 * @author	Zachary Kaplan
 * @version	1.0
 * @param <E> the type of the data stored in this list
 */
public class SortedList<E extends Comparable<? super E>> extends List<E> {
	
	//protected Node<E> class inherited from List

	/**
	 * Inserts <code>data</code> into this list, maintaining ascending sorted order.
	 * 
	 * @param data	 the element to insert into this list
	 */
	public void insert(E data) {
		head = insert(head, data);
	}

	private Node<E> insert(Node<E> n, E data) {
		if(n == null || data.compareTo(n.data) <= 0) {
			Node<E> tmp = new Node<>(data);
			tmp.next = n;
			return tmp;
		} else {
			n.next = insert(n.next, data);
			return n;
		}
	}
	
	/**
	 * Gets an <code>Iterator<E></code> that can traverse the list moving forward only.
	 *
	 * @return	an <code>Iterator<E></code> to traverse the list
	 */
	public Iterator<E> iterator() {
		return new Iterator<E>() {
			
			public boolean hasNext() {
				return curr != null;
			}

			public E next() {
				E data = curr.data;
				curr = curr.next;
				return data;
			}
			
			public void remove() {
				throw new UnsupportedOperationException();
			}

			private Node<E> curr = head;
		};
	}

	/**
	 * Removes element <code>data</code> from this list, maintaining ascending sorted order.
	 *
	 * @param data	the element to remove from this list
	 */
	public void remove(E data) {
		if(head != null) {			//not empty
			if(head.data.compareTo(data) == 0)	//remove first element
				head = head.next;
			else
				remove(data, head);	//general case
		}
	}

	private void remove(E data, Node<E> n) {
		if(n.next != null) { 			//if null, (null -> went through whole list)
			if(n.next.data.compareTo(data) == 0)		//remove next node
				n.next = n.next.next;
			else						//move on
				remove(data, n.next);
		}
	}

	/**
	 * Retrieves the element at <code>index</code> from the list, and returns its value.
	 * 
	 * @param index							index of element to return this list
	 * @return 								element at the specified index
	 * @throws IndexOutOfBoundsException	if <code>index</code> is less than 0, or 
	 * 										greater than or equal to the number of elements	
	 */
	public E retrieve(int index) {
		if(index < 0)
			throw new IndexOutOfBoundsException("Index '" + index + "' is less than 0");
		
		return retrieve(index, head);
	}

	private E retrieve(int index, Node<E> n) {
		if(n == null)
			throw new IndexOutOfBoundsException("Index is past the end of the list");

		return index == 0 ? n.data : retrieve(index - 1, n.next); 	//if there, return n.data, else move on
	}

	/**
	 * Searches for the element <code>data</code> in this list, and returns <code>true</code> if it is found.
	 *
	 * @param data	the element to search for in this list
	 * @return		<code>true</code> if data is in the list, and <code>false</code> if not
	 */
	public boolean search(E data) {
		return search(data, head);
	}

	private boolean search(E data, Node<E> n) {
		if(n == null)		//end of list
			return false;
		else
			return n.data.compareTo(data) == 0 ? true : search(data, n.next); 	//if matched, return true, else move on
	}

	//protected Node<E> head inherited from List
	
	/**
	 * Method that allows a SortedList to be converted to a String.
	 * This is useful for printing the list
	 */
	public String toString() {
		StringBuilder sb = new StringBuilder("[ ");
		for(E e : this) {
			sb.append(e);
			sb.append(' ');
		}
		sb.append(']');
		return sb.toString();
	}

	/**
	 * Main routine for testing Sorted List.
	 * Packs in n random numbers, does some sorting and retreiving, and then unpacks the numbers
	 *
	 * @param args	String[] that holds the argument list from the CLI
	 */
	public static void main(String[] args) {
		int n = args.length == 0 ? 10 : Integer.parseInt(args[0]);
		Random r = new Random(1);
		List<Integer> l = new SortedList<>();

		System.out.println("\nInserting:\n");

		for(int i = 0; i < n; ++i) {
			int tmp = r.nextInt(n);
			l.insert(tmp);
			System.out.print(tmp + ": ");
			System.out.println(l);
		}
		
		System.out.println();
		System.out.println("\nSearching:\n");

		for(int i = 0; i < n; ++i) {
			int tmp = r.nextInt(n);
			System.out.println(tmp + ": " + l.search(tmp));
		}

		System.out.println();
		System.out.println("\nRetrieving:\n");
		
		for(int i = 0; i < n; ++i)
			System.out.println(i + ": " + l.retrieve(i));
		
		System.out.println();
		System.out.println("\nRemoving:\n");

		r = new Random(1);

		for(int i = 0; i < n; ++i) {
			int tmp = r.nextInt(n);
			l.remove(tmp);
			System.out.print(tmp + ": ");
			System.out.println(l);
		}

		System.out.println();
	}
}
