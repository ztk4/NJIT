%% Zachary Kaplan
 % Math 448
 % 12/6/18
 % HW 6
 
%% Setup 
% Seed the random generator so that consecutive runs produce the same
% results.
rng(448120618);

%% 9.12

% Common variables
n = 100;            % Sample size.
U = rand(1, n);     % Uniforms.
X = exp(U.^2);  % X = h(U) is our raw estimator.
Y = exp(U);     % Y = e^U is our control variate.

% Part b).
% Precalculated means.
Xbar = mean(X);
Ybar = mean(Y);
% First estimate c^*.
c_hat = - sum((X - Xbar) .* (Y - Ybar)) / sum((Y - Ybar).^2);

% Precalculate sample covariance.
cov_hat = sum((X - Xbar) .* (Y - Ybar)) / (n-1);
% Now estimate var of control variate estimator.
% NOTE: var normalizes by N-1 and is therefore a sample estimators.
control_var_hat = 1/n * (var(X) - cov_hat^2/var(Y));

fprintf('Control Variate: c^* ~= %f, var ~= %f\n', c_hat, control_var_hat);

% Part c).
% We just need to find the sample variance of n evaluations of the
% anithetic estimator.
antithetic_var_hat = var(exp(U.^2) .* (1 + exp(1 - 2*U)) / 2);

fprintf('Antithetic Variate: var ~= %f\n', antithetic_var_hat);

%% 9.18

% Common variables.
n = 1e2;

% e) - (RAW).

U1 = rand(1, n);
U2 = rand(1, n);

theta_raw = (2*norminv(U1) + norminv(U2) + 1) > 1;
fprintf('E[theta_raw] ~= %f, Var[theta_raw] ~= %f\n', ...
        mean(theta_raw), var(theta_raw));
    
% f) - (C)onditional.

U = rand(1, n);

theta_c = 1 - normcdf(norminv(U)/2);
fprintf('E[theta_c] ~= %f, Var[theta_c] ~= %f\n', ...
        mean(theta_c), var(theta_c));
    
% g) - (A)ntithetic (C)onditional (skipped b/c constant).

% h) - (C)ontrol (C)onditional.

U = rand(1, n);
X = 1 - normcdf(norminv(U)/2);  % The conditional estimator.
Y = norminv(U);                 % The control variate.

Xbar = mean(X);
Ybar = mean(Y);
c_hat = - sum((X - Xbar) .* (Y - Ybar)) / sum((Y - Ybar).^2);

% Actually evaluate the estimator n times to find sample mean and var.
theta_cc = X + c_hat*Y;

fprintf('E[theta_cc] ~= %f, Var[theta_cc] ~= %f\n', ...
        mean(theta_cc), var(theta_cc));