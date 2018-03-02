function Pn = pfit(X, Y)
% pfit finds the polynomial fitting points defined by the cocentric vectors
%      X and Y. NB: X and Y must have the same length.

% Force X to be a row vec.
X = reshape(X, 1, length(X));
% Force Y to be a row vec.
Y = reshape(Y, 1, length(Y));
         
    function y = pn(x)
        y = 0;
        for i = 1:length(X)
            % The i'th column of y is Li evaluated on each input x.
            y = y + Y(i)*lagrange(x, i, X);
        end
    end

Pn = @pn;
end

function y = lagrange(x, i, X)
% lagrange returns the ith Lagrange function from the length(X) - 1 order 
%          Lagrange basis as defined by vector X evaluated at x
%          NB: X must be a row vector.
%          returns y as a column vector.

% Force x to be a col vec.
x = reshape(x, length(x), 1);

% \Pi_{j = 1, j \not= i}^{|X|} (x - X_j)/(X_i - X_j)
skip = [X(1:i-1) X(i+1:end)];  % skips X(i)
% The below uses matlab R2017's ability to automatically repmat matrix
% arguments to binary matrix operators. MAY NOT WORK IN R2016.
y = prod((x - skip)./(X(i) - skip), 2);  % Product along column dimension
end