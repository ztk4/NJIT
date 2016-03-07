#include <chrono> //portable!!
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <cstdio>

using namespace std::chrono;
typedef unsigned long long ull;
typedef high_resolution_clock hr_clock;

const char *usage = "Usage: %s [number of terms]\n";

//Implementation of Assignment
ull fib(ull *res, unsigned int n) {
    if(n < 2)
        return res[n] = n; //f(0) = 0, f(1) = 1

    return res[n] = fib(res, n-1) + fib(res, n-2); //could easily be memoized (but that's not the point!)
}

//Overly Verbose Error Checking And Etc. (just for fun)
int main(int argc, char **argv) {
    unsigned int n = 50;
    
    if(argc > 2) {
        fprintf(stderr, usage, *argv);
        return 1;
    } else if (argc == 2) {
        unsigned long tmp = strtoul(argv[1], NULL, 0); //0 allows any number format, octal, decimal, and hex

        if(errno == ERANGE) {
            fprintf(stderr, "Error: Argument '%s' is not a valid positive integer\n", argv[1]);
            return 2;
        }

        if(tmp > UINT_MAX) {
            fprintf(stderr, "Error: Integer '%lu' is too large (Maximum Value: %u)\n", tmp, UINT_MAX);
            return 3;
        }

        n = (unsigned int) tmp;
    }

    ull *res  = (ull *) malloc(sizeof(ull) * (n+1)),
        *eres = res + (n+1);

    hr_clock::time_point tp = hr_clock::now();
    fib(res, n);
    duration<double> seconds = duration_cast<duration<double>>(hr_clock::now() - tp); //converts default diff to seconds

    printf("Fibonacci terms 0 through %u:\n", n);
    
    printf("%llu", *res);
    for(ull *v = res + 1; v < eres; ++v)
        printf(", %llu", *v);

    printf("\nTime Taken: %.9f seconds\n", seconds.count());

    return 0;
}
