%% Zachary Kaplan
 % MATH 340
 % Assignment 10
 % 4/12/18

%% Problem 1

% Number of points to use.
figure
axes('xscale', 'log', 'yscale', 'log')
hold on
xlabel('Number of Points');
ylabel('Error');
n = 2:8;

% x ln(x)
f = @(x) x .* log(x);
[a, b] = deal(1, 2);
trueval = -3/4 + 2*log(2);

vals = glquad(f, a, b, n, []);

plot(n, abs(trueval - vals));

% x^3 / (x^2 + 1)
f = @(x) x.^3 ./ (x.^2 + 1);
[a, b] = deal(0, 1);
trueval = 1/2 - log(2)/2;

vals = glquad(f, a, b, n, []);

plot(n, abs(trueval - vals));

% x^8
f = @(x) x.^8;
[a, b] = deal(0, 1);
trueval = 1/9;

vals = glquad(f, a, b, n, []);

plot(n, abs(trueval - vals));

% sin(x)
f = @(x) sin(x);
[a, b] = deal(0, pi);
trueval = 2;

vals = glquad(f, a, b, n, []);

plot(n, abs(trueval - vals));

legend('\int_1^{ 2} x ln(x) dx', '\int_0^{ 1} x^3/(x^2 + 1) dx', ...
       '\int_0^{ 1} x^8 dx', '\int_0^{ \pi} sin(x) dx');
   
fprintf('Functions Used:\n');
dbtype glquad