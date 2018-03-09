function [p, points] = cheb(a, b, f, n)
% cheb uses chebyshev nodes for interpolating f with a degree n polynomial.

X = (a + b)/2 + (a - b)/2 * cos((2*(1:n+1) - 1)./(2*n + 2) * pi);
Y = f(X);

% Always using Lagrange Basis Functions now.
p = lpfit(X, Y);

points = [X' Y'];
end