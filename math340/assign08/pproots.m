function r = pproots(pp)

% Unpack information about the pieces of pp.
[breaks, coefs, L, k, ~] = unmkpp(pp);

% Allocate space for roots. There can be at most `k - 1` roots per
% subinterval, and there are `L` subintervals.
r = nan((k - 1) * L, 1);
ridx = 1;  % Current index into r.

% For the ith subinterval, find all the roots and add them.
for i = 1:L
    % Poly roots for a given piece of the spline, shifted by leftmost point
    % of subinterval.
    proots = roots(coefs(i, :)) + breaks(i);
    % Only add the roots that are real and in the interval.
    for proot = proots'
        if isreal(proot) && proot >= breaks(i) && proot <= breaks(i + 1)
            r(ridx) = proot;
            ridx = ridx + 1;
        end
    end
end

% Remove excess NaN values from r.
r(ridx:end) = [];
% Remove duplicates from r (also sorts r).
r = unique(r);
end