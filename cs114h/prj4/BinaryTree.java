public abstract class BinaryTree<E> implements Iterable<E> {

    protected class Node<T> {

        protected Node(T data) {
            this.data = data;
        }

        protected T data;
        protected Node<T> left;
        protected Node<T> right;
    }

    public abstract void insert(E data);
    public abstract void remove(E data);
    public abstract boolean search(E data);

    protected Node<E> root;
}
