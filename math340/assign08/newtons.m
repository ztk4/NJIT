function [estimate, err, order] = newtons(f, df, x0, tol)

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