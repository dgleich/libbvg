function test_main
% TEST_MAIN The main test driver for the bvgraph library.

try
    G = bvgraph('../data/wb-cs.stanford');
    fprintf('The library was already compiled!\n');
catch
    s = lasterror;
    if strcmp(s.identifier, 'bvgfun:notCompiled')
        fprintf('The library is now compiled!\n');
    else
        error('bvgraph:compileError','the library could not automatically compile!');
    end
end

G = bvgraph('../data/wb-cs.stanford');
y = pagerank(G);
d = diag(G);

n = size(G,1);

x1 = G*ones(n,1);
x2 = G'*ones(n,1);
x3 = ones(1,n)*G;

if ~isequal(x2,x3')
    error('bvgraph:test','there was an error multiplying two vectors');
end

G2 = bvgraph('../data/wb-cs.stanford',struct('offline',1));

n2 = size(G2,1);

y1 = G2*ones(n,1);
y2 = G2'*ones(n,1);
y3 = ones(1,n)*G2;

if ~isequal(x1,y1) || ...
   ~isequal(x2,y2) || ...
   ~isequal(x3,y3)
   error('bvgraph:test','there was an error with the offline graph');
end



    