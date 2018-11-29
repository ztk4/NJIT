%% Zachary Kaplan
 % Math 448
 % 11/29/18
 % HW 5
 
%% Setup 
% Seed the random generator so that consecutive runs produce the same
% results.
rng(448112918);
 
%% 8.4

runs = 1e5;
N = zeros(1, runs);
bound = 0.1;
for i = 1:runs
    % Generate normals [the below is equiv to norminv(rand(1, n))].
    n = 100;               % Current sample size.
    Z = randn(1, n);       % Normals.
    sum_z = sum(Z);        % Current sum.
    sum_sq_z = sum(Z.^2);  % Current sum of squares.
    
    % The below condition is a rearrangement of the negation of
    % S_n/sqrt(n) < 0.1
    while sum_sq_z - (sum_z^2)/n >= bound^2 * n*(n-1)
        z = randn(1, 1);   % Generate a new normal.
        % Accumulate.
        sum_z = sum_z + z;
        sum_sq_z = sum_sq_z + z^2;
        % Increment sample size.
        n = n + 1;
    end
    
    N(i) = n;
end

% Not really required but I was curious.
% Constructs a CI about our estimate by dividing the values into 40 samples
% and using the variance of their means.
% This is a two-sided 95% CI.
batches = 40;
bsize = runs/batches;
Nb = zeros(1, batches);
for b = 1:batches
    Nb(b) = mean(N((b-1)*bsize + 1:b*bsize));
end
fprintf('CI(95%%) for n: %f +/- %f\n', ...
        mean(N), tinv(1-(1-.95)/2, batches - 1)*std(Nb)/sqrt(batches));

% Regular Output
fprintf('Sample Mean of n = %f, Sample Variance of n = %f\n', ...
        mean(N), var(N));
fprintf('The last trial had n = %d, mu = %f, and sigma^2 = %f\n', ...
        n, sum_z/n, (sum_sq_z - sum_z^2/n)/(n-1));
    
%% 8.5

runs = 1e3;
N = zeros(1, runs);
bound = 0.01;
for i = 1:runs
    % Generate normals [the below is equiv to norminv(rand(1, n))].
    n = 100;               % Current sample size.
    Z = randn(1, n);       % Normals.
    sum_z = sum(Z);        % Current sum.
    sum_sq_z = sum(Z.^2);  % Current sum of squares.
    
    % The below condition is a rearrangement of the negation of
    % S_n/sqrt(n) < 0.01
    while sum_sq_z - (sum_z^2)/n >= bound^2 * n*(n-1)
        z = randn(1, 1);   % Generate a new normal.
        % Accumulate.
        sum_z = sum_z + z;
        sum_sq_z = sum_sq_z + z^2;
        % Increment sample size.
        n = n + 1;
    end
    
    N(i) = n;
end

% Not really required but I was curious.
% Constructs a CI about our estimate by dividing the values into 40 samples
% and using the variance of their means.
% This is a two-sided 95% CI.
batches = 40;
bsize = runs/batches;
Nb = zeros(1, batches);
for b = 1:batches
    Nb(b) = mean(N((b-1)*bsize + 1:b*bsize));
end
fprintf('CI(95%%) for n: %f +/- %f\n', ...
        mean(N), tinv(1-(1-.95)/2, batches - 1)*std(Nb)/sqrt(batches));

% Regular Output.
fprintf('Sample Mean of n = %f, Sample Variance of n = %f\n', ...
        mean(N), var(N));
fprintf('The last trial had n = %d, mu = %f, and sigma^2 = %f\n', ...
        n, sum_z/n, (sum_sq_z - sum_z^2/n)/(n-1));
    
%% 8.15

% Data values
X = [5, 4, 9, 6, 21, 17, 11, 20, 7, 10, 21, 15, 13, 16, 8];
n = length(X);      % Size of vector.
R = 1e5;            % Number of evaluations of g.
g = @(X) var(X);    % The function that computes our estimator on a sample.
% Vectorized inverse of the emperical CDF Fe.
% Takes a vectors of uniforms and returns a vectors of samples ~ Fe.
Fe_inv = @(U) X(ceil(n*U));  % Recall matlab is 1-indexed.
% The true variance of Fe_inv.
theta_fe = var(X, 1);  % NOTE: the 1 normalizes by N (instead of N-1).

mse_ests = zeros(1, R);
for r = 1:R
    mse_ests(r) = (g(Fe_inv(rand(1, n))) - theta_fe)^2;
end

fprintf('MSE_Fe Estimate: %f\n', mean(mse_ests));