function [estimate, err, order] = complex_newtons(f, df, z0, a, tol)

prev1_err = inf;
prev2_err = inf;

% These are complex now.
estimate = z0;
err = a*f(estimate)/double(df(estimate));

% NOTE: abs(err) gets the modulus of err.
while abs(err) > tol
  estimate = estimate - err;
  prev2_err = prev1_err;
  prev1_err = err;
  err = a*f(estimate)/double(df(estimate));
end

% Easily derived from the relation e_n = C|e_n-1|^p and
%                                e_n-1 = C|e_n-2|^p.
order = log(abs(err/prev1_err))/log(abs(prev1_err/prev2_err));

err = abs(err);
end