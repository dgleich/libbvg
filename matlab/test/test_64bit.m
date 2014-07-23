function test_64bit
% TEST_64BIT tests matlab interface for 64 bit integers
%% Make sure the library is compiled

try
    G = bvgraph('../../test/bv_head_tail_1000');
    fprintf('The library was already compiled!\n');
catch
    s = lasterror;
    if strcmp(s.identifier, 'bvgfun:notCompiled')
        fprintf('The library is now compiled!\n');
    else
        error('bvgraph:compileError','the library could not automatically compile!');
    end
end 
G = bvgraph('../../test/bv_head_tail_1000');
display(G);
A = sparse(G);
[x, y] = find(A);
n = size(x, 1);
summation = x(1)+y(1);
rval = any(x + y - summation .* ones(n, 1));
if rval != 0
    error('head tail summation not correct');
end
end