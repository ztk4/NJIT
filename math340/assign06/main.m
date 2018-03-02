%% Zachary Kaplan
 % Math 340
 % Assignment 6
 % 3/1/18
 
%% Part 1

order = 3:12;
f = @(x) 1./(1 + 10*x.^2);
max_err = zeros(size(order));

for n = order
    X = linspace(-1, 1, n + 1);
    Y = f(X);
    
    p = pfit(X, Y);
    
    figure
    hold on
    title(sprintf('Order %d Interpolation', n));
    scatter(X, Y);  % The points.
    XN = linspace(-1, 1, 1e3);  % Domain
    YP = p(XN);
    YF = f(XN);
    plot(XN, YP);   % The interpolation
    plot(XN, YF);   % The function.
    legend('Sample Points', 'Interpolation', '1/(1 + 10*x^2)')
    
    max_err(n - order(1) + 1) = norm(YP - YF, Inf);
    fprintf('Max Error for Order %2d: %e\n', n, max_err(n - order(1) + 1));
end

figure
plot(order, max_err);
title('Error vs. Order');
ylabel('Maximum Error');
xlabel('Order of Interpolation');

fprintf('\nFunctions used:');
dbtype pfit

%% Part 2

n = 50;
f = @(x) 1./(1 + 10*x.^2);

X = linspace(-1, 1, n);
Y = f(X);

pp = spline(X, Y);

figure
hold on
title(sprintf('Spline with %d points', n));
scatter(X, Y);  % The points.
XN = linspace(-1, 1, 1e3);  % Domain
YP = ppval(pp, XN);
YF = f(XN);
plot(XN, YP);   % The spline.
plot(XN, YF);   % The function.
legend('Sample Points', 'Spline', '1/(1 + 10*x^2)')

max_err = norm(YP - YF, Inf);
fprintf('Max Error for Spline with %d points: %e\n', n, max_err);