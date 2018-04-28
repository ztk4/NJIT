function [tn, y] = backward_euler(g, t0, y0, T, h)
% BACKWARD_EULER computers the backwards euler approx for y(T) where
%                that y_{n+1} = g(t_n, y_n, h) and that y(t0) = y0 using a
%                step size of h.

% Generate t's
tn = t0:h:T;
if tn(end) ~= T
    error('h does not divide (T - t0) evenly')
end

% Generate approximation.
y = zeros(size(tn));
y(1) = y0;
for i = 2:length(tn)
    y(i) = g(tn(i-1), y(i-1), h);
end

end