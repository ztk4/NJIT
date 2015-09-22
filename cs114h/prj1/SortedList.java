import java.util.Iterator;
import java.lang.IndexOutOfBoundsException;

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
		if(head == null) {				//empty list
			head = new Node<E>(data);
		} else if(head.data.compareTo(data) >= 0) {	//insert before head
			Node<E> tmp = head.next;
			head = new Node<E>(data);
			head.next = tmp;
		} else {						//general case
			insert(data, head);
		}
	}

	private void insert(E data, Node<E> n) {
		if(n.next == null) {				//insert at end
			n.next = new Node<E>(data);
		} else if(n.next.data.compareTo(data) >= 0) {	//insert after n
			Node<E> tmp = n.next;
			n.next = new Node<E>(data);
			n.next.next = tmp;
		} else {							//move on
			insert(data, n.next);
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
				return curr == null;
			}

			public E next() {
				E data = curr.data;
				curr = curr.next;
				return data;
			}

			private Node<E> curr = head;
		};
	}

	/**
	 * Removes element <code>data</code> from this list, maintaining ascenging sorted order.
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
		if(n.next != null) { 			//if null, went through whole list
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
	public E retrieve(int index) throws IndexOutOfBoundsException {
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
	 * Test routine for this class.
	 *
	 * @param args	a <code>String[]</code> of the args passed to this program from the CLI
	 */
	public static void main(String[] args) {
		
	}

}
