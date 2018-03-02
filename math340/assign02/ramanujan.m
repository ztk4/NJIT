function [approx_pi, n] = ramanujan(tol)
if tol < 0
  error("Can't have a negative tolerance");
end

approx_1bypi = 0;
n = 0;

% Some precomputed values.
coef = 2*sqrt(2)/99^2;
while abs(pi - 1/approx_1bypi) > tol
    approx_1bypi = approx_1bypi + coef*factorial(4*n)/factorial(n)^4 * ...
                                  (26390*n + 1103)/396^(4*n);
    n = n + 1;
end

approx_pi = 1/approx_1bypi;
end