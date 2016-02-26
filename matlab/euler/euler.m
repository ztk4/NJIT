% Zachary Kaplan
% Differential Equations
% MatLab Assignment 1 (Program for Part 2 Only)
% 2/26/2016

function euler()
%euler drives the euler approximation of y' = ode (defined below)
for y0 = [0.5 1]                        %all y0 in {.5, 1} crossed with...
    for h = [0.025 0.05 0.1 0.2]        %all h  in {.025, .05, .1, .2}
        clear y;                        %clear in case
        y = eulerODE(0, 5, h, y0, @ode);%get soln vector
        fprintf(['Approximation for y(0) = %.1f and h = %.3f:'...
                 '\ty(5) = % f\n'], y0, h, y(end))
    end
end

end

function y = eulerODE(t0, tf, h, y0, yp)
%eulerODE Evaluates an euler approx of an ODE
%   t0 - time initial (inclusive)
%   tf - time final (inclusive)
%   h  - step size on t axis
%   y0 - intial y value; y(t0)
%   yp - the ODE as a function of y and t; y'(y, t)
%   returns - vector with soln value for all times in [t0:h:tf]

%generate all times and set y0
t = [t0:h:tf];
y(1) = y0;

%loop over all indexes in t (and y)
for n = [2:length(t)]
    %set y_n = y_(n-1) + y'(y_(n-1), t_(n-1))*h => do the approx
    y(n) = y(n-1) + yp(y(n-1), t(n-1))*h;
end
    
end

function yp = ode(y, t)
%ode returns y' at a given y and t
yp = (1 + sin(t))/5 * y - 1/5;
end
