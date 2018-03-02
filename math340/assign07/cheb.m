function [p, points] = cheb(a, b, f, n, type)
% cheb uses chebyshev nodes for interpolating f with a degree n polynomial.
%      type has two valid values: 'newton' and 'lagrange'.

X = (a + b)/2 + (a - b)/2 * cos((2*(1:n+1) - 1)./(2*n + 2) * pi);
Y = f(X);

if strcmp(type, 'newton')
    p = npfit(X, Y);
elseif strcmp(type, 'lagrange')
    p = lpfit(X, Y);
else
    error(['invalid type ' type ]);
end
points = [X' Y'];
end