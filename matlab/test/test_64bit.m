function rval = test_64bit()
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
assert(all(x+y == summation), 'failed to correctly read head-tail graph');
if all(x+y == summation)
    rval = 0
else
    rval = 1
end
end