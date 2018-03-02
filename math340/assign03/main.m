%% Zachary Kaplan
 % MATH 340
 % Assignment 3
 % 2/8/18
 
%% Problem 1

% Polynomial whose root is the golden ratio.
f1 = @(phi) phi.^2 - phi - 1;
[estimate, err] = bisection(f1, 1, 2, 1e-16);

fprintf(['Calculating the Golden Ratio:\n' ...
         '  Approx: %.15f\n' ...
         '  Error : %e\n'], estimate, err);
     
fprintf( '  Function Used:\n');
dbtype bisection
     
%% Problem 2

% Polynomial we are working with.
f2 = @(x) 816*x.^3 - 3835*x.^2 + 6000*x - 3125;

x0 = linspace(1.4, 1.7, 100);
alpha = arrayfun(@(x0) newtons(f2, x0, 1e-10), x0);

figure
scatter(x0, alpha, 10);
yticks([25/17 25/16 5/3]);
yticklabels({'25/17', '25/16', '5/3'});
xlabel('Intial Value x0');
ylabel('Root alpha');

fprintf('Function Used:\n');
dbtype newtons

%% Extra Credit

[r, err, order] = newtons(f1, 1, 1e-10);

fprintf(['When calculating the Golden Ratio using Newtons Method:\n' ...
         '  Approx: %f\n' ...
         '  Order : %d\n'], r, round(order));
fprintf( '  Function Used:\n');
dbtype newtons
