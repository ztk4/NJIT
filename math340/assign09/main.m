%% Zachary Kaplan
 % Math 340
 % Assignment 9
 % 3/29/18
 
%% Problem 1

% Function and it's derivative.
f = @(x) exp(-x) .* cos(x);
F = @(x) exp(-x)./2 .* (sin(x) - cos(x));
tol = 1e-11;
[a, b] = deal(0, 12*pi);

% Value to compare with.
trueval = F(b) - F(a);

N = 1;  % Number of trapezoids to use.
est = inf;
err = est - trueval;
while abs(err) > tol
    % Estimate with next N (increasing exponentially to speed process).
    N = N * 2;
    est = mytrapz(f, a, b, N);
    err = est - trueval;
end

% Find the error for the last two runs to estimate order.
prev_err1 = mytrapz(f, a, b, N - 1) - trueval;
prev_err2 = mytrapz(f, a, b, N - 2) - trueval;
order = log(abs(err/prev_err1))/log(abs(prev_err1/prev_err2));

fprintf(['Using Trapezoid Method to Estimate ' ...
         'I(e^{-x} cos(x))_0^{12pi}:\n' ...
         '  Estimate: %.12f\n' ...
         '  Actual  : %.12f\n' ...
         '  Error   : %e\n' ...
         '  Order   : %f\n'], est, trueval, err, order);

fprintf('\nFunction Used:\n');
dbtype mytrapz

%% Problem 2

% Function and it's derivative.
f = @(x) exp(-x) .* cos(x);
F = @(x) exp(-x)./2 .* (sin(x) - cos(x));
tol = 1e-11;
[a, b] = deal(0, 12*pi);

% Value to compare with.
trueval = F(b) - F(a);

N = 0;  % Number of parabolas to use.
est = inf;
err = est - trueval;
prev_err1 = inf;
prev_err2 = inf;
while abs(err) > tol
    % Fast enough that linear probing is fine.
    N = N + 2;
    est = mysimpsons(f, a, b, N);
    
    % Error Bookkeeping.
    prev_err2 = prev_err1;
    prev_err1 = err;
    err = est - trueval;
end

% This order assumes that e_n corresponds to simpsons with 2n+1 parabolas.
order = log(abs(err/prev_err1))/log(abs(prev_err1/prev_err2));

fprintf(['Using Simpson''s Rule to Estimate ' ...
         'I(e^{-x} cos(x))_0^{12pi}:\n' ...
         '  Estimate: %.12f\n' ...
         '  Actual  : %.12f\n' ...
         '  Error   : %e\n' ...
         '  Order   : %f\n'], est, trueval, err, order);

fprintf('\nFunction Used:\n');
dbtype mysimpsons