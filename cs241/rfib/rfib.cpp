#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono> //portable!!
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <cstdio>

using namespace std;
using namespace std::chrono;
typedef unsigned int ui;
typedef unsigned long long ull;
typedef high_resolution_clock hr_clock;

const char *usage = "Usage: %s [number of terms]\n";

//Implementation of Assignment
ull fib(ui n) {
    if(n < 2)
        return n; //f(0) = 0, f(1) = 1

    return fib(n-1) + fib(n-2); //could easily be memoized (but that's not the point!)
}

//Overly Verbose Error Checking And Etc. (just for fun)
int main(int argc, char **argv) {
    ui N = UINT_MAX; //will run until overflow by default
    ofstream csv("result.csv");
    
    if(argc > 2) {
        fprintf(stderr, usage, *argv);
        return 1;
    } else if (argc == 2) {
        unsigned long tmp = strtoul(argv[1], NULL, 0); //0 allows any number format, octal, decimal, and hex

        if(errno == ERANGE) {
            cerr << "Error: Argument '" << argv[1] << "' is not a valid positive integer" << endl;
            return 2;
        }

        if(tmp > UINT_MAX) {
            cerr << "Error: Integer '" << tmp << "' is too large (Maximum Value: " << UINT_MAX << ')' << endl;
            return 3;
        }
        
        N = (ui) tmp;
    }

    cout << setprecision(9) << fixed; //for printing of time
    csv << setprecision(9) << fixed;

    csv << "Term #,Value,Time Taken (seconds)" << endl;
    ull last = 0;
    for(ui n = 0; n <= N; ++n) {
        hr_clock::time_point tp = hr_clock::now();
        ull tmp = fib(n);
        double s = (double)(hr_clock::now() - tp).count() * hr_clock::period::num / hr_clock::period::den; //converts default diff to seconds

        if(last > tmp) {
            cout << "Overflow occurred!" << endl;
            break;
        } else
            last = tmp;

        cout << "Time Taken: " << setw(15) << s << " seconds => fib(" << n << ") = " << tmp << endl; //width needs to be set every time
        csv << n << ',' << tmp << ',' << s << endl; 
    }

    return 0;
}
