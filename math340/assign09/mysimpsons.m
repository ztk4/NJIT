function est = mysimpsons(f, a, b, N)

X = linspace(a, b, N + 1);

evens = sum(arrayfun(f, X(3:2:end-2)));
odds  = sum(arrayfun(f, X(2:2:end-1)));

est = (b - a)/(3*N) * (f(X(1)) + 2*evens + 4*odds + f(X(end)));
end