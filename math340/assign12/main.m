%% Zachary Kaplan
 % MATH 340
 % Assignment 12
 % 4/26/18

%% Problem 1

f = @(t, y) cos(t) * exp(sin(t));
[t0, y0] = deal(0, 1);
T = 2*pi;
yT_trueval = exp(sin(T));

% Values of h to use.
num_h = 8;
hn = (T - t0) .* logspace(0, -num_h + 1, num_h);
yn = zeros(size(hn));

for i = 1:length(hn)
    yn(i) = forward_euler(f, t0, y0, T, hn(i));
end


figure
axes('xscale', 'log', 'yscale', 'log', 'xdir', 'reverse');
hold all
plot(hn, abs(yn - yT_trueval));   % Error.
plot(hn, hn);                     % Linearly decreasing function in h.
xlabel('Step Size (h)')
legend('Error of y_h and y(2*\pi)', 'Linear function (for comparison)');

fprintf('Funtion used:\n');
dbtype forward_euler
