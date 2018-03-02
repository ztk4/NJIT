%% Zachary Kaplan
 % MATH 340
 % Assignment 4
 % 2/15/18
 
%% Problem 1

f  = @(x)   x.^6 +   x.^3 - 1;
df = @(x) 6*x.^5 + 3*x.^2;
tol = 1e-6;

n = 100;
A = zeros(2*n + 1, 2*n + 1);
X = linspace(-1, 1, 2*n + 1);  % Real parts
Y = linspace(-1, 1, 2*n + 1);  % Complex parts
for a = 1:0.2:1.8  % My computer catches at a = 2...
  for j = 1:2*n + 1
    for k = 1:2*n + 1
      z0 = X(j) + Y(k)*1i;
      A(j, k) = complex_newtons(f, df, z0, a, tol);
    end
  end
  dcolor(X, Y, A);
  pause(0.1);
end

fprintf('The function used:\n');
dbtype complex_newtons;

%% Problem 2

f = @(x) exp(x)-1;
tol = 1e-7;
[root, err, order] = secant(f, -1, 1, tol);

fprintf(['Secant method for approximating the root of e^x - 1:\n' ...
         '  Approx : %.8f\n' ...
         '  Error  : %e\n' ...
         '  Order  : %f\n'], root, err, order);
     
fprintf('The function used:\n');
dbtype secant;