%% Zachary Kaplan
 % Math 440
 % HW3
 % 4/9/19
 
%% Problem 1
% We are using root locus method to find the stability region of BDF-3
 
% Linear granularity of discretization for curve plotting.
N = 1000;
% We are interested in values of g that cross |g| = 1, so we consider
% many values of g along that curve.
G = exp(1i * linspace(0, 2*pi, N));
% By hand, we found that f(g, z) = 0 can be explicity solved for z(g):
Z = 11/6 + 1/6*(-18./G + 9./(G.^2) -2./(G.^3));

% Lets plot the image of G under the equation f(g, z) = 0.
figure;
subplot(2, 1, 1);
plot(Z);

% We also found by hand the matrix for the recurrance relation in BDF-3,
% whose eigen values are our growth factors.
M = @(z) [ 18/(11-6*z), -9/(11-6*z), 2/(11-6*z) ; ...
           1          ,  0         , 0          ; ...
           0          ,  1         , 0         ];

% The graph clearly has two regions, one which contains -2,
% and the other which contains 2. We will use these values as test points.
for z = [-2 2]
    m = M(z);    % Get the matrix for test point.
    g = eig(m);  % Find the eigen values.
    num_lt_1 = sum(abs(g) < 1);  % Adding eps for float comparison.
    text(real(z), imag(z), ...
        sprintf('%d g''s\ns.t. |g| < 1', num_lt_1));
end

% Make sure everything is visible.
X_bounds = [-3 8];
Y_bounds = [-5 5];
axis([X_bounds Y_bounds])

% Title the plot for clarity.
title({'Stability Region for BDF-3', ...
       '(where there are 3 g''s s.t. |g| < 1)'});
   
% Add axis to help visualize.
hold on;
plot(complex(linspace(X_bounds(1), X_bounds(2), N)));
plot(1i*linspace(Y_bounds(1), Y_bounds(2), N));

legend('At least one g s.t. |g| = 1', 'Im z = 0', 'Re z = 0');

%% Problem 2
% Now we just want to plot a simple complex region for RK-4 stability.

% Bounds on region (guess and check until sufficient).
X_bounds = [-5 5];
Y_bounds = [-5 5];

% Precision desired (number of points per linear grid unit).
p = 1000;

% Create grid of points.
x = X_bounds(1):1/p:X_bounds(2);
y = Y_bounds(1):1/p:Y_bounds(2);
[X,Y] = meshgrid(x, y);
Z = X + 1i*Y;

% Function describing our growth factor as a function of z.
g = @(z) 1 + 1/6*(6*z + 3*z.^2 + z.^3 + z.^4/4);

subplot(2, 1, 2);
% Plot where the abs(g(Z)) < 1
plot(Z(abs(g(Z)) <= 1));

% Add axis to help visualize.
hold on;
plot(complex(linspace(X_bounds(1), X_bounds(2), N)));
plot(1i*linspace(Y_bounds(1), Y_bounds(2), N));

% Title the plot for clarity.
title('Stability Region for RK-4');
   
legend('SS_{RK-4}', 'Im z = 0', 'Re z = 0');

% Create a step function for when z is in ss_rk4 and not.
ss_rk4_step = zeros(size(Z));
ss_rk4_step(abs(g(Z)) <= 1) = 1;

% Label the points where the imaginary axis exits the region.
ind_plus = find(ss_rk4_step(y > 0, x == 0) == 0, 1) + sum(y <= 0);
ind_neg = find(ss_rk4_step(y < 0, x == 0) == 0, 1, 'last');

text(0, y(ind_plus), sprintf('%fi', y(ind_plus)));
text(0, y(ind_neg), sprintf('%fi', y(ind_neg)));