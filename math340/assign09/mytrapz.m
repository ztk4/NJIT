function est = mytrapz(f, a, b, N)

X = linspace(a, b, N + 1);

est = (b - a)/(2*N) * ...
      (f(X(1)) + 2*sum(arrayfun(f, X(2:end-1))) + f(X(end)));
end