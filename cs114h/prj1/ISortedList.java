import java.util.Iterator;
import java.lang.IllegalStateException;
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
public class ISortedList<E extends Comparable<? super E>> extends List<E> {
	
	//protected Node<E> class inherited from List

	/**
	 * Inserts <code>data</code> into this list, maintaining ascending sorted order.
	 * 
	 * @param data	 the element to insert into this list
	 */
	public void insert(E data) {
		if(head == null) {				//empty list
			head = new Node<E>(data);
		} else if(head.data.compareTo(data) >= 0) {	//front of list
			Node<E> temp = head;
			head = new Node<E>(data);
			head.next = temp;
		} else {						//general case
			Node<E> curr = head,	//node before insertion
				 temp = null;	//node after insertion

			while(curr.next != null) { 
				if(curr.next.data.compareTo(data) >= 0) {	//insertion point found
					temp = curr.next;			//set temp to next node
					break;
				}
				curr = curr.next;
			}									//if not reassigned, temp remains null (as if end of list)

			curr.next = new Node<E>(data);
			curr.next.next = temp;
		}
	}
	
	/**
	 * Gets an <code>Iterator<E></code> that can traverse the list moving forward only.
	 *
	 * @return	an <code>Iterator<E></code> to traverse the list
	 */
	public Iterator<E> iterator() {
		return new Iterator<E>() {
			private Node<E> bk2 = new Node<>(null);//bs node 'two back' from head
			
			{	//intance initializer
				bk2.next = new Node<>(null);
				bk2.next.next = head;
			}

			public boolean hasNext() {
				return bk2.next.next == null;
			}

			public E next() {
				bk2 = bk2.next;
				return bk2.next.data;
			}

			public void remove() {
				if(bk2.next.next == head || !hasNext())
					throw new IllegalStateException();
				bk2.next = bk2.next.next;	
			}

		};
	}

	/**
	 * Removes element <code>data</code> from this list, maintaining ascending sorted order.
	 *
	 * @param data	the element to remove from this list
	 */
	public void remove(E data) {
		if(head != null) {				//not empty
			if(head.data.compareTo(data) == 0) {     //remove first node
				head = head.next;
			} else {					//general case
				Node<E> curr = head;		//before removal point
				
				while(curr.next != null) {
					if(curr.next.data.compareTo(data) == 0) {
						curr.next = curr.next.next;
						return;
					}
				}
			}
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
		if(index < 0) {
			throw new IndexOutOfBoundsException("Index '" + index + "' is less than 0");
		}

		Node<E> curr = head;
		while(index-- > 0 && curr != null) {
			curr = curr.next;
		}
		
		if(curr == null) {
			throw new IndexOutOfBoundsException("Index '" + index + "' is past the end of the list");
		}
		return curr.data;
	}

	/**
	 * Searches for the element <code>data</code> in this list, and returns <code>true</code> if it is found.
	 *
	 * @param data	the element to search for in this list
	 * @return		<code>true</code> if data is in the list, and <code>false</code> if not
	 */
	public boolean search(E data) {
		Node<E> curr = head;
		while(curr != null) {
			if(curr.data.compareTo(data) == 0) {
				return true;
			} 
			curr = curr.next;
		}
		return false;
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
