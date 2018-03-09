%% Zachary Kaplan
 % MATH 340
 % Assignment 8
 % 3/15/18
 
%% Problem 1

f = @(x) exp(-x) .* cos(x);
df = @(x) -exp(-x) .* (cos(x) + sin(x));
a = -4;
b = 5;

% Approximate f
chebf = cheb(a, b, f, 200);

% Evaluate on interval, take numerical derivative
X = linspace(a, b, 1e5);
Y = chebf(X);
dfdx = gradient(Y) ./ gradient(X);
DF = df(X);

fprintf(['Relative L^{inf} norm : %e\n' ...
         'Relative L^2 norm     : %e\n'], ...
         norm(DF - dfdx, Inf)/norm(DF, Inf), ...
         norm(DF - dfdx, 2) / norm(DF, 2));
     
fprintf('\nFunctions used:\n');
dbtype cheb
dbtype lpfit
     
%% Problem 2

f = @(x) exp(-x) .* cos(x);
% NOTE: The additional 1/2 was chosen so that the integral was based at 0,
%       since e^{-x}(sin(x) - cos(x)) is -1/2 at a.
F = @(x) (exp(-x) .* (sin(x) - cos(x)) + 1) ./ 2;
a = 0;
b = 12*pi;

% Approximate f
chebf = cheb(a, b, f, 200);

% Evaluate on interval, take numerical integral.
X = linspace(a, b, 1e5);
Y = chebf(X);
FN = cumtrapz(X, Y);
FA = F(X);

fprintf(['Relative L^{inf} norm : %e\n' ...
         'Relative L^2 norm     : %e\n'], ...
         norm(FN - FA, Inf)/norm(FA, Inf), ...
         norm(FN - FA, 2) / norm(FA, 2));
     
fprintf('\nAbsolute error at b = 12pi: %e\n', abs(FN(end) - FA(end)));

fprintf('\nFunctions used:\n');
dbtype cheb
dbtype lpfit

%% Problem 3

