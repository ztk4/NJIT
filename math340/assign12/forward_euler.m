function yT = forward_euler(f, t0, y0, T, h)
% FORWARD_EULER computers the forward euler approx for y(T) given
%               that y' = f(t, y) and that y(t0) = y0 using a step size of
%               h.

% Generate t's.
tn = t0:h:T;
if (tn(end) ~= T)
    error('h does not divide (T - t0) evenly');
end

% Generate approximation.
yT = y0;
for t = tn
    yT = yT + h*f(t, yT);
end

end