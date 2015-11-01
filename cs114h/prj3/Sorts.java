/*
 * Zachary Kaplan
 * ztk4 - 31353570
 * CS 114-H01
 * Project 3 - HeapSort
 * 11/20/15
 */

import java.util.Arrays;

public class Sorts {

	/**
	 * Sorts an array by converting it to a max-heap, and then repeatedly popping the head and re-heaping.
	 *
	 * @param array		The array to be sorted
	 */
    public static <T extends Comparable<? super T>> void heapSort(T[] array) {
		heapify(array);

		for(int end = array.length - 1; end > 0; --end) {
			T tmp = array[0];
			array[0] = array[end];
			array[end] = tmp;

			siftDown(array, 0, end);
		}
    }
	
	/**
	 * Turns an array into a max-heap.
	 *
	 * @param array		The array representing the heap
	 */
	private static <T extends Comparable<? super T>> void heapify(T[] array) {
		int root = (array.length - 2) / 2; //floor division of one less than the last element (array.length - 1) by 2

		do
			siftDown(array, root, array.length);
		while(root-- > 0);
	}

	/**
	 * Sifts and element down a binary treey implemented with an array.
	 * Creates a heap rooted at root assuming all decendants of root are heaps.
	 *
	 * @param array		The array representing the binary tree
	 * @param root		The head of the binary tree
	 * @param end		One more than the index of the last element in the tree
	 */
	private static <T extends Comparable<? super T>> void siftDown(T[] array, int root, int end) {
		int swap = root;
		
		do {
			root = swap;

			int child = root * 2 + 1;
			if(child < end && array[child].compareTo(array[swap]) > 0)
				swap = child;
			
			child += 1;
			if(child < end && array[child].compareTo(array[swap]) > 0)
				swap = child;
			
			T tmp = array[root];
			array[root] = array[swap];
			array[swap] = tmp;
		} while(swap != root);
	}

    public static <T extends Comparable<? super T>> void bubbleSort(T[] array) {

        T temp;
        boolean sorted;

        for (int i = array.length - 1; i > 0; --i) {

            sorted = true;

            for (int j = 0; j < i; ++j) {

                if (array[j].compareTo(array[j + 1]) > 0) {

                    sorted = false;

                    temp = array[j];
                    array[j] = array[j + 1];
                    array[j + 1] = temp;
                }
            }

            if (sorted) {
                break;
            }
        }
    }

    public static <T extends Comparable<? super T>> void insertSort(T[] array) {

        T temp;
        int j;

        for (int i = 1; i < array.length; ++i) {

            temp = array[i];

            for (j = i; j > 0; --j) {

                if (array[j - 1].compareTo(temp) > 0) {
                    array[j] = array[j - 1];
                }
                else {
                    break;
                }
            }

            if (j < i) {
                array[j] = temp;
            }
        }
    }

    public static <T extends Comparable<? super T>> void mergeSort(T[] array) {

        if (array.length > 1) {

            T[] left  = Arrays.copyOfRange(array, 0, array.length / 2);
            T[] right = Arrays.copyOfRange(array, array.length / 2, array.length);

            mergeSort(left);
            mergeSort(right);

            int i, l = 0, r = 0;

            for (i = 0; i < array.length && l < left.length && r < right.length; ++i) {
                if (left[l].compareTo(right[r]) <= 0) {
                    array[i] = left[l++];
                }
                else {
                    array[i] = right[r++];
                }
            }

            if (i < array.length) {
                if (l < left.length) {
                    while (i < array.length) {
                        array[i++] = left[l++];
                    }
                }
                else {
                    while (i < array.length) {
                        array[i++] = right[r++];
                    }
                }
            }
        }
    }

    public static <T extends Comparable<? super T>> void quickSort(T[] array) {

        quickSort(array, 0, array.length - 1);
    }

    private static <T extends Comparable<? super T>> void quickSort(T[] array, int left, int right) {

        int pivot = right;

        int l = left, r = right;

        if (left < right) {
            while (l < r) {

                while (l < r && array[l].compareTo(array[pivot]) <= 0) {
                    ++l;
                }

                while (l < r && array[pivot].compareTo(array[r]) <= 0) {
                    --r;
                }

                if (l < r) {
                    T temp = array[l];
                    array[l] = array[r];
                    array[r] = temp;
                }
            }

            if (r != pivot) {
                T temp = array[pivot];
                array[pivot] = array[r];
                array[r] = temp;
            }

            quickSort(array, left, r - 1);
            quickSort(array, r + 1, right);
        }
    }

    public static <T extends Comparable<? super T>> void selectSort(T[] array) {

        T temp;
        int mini;

        for (int i = 0; i < array.length - 1; ++i) {

            mini = i;

            for (int j = i + 1; j < array.length; ++j) {

                if (array[j].compareTo(array[mini]) < 0) {
                    mini = j;
                }
            }

            if (i != mini) {

                temp = array[i];
                array[i] = array[mini];
                array[mini] = temp;
            }
        }
    }
}
