function [estimate, err, order] = newtons(f, x0, tol)

syms x;
try
  df = diff(symfun(f(x), x));
catch e
  % We could handle the error of a non-differentiable f here, but it's not
  % clear what we could do, so we throw the error up.
  error(e);
end

prev1_err = inf;
prev2_err = inf;

estimate = x0;
err = f(estimate)/double(df(estimate));

while abs(err) > tol
  estimate = estimate - err;
  prev2_err = prev1_err;
  prev1_err = err;
  err = f(estimate)/double(df(estimate));
end

% Easily derived from the relation e_n = C|e_n-1|^p and
%                                e_n-1 = C|e_n-2|^p.
order = log(abs(err/prev1_err))/log(abs(prev1_err/prev2_err));

err = abs(err);
end