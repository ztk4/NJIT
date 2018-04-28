%% Zachary Kaplan
 % MATH 340
 % Assignment 13
 % 4/27/18

%% Problem 1

% f where y' = f(t, y).
f = @(t, y) 10*(1 - y);
% g where y_{n+1} = g(t_n, y_n, h), solved explicity from backward euler's.
g = @(t, y, h) (y + 10*h) / (1 + 10*h);
% Initial point.
[t0, y0] = deal(0, 1/2);
% Final t value.
T = 1;
% Step size.
h = 1/5;
% Actual solution.
y = @(t) 1 - exp(-10*t)/2;

% Approximation.
[tf, yf] = forward_euler(f, t0, y0, T, h);
[tb, yb] = backward_euler(g, t0, y0, T, h);

figure
hold on
plot(tf, yf);       % Forward
plot(tb, yb);       % Backward
plot(tf, y(tf));    % Exact
legend('Forward Approx', 'Backward Approx', 'Exact Solution');
xlabel('t')
ylabel('y')
title('Solutions to y'' = 10(1-y)');
set(get(gca, 'ylabel'), 'rotation', 0);

fprintf('Functions Used:\n');

dbtype forward_euler
dbtype backward_euler