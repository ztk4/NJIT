%% Zachary Kaplan
 % Math 340
 % Assignment 11
 % 4/13/18
 
%% Problem 1

f = @(x) exp(-x) .* cos(x);
df = @(x) -exp(-x) .* (sin(x) + cos(x));
n = 0:8;
h = 0.5 .^ n;

% Calculate derivatives about pi/3.
x0 = pi/3;
df_trueval = df(x0);
df_forward = zeros(size(n));
df_centered = zeros(size(n));
for i = 1:length(n)
    df_forward(i) = diff1(f, x0, h(i), 'forward');
    df_centered(i) = diff1(f, x0, h(i), 'centered');
end

err_forward = abs(df_forward - df_trueval);
err_centered = abs(df_centered - df_trueval);
     
% Plotting the error.
figure
axes('yscale', 'log');
hold on
title('Forward Difference Method');
plot(n, err_forward);
plot(n, h.^1);
xlabel('n = log2(h)');
legend('Error', 'h^1');

figure
axes('yscale', 'log');
hold on
title('Ceneterd Difference Method');
plot(n, err_centered);
plot(n, h.^2);
xlabel('n = log2(h)');
legend('Error', 'h^2');

% Approximate using forward difference over [0, 2pi].
h = 2^-40;
X = linspace(0, 2*pi, 1e6);

df_forward = diff1(f, X, h, 'forward');

figure
hold on
title('d/dx[e^{-x} cos(x)]');
plot(X, df(X));
plot(X, df_forward);
legend('True Derivative', 'Forward Approximation', 'Location', 'southeast');

%% Problem 2


