function dfx0 = diff1(f, x0, h, method)

if strcmp(method, 'forward')
    dfx0 = (f(x0 + h) - f(x0)) ./ h;
elseif strcmp(method, 'centered')
    dfx0 = (f(x0 + h) - f(x0 - h)) ./ (2*h);
else
    error('Unsupported different type %s', method);
end

end