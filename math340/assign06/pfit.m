function Pn = pfit(X, Y)
% pfit finds the polynomial fitting points defined by the cocentric vectors
%      X and Y. NB: X and Y must have the same length.

% Force X to be a row vec.
X = reshape(X, 1, length(X));
% Force Y to be a row vec.
Y = reshape(Y, 1, length(Y));

% Initialize memoization for dd (NxN mat).
mem = NaN(length(X));
         
    function y = pn(x)
        y = 0;
        for i = 1:length(X)
            [diff, mem] = dd(1, i, X, Y, mem);
            % The i'th column of y is Ni evaluated on each input x,
            % multiplied by [y_1, ..., y_i]
            y = y + diff*newton(x, i, X);
        end
    end

Pn = @pn;
end

function ni = newton(x, i, X)
% newton returns the ith Newton function from the length(X) - 1 order 
%        Newton basis as defined by vector X evaluated at x
%        NB: X must be a row vector.
%        returns y as a column vector.

% Force x to be a col vec.
x = reshape(x, length(x), 1);

% \Pi_{j = 1}^{i-1} (x - X_j)
ni = prod(x - X(1:i-1), 2);  % Product along column dimension
end

function [diff, mem] = dd(i, j, X, Y, mem)
% dd returns the divided difference [Y_i, ..., Y_j] with points (X_i, Y_i)

% Handle error case
if i > j
    error('i may not exceed j');
end

% Check memoization
if not(isnan(mem(i, j)))
    diff = mem(i, j);
    return
end

% Compute unknown value
if i == j
    diff = Y(i);
else
    [ld, mem] = dd(i+1, j, X, Y, mem);
    [rd, mem] = dd(i, j-1, X, Y, mem);
    diff = (ld - rd) / (X(j) - X(i));
end
mem(i, j) = diff;
end