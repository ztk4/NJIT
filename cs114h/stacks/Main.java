public class Main {
	
	public static void main(String[] args) {
		Stack<Integer> stack = Math.random() < 0.5 ? 
					new AStack<>(): 
					new RStack<>();
		System.out.println(stack);
		for(int i = 0; i < 100; ++i) {
			stack.push(i);
			System.out.println(i);
		}

		for(int i = 0; i < 100; ++i) {
			System.out.println(stack.pop(i));	
		}
	}

}

interface Stack<T> {
	T pop();
	void push(T t);
}

class AStack<T> implements Stack<T> {

	private Object[] stack = new Object[10];
	private int top;
	
	public T pop() {
		T tmp = null;

		if(stack.length > 10 && top*3 < stack.length)
			shrink();
		}

		if(top > 0) {
			tmp = (T)stack[--top];
		}

		return tmp;
	}

	public void push(T t) {
		if(top >= stack.length) {
			grow();
		}

		stack[top++] = t;
	}

	private void grow() {
		Object[] tmp = new Object[stack.length * 2];
		
		for(int i = 0; i < top; ++i) 
			tmp[i] = stack[i];

		stack = tmp;
	}

	private void shrink() {
		Object tmp = new Object[stack.length/2];

		for(int i = 0; i < top; ++i)
			tmp[i] = stack[i];

		stack = tmp;
	}
}

class RStack<T> implements Stack<T> {

	private class Node<T> {
	
		private Node(T t) {
			this.t = t;
		}

		private T t;
		private Node<T> next;
	}

	public T pop() {
		T tmp = null;

		if(head != null) {
			tmp = head.t;
			head = head.next;
		}
	}

	public void push(T t) {
		Node<T> tmp = new Node<>(t);

		tmp.next = head;
		head = tmp;
	}

	private Node<T> head;
}
