function [estimate, err, order] = secant(f, x0, x1, tol)
a = x0;
b = x1;
err = f(b)*(b - a)/(f(b) - f(a));

prev1_err = inf;
prev2_err = inf;

while abs(err) > tol
  a = b;
  b = b - err;
  prev2_err = prev1_err;
  prev1_err = err;
  err = f(b)*(b - a)/(f(b) - f(a));
end

% Easily derived from the relation e_n = C|e_{n-1}|^p and
%                              e_{n-1} = C|e_{n-2}|^p.
order = log(abs(err/prev1_err))/log(abs(prev1_err/prev2_err));

estimate = b;
end