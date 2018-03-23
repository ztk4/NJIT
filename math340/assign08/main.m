%% Zachary Kaplan
 % MATH 340
 % Assignment 8
 % 3/22/18
 
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

%% Extra Credit

% Get X and Y values from data file.
load data.mat
[X, Y] = deal(data(1, :), data(2, :));
[a, b] = deal(-7, 7);
delta = (b - a) * 1e-2;  % Default delta deviating from true value of root.
tol = 1e-12;  % Tolerance for numerical root finding.

% Calculate the spline piecewise polynomial
pp = spline(X, Y);

% Finds all roots of the spline (in sorted order).
r = pproots(pp);
dr = diff(r);
% Require that delta is smaller than the minimum distance between roots.
if not(isempty(dr))
    if delta > min(dr)
        delta = min(dr) / 2;
    end
end

% Allocate storage for convergence of numerical methods.
% Secent method using [r - delta, r + delta].
secent_order = zeros(size(r));
% Newtons method starting at r - delta.
newton_order_neg = zeros(size(r));
% Newtons method starting at r + delta.
newton_order_pos = zeros(size(r));

% Approximate each root with secant and newtons method.
for i = 1:length(r)
    root = r(i);
    
    % SECANT METHOD:
    x0 = max(a, root - delta);
    x1 = min(b, root + delta);
    [est, err, secent_order(i)] = secant(@(x) ppval(pp, x), x0, x1, tol);
    if abs(est - root) > err
        error('Secent method failed to converge to root %f\n', root);
    end
    
    % NEWTON METHOD:
    % Exact derivative of spline.
    dpp = fnder(pp);
    % From Below.
    [est, err, newton_order_neg(i)] = ...
        newtons(@(x) ppval(pp, x), @(x) ppval(dpp, x), x0, tol);
    if abs(est - root) > err
        error(['Newton''s method failed to converge to root %f' ...
               'from below\n'], root);
    end
    % From Above.
    [est, err, newton_order_pos(i)] = ...
        newtons(@(x) ppval(pp, x), @(x) ppval(dpp, x), x1, tol);
    if abs(est - root) > err
        error(['Newton''s method failed to converge to root %f' ...
               'from above\n'], root);
    end
end

figure
hold on
scatter(r, secent_order);
scatter(r, newton_order_neg);
scatter(r, newton_order_pos);
xlabel('Root of the Spline');
ylabel('Rate of Convergence');
legend('Secant Method', 'Newton''s Method From Below', ...
       'Newton''s Method From Above', 'Location', 'eastoutside');
   
fprintf(['By inspection of the above, Newton''s method from the ' ...
         'positive side of -2 converges at a rate of ~1,\nwhich is ' ...
         'unusual compared to the expected Newton rate of convergence ' ...
         'of 2.\n']);
     
fprintf('\nFunctions Used:\n');

dbtype pproots
dbtype secant
dbtype newtons