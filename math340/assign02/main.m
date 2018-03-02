%% Zachary Kaplan
 % MATH 340-006
 % Assignment 2
 % 2/1/18
 
%% Part 1

meters = 17;
res = to_planck_len(meters);
fprintf("%d meters in planck lengths: %e\n", meters, res);

fprintf("The function used:\n");
dbtype to_planck_len;

fprintf("\nError when called with negative value: ");
err = "No Error";
try
  to_planck_len(-2);
catch e
  err = e.message;
end
fprintf("%s\n", err);

%% Part 2

tol = 1e-5;
res = ramanujan(tol);
fprintf("Pi with at least tolerance %e is: %.17f\n", tol, res);

fprintf("The function used:\n");
dbtype ramanujan;

fprintf("\nError when called with a negative tolerance: ");
err = "No Error";
try
  ramanujan(-1e-5);
catch e
  err = e.message;
end
fprintf("%s\n", err);

%% Part 3

tol = 1e-12;
real = @(x) x ./ (1 - x - x.^2);
approx = taylors_expand(real, 0, -1/4, 1/4, tol);

X = -1/4:1e-4:1/4;
figure
hold on
plot(X, real(X));
plot(X, approx(X));
legend('x/(1-x-x^2)', 'approx')

fprintf("The function used to generate the approximation:\n");
dbtype taylors_expand;

%% Extra Credit

% NOTE: Assuming rand has roughly uniform coverage of (0, 1), and that
%       equal ganularity across (0, 1) will converge fast enough.

f = @(x) sqrt(1 - x.^2);
real = int(sym(f), 0, 1);

% Number of samples to approximate with.
N = 1e8;
X = rand(1, N);
Y = f(X);

figure
scatter(X, Y);
title('Points used to approximate integral');

estimate = sum(Y ./ N);  % Integral estimate.
fprintf(['Real Value : %f\n', ...
         'Approx     : %f\n', ...
         'Error      : %e\n'], ...
         real, estimate, abs(real - estimate));