%% Zachary Kaplan
 % MATH 340
 % Assignment 5
 % 2/22/18

 
%% Part 1

orders = 3:12;
f = @(x) 1./(1 + 10*x.^2);
max_err = zeros(size(orders));

for n = orders
    X = linspace(-1, 1, n + 1);
    Y = f(X);
    
    p = pfit(X, Y);
    
    figure
    hold on
    title(sprintf('Order %d Interpolation', n));
    scatter(X, Y);  % The points.
    XN = linspace(-1, 1, 1e3);
    YP = p(XN);
    YF = f(XN);
    plot(XN, YP);   % The interpolation
    plot(XN, YF);   % The function.
    legend('Sample Points', 'Interpolation', '1/(1 + 10*x^2)')
    
    max_err(n - orders(1) + 1) = norm(YP - YF, Inf);
    fprintf('Error for Order %2d: %e\n', n, max_err(n - orders(1) + 1));
end

figure
plot(orders, max_err);
title('Error vs. Order');
ylabel('Maximum Error');
xlabel('Order of Interpolation');

fprintf(['\n'...
         'Looks like the error would increase dramatically \n'...
         'as n increases. It also appears that the error is \n'...
         'significantly worse on even-powered terms\n\n']);

fprintf('Functions Used:\n')
dbtype pfit

%% Part 2
load('DataA')
load('DataB')
load('DataC')
points = vertcat(A, B, C);

p = pfit(points(:, 1), points(:, 2));
fprintf('The secret: %f\n', p(0));