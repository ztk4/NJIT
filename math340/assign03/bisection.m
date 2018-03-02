function [estimate, err] = bisection(f, a, b, tol)

if f(a) < 0 && f(b) > 0
  neg = a;
  pos = b;
elseif f(a) > 0 && f(b) < 0
  neg = b;
  pos = a;
else
  error('The endpoints must differ in sign!');
end
n = 1;
while abs(pos - neg)/2 > tol
  mid = (pos + neg) / 2;
  v = f(mid);
  if v < 0
    neg = mid;
  elseif v > 0
    pos = mid;
  else
    break;  % We actually guessed it perfectly (within machine percision).
  end
  
  n = n + 1;
end

estimate = (pos + neg) / 2;
err = abs(pos - neg) / 2;
end