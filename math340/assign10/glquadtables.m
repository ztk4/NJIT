function quadtables = glquadtables(n)
% Returns the Gauss-Legendre quadrature tables with n points.
% Return value is a cell array of tables.

quadtables = cell(1, length(n));

for i = 1:length(n)
    % Hardcode empty tables for n < 2.
    if n(i) < 2
        quadtables{i} = [];
        continue;
    end
    
    
end