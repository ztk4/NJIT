function [tn, y] = forward_euler(f, t0, y0, T, h)
% FORWARD_EULER computers the forward euler approx for y(T) given
%               that y' = f(t, y) and that y(t0) = y0 using a step size of
%               h.

% Generate t's.
tn = t0:h:T;
if (tn(end) ~= T)
    error('h does not divide (T - t0) evenly');
end

% Generate approximation.
y = zeros(size(tn));
y(1) = y0;
for i = 2:length(tn)
    y(i) = y(i-1) + h*f(tn(i-1), y(i-1));
end

end