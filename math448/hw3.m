%% Zachary Kaplan
 % Math 448
 % 10/18/18
 % HW 3
 
%% 4.17

N = 10;  % The number of samples of X we desire.

% First let's calculate the first N values of the text's random sequence.
x = zeros(1, N);
x(1:2) = [ 23 66 ];  % x_1 and x_2 are hard-coded.
for n = 3:N
    x(n) = mod(3*x(n-1) + 5*x(n-2), 100);
end

% Then let's generate samples of X.
X = zeros(1, N);
for n = 1:N
    j = 1;
    f1 = 1/4;
    f2 = 4/9;
    p_cum = f1 + 3/8 * f2;
    U = x(n)/100;
    
    while U > p_cum
        f1 = 1/2 * f1;
        f2 = 2/3 * f2;
        p_cum = p_cum + f1 + 3/8 * f2;
        j = j + 1;
    end
    
    X(n) = j;
end

% Display our samples of X.
X