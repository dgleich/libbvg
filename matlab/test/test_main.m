function test_main
% TEST_MAIN The main test driver for the bvgraph library.

%
% David Gleich
% 3 September 2007
% Copyright, Stanford University, 2007
%

msgid = 'bvgraph:test';

%% Make sure the library is compiled

try
    G = bvgraph('../../data/wb-cs.stanford');
    fprintf('The library was already compiled!\n');
catch
    s = lasterror;
    if strcmp(s.identifier, 'bvgfun:notCompiled')
        fprintf('The library is now compiled!\n');
    else
        error('bvgraph:compileError','the library could not automatically compile!');
    end
end

%% Test the code documentation

G = bvgraph('../../data/wb-cs.stanford');
A = sparse(G);
x = ones(size(A,1),1);
if norm(G*x - A*x) > eps(1)
    error(msgid, 'the sparse routine example failed');
end

%% Additional tests



G = bvgraph('../../data/wb-cs.stanford');
y = bvpagerank(G);
d = diag(G);

n = size(G,1);

x1 = G*ones(n,1);
x2 = G'*ones(n,1);
x3 = ones(1,n)*G;

if ~isequal(x2,x3')
    error(msgid,'there was an error multiplying two vectors');
end

G2 = bvgraph('../../data/wb-cs.stanford',struct('offline',1));

n2 = size(G2,1);

y1 = G2*ones(n,1);
y2 = G2'*ones(n,1);
y3 = ones(1,n)*G2;

if ~isequal(x1,y1) || ...
   ~isequal(x2,y2) || ...
   ~isequal(x3,y3)
   error(msgid,'there was an error with the offline graph');
end

if ~isa(G,'bvgraph')
    error(msgid, 'bvgraph did not report the correct type');
end

if ~isa(G,'bvgraph')
    error(msgid, 'bvgraph did not pretend to be a logical');
end

nz = nnz(G);
A = sparse(G);
nz2 = nnz(A);
if nz ~= nz2 
    error(msgid, 'bvgraph did not return the correct number of non-zeros');
end

%% Test the istrans function
Gt = G';
if ~istrans(Gt)
    error(msgid,'istrans reported the incorrect transpose state');
end

if istrans(G)
    error(msgid,'istrans reported the incorrect transpose state');
end


%% Test the sum function
A = sparse(G);

d_A = sum(A);
d_G = sum(G);

d_A_1 = sum(A,1);
d_G_1 = sum(G,1);

d_A_2 = sum(A,2);
d_G_2 = sum(G,2);

if ~isequal(d_A, d_G)
    error(msgid,'sum returned incorrect results');
end

if ~isequal(d_A_1, d_G_1)
    error(msgid,'sum1 returned incorrect results');
end

if ~isequal(d_A_2, d_G_2)
    error(msgid,'sum2 returned incorrect results');
end

Gt = G';
At = sparse(Gt);

d_A = sum(At);
d_G = sum(Gt);

d_A_1 = sum(At,1);
d_G_1 = sum(Gt,1);

d_A_2 = sum(At,2);
d_G_2 = sum(Gt,2);

if ~isequal(d_A, d_G)
    error(msgid,'sum returned incorrect results');
end

if ~isequal(d_A_1, d_G_1)
    error(msgid,'sum1 returned incorrect results');
end

if ~isequal(d_A_2, d_G_2)
    error(msgid,'sum2 returned incorrect results');
end

%% Test the stochastic mult function

rand('state',0);
x = rand(n,1);
G = bvgraph('../../data/wb-cs.stanford');
A = sparse(G);

% create the substochastic matrix corresponding to a random walk on G/A
[i j v] = find(A);
d = sum(A,2);
P = sparse(i, j, v(i)./d(i), size(A,1), size(A,2));

y_P = P*x;
y_G = substochastic_mult(G,x);

if norm(y_P-y_G,inf) > 100*eps(1)
    error(msgid,'stochastic_mult results are not correct to 100*eps');
end

y_Pt = P'*x;
y_Gt = substochastic_mult(G',x);

if norm(y_Pt-y_Gt,inf) > 100*eps(1)
    error(msgid,'stochastic_mult(tranpose) results are not correct to 100*eps');
end

    