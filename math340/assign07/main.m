%% Zachary Kaplan
 % MATH 340
 % Assignment 7
 % 3/2/18
 
%% Problem 1

f = @(x) 1./(1 + 10*x.^2);
degs = 0:40;
a = -1;
b = 1;
X = linspace(a, b, 1e3);

for type = {'newton', 'lagrange'}
    max_err = zeros(size(degs));
    
    for n = degs
        [p,points] = cheb(a, b, f, n, type{1});
        YP = p(X);
        YF = f(X);
        
        max_err(n - degs(1) + 1) = norm(YF - YP, Inf);
    end
    
    figure
    hold on;
    plot(X, YF);  % The function.
    plot(X, YP);  % The interpolation.
    scatter(points(:,1), points(:,2));  % Chebyshev points.
    title(sprintf('%d degree %s interpolation', n, type{1}));
    legend('1/(1 + 10*x^2)', 'Interpolation', 'Chebyshev Points')
    
    figure
    scatter(degs, log(max_err));
    title(sprintf('max error vs. degree of %s interpolation', type{1}));
    xlabel('Degree of Interpolation');
    ylabel('Log(Max Error)');
end

fprintf('Functions Used:\n');

dbtype cheb;
dbtype lpfit;
dbtype npfit;