%% Zachary Kaplan
 % Math 448
 % 10/4/18
 % HW 2
 
%% 3.2

xn = zeros(1, 11);
xn(1) = 3;  % NOTE: matlab uses 1-indexing, so xn(i) = x_{i-1}.
for i = 1:10
    xn(i + 1) = mod(5*xn(i) + 7, 200);
end

% Display x_1 through x_10.
xn(2:end)
 
%% 3.3

t = 6.31656;
N = 1e6;
U = rand(1, N);
G = exp(exp(U));

theta_N = mean(G)
err = abs(theta_N - t)

%% 3.6

t = 1/2;
N = 1e6;
U = rand(1, N);
g = @(x) x ./ ((1 + x.^2).^2);
G = g(1./U - 1);

theta_N = mean(G./(U.^2))
err = abs(theta_N - t)

%% 3.11

N = 1e6;
U = rand(1, N);
Xa = U;
Xb = U.^2;
Y = sqrt(1 - U.^2);
mu_xa = mean(Xa);
mu_xb = mean(Xb);
mu_y = mean(Y);
sigma_xa = std(Xa);
sigma_xb = std(Xb);
sigma_y = std(Y);

theta_Na = mean((Xa - mu_xa) .* (Y - mu_y)) / (sigma_xa * sigma_y)
theta_Nb = mean((Xb - mu_xb) .* (Y - mu_y)) / (sigma_xb * sigma_y)

%% 3.13

M = 1e6;
N = arrayfun(@(~) sample_N, 1:M);

theta_M = mean(N)
for i = [0 1 2 3 4 5 6]
    fprintf('theta^%d_M = \n\n    %f\n\n', i, mean(N == i))
end

%% Helper Functions

function N = sample_N()
    cumm_prod = 1;
    i = 0;
    while cumm_prod >= exp(-3)
        cumm_prod = cumm_prod * rand;
        i = i + 1;
    end
    
    N = i - 1;
end