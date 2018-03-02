function [approx, n] = taylors_expand(f, a, lo, hi, tol)
num_points = 1e6;
X = linspace(lo, hi, num_points);  % Our support.

% Assume max of 100 terms.
coeffs = zeros(1, 100);
coeffs(end) = f(a);  % First coeff.
n = 1;

% Storage for symbolic derivatives.
sym_f = sym(f);
% Storage for factorial
fact = 1;

while norm(abs(f(X) - polyval(coeffs, X - a)), inf) > tol
  syms x df;
  sym_df = diff(sym_f, x);
  fact = fact * n;
  x = a;
  coeffs(end - n) = double(subs(sym_df)) / fact;
  
  sym_f = sym_df;
  n = n + 1;
end
    
approx = @(x) polyval(coeffs, x - a);
end