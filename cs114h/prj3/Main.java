import java.util.Random;

public class Main {

    public static void main(String[] args) {

        int num = args.length == 1 ? Integer.parseInt(args[0]) : 10;
        Integer[] a = new Integer[num];
        Random rand = new Random(1);

        for (int i = 0; i < a.length; ++i) {
            a[i] = rand.nextInt(num);
        }

        long start = System.currentTimeMillis();
        Sorts.mergeSort(a);
        long stop = System.currentTimeMillis();
        System.out.print("merge: " + (stop - start) + " ");

        for (int i = 0; i < a.length - 1; ++i) {
            if (a[i] > a[i+1]) {
                System.out.println("Fail");
                System.exit(0);
            }
        }

        System.out.println("Pass");

        rand = new Random(1);

        for (int i = 0; i < a.length; ++i) {
            a[i] = rand.nextInt(num);
        }

        start = System.currentTimeMillis();
        Sorts.quickSort(a);
        stop = System.currentTimeMillis();
        System.out.print("quick: " + (stop - start) + " ");

        for (int i = 0; i < a.length - 1; ++i) {
            if (a[i] > a[i+1]) {
                System.out.println("Fail");
                System.exit(0);
            }
        }

        System.out.println("Pass");

        rand = new Random(1);

        for (int i = 0; i < a.length; ++i) {
            a[i] = rand.nextInt(num);
        }

        start = System.currentTimeMillis();
        Sorts.heapSort(a);
        stop = System.currentTimeMillis();
        System.out.print("heap: " + (stop - start) + " ");

        for (int i = 0; i < a.length - 1; ++i) {
            if (a[i] > a[i+1]) {
                System.out.println("Fail");
                System.exit(0);
            }
        }

        System.out.println("Pass");
    }
}
