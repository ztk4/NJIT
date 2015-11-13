/*
 * Zachary Kaplan
 * ztk4 - 31353570
 * CS 114-H01
 * Project 5 - Binary Search Tree
 * MM/DD/YY
 */

import java.util.Iterator;
import java.util.Arrays;
import java.util.Random;

/**
 * Stores comparable in a sorted Binary Tree.
 * Avg Case O(logn) lookup
 * NOT BALANCED (can degenerate to a linked list)
 *
 * @author      Zachary Kaplan
 * @version     1.0
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
        return search(data, root);
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
        return new Iterator<E>() {
            
            Object[] stack = new Object[20];
            int end, leftDone = -1;

            {
                if(root == null)
                    end = -1;
                else
                    stack[end] = root;
            }

            private void setCapacity(int capacity) {
                Object[] tmp = new Object[capacity];

                for(int i = 0; i <= end; ++i)
                    tmp[i] = stack[i];

                stack = tmp;
            }

            public boolean hasNext() {
                return end >= 0;    
            }

            @SuppressWarnings("unchecked")
            public E next() {
                if(leftDone < end) {
                    while( ((Node<E>) stack[end]).left != null) {
                        if(end == stack.length - 1)
                            setCapacity(stack.length * 2);

                        stack[end + 1] = ((Node<E>) stack[end]).left;   //push node to stack
                        ++end;
                    }
                }

                Node<E> tmp = (Node<E>) stack[end--];                   //pop node off stack
                leftDone = end;

                if(tmp.right != null)
                    stack[++end] = tmp.right;                           //push right node to stack
                else if(end < stack.length / 3 && stack.length >= 10)   //only popped, no push, might need to shrink
                    setCapacity(stack.length / 2);

                return tmp.data;
            }

            public void remove() {
                throw new UnsupportedOperationException();
            }

        };
    }

    public static void main(String[] args) {
        int N = args.length < 1 ? 10 : Integer.parseInt(args[0]);
        Random rand = new Random(1);

        int[] arr = new int[N];
        BinaryTree<Integer> bst = new BinarySearchTree<>();

        for(int i = 0; i < N; ++i) {
            arr[i] = rand.nextInt(N);
            bst.insert(arr[i]);
        }

        Arrays.sort(arr);
        Iterator<Integer> it = bst.iterator();

        System.out.println(Arrays.toString(arr));

        System.out.print('[');
        for(int i : bst) {
            System.out.print(i + ", ");
        }
        System.out.println("\b\b]");

        for(int i : arr) {
            if(!it.hasNext()) {
                System.err.println("No Next Element");
                System.exit(1);
            }

            if(i != it.next()) {
                System.err.println("Element Mismatch");
                System.exit(1);
            }
        }

        System.out.println("All Good!");
    }
    
}
