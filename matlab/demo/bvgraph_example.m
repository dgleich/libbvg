%% A simple example using the bvgraph class
% This sample will demonstrate the bvgraph class.  It will show a few
% features and limitations of the class for working with these large graph
% files in Matlab.

%% bvgraph background
% The BV graph compression scheme and the *.graph/BVGraph file type are
% data structures and techniques created to work with extremely large web
% graph matrices.  Many researchers observed that these matrices have a
% self-similar structure and exhibit considerable index locality.  Both of
% these features are exploited by the BV graph compression scheme to store
% these large webgraph matrices at only a few bits per edge.  
%
% Paolo Boldi and Sebastiano Vigna wrote the initial BVGraph implementation
% in Java.  While Matlab has nice features to work with Java, these
% features do not scale to working with extremely large vectors.
% Consequently, this Matlab package re-implements the procedures
% necessary to _read_ a BVGraph file in C.  Further, it provides routines
% to operate on these files and makes them look like a Matlab matrix.  The
% goal is to be as efficient as possible and support computing the PageRank
% vector for extremely large graphs in Matlab.  

%% Load a graph
% Loading a graph with the bvgraph class is simple.  
G = bvgraph('../data/wb-cs.stanford')

%% 
% There is a slight caveat about loading.  The file with most of the data
% for the bvgraph file is ../data/wb-cs.stanford.graph.  However, you must
% load the file without the .graph extension.  For example, you'll get a
% slightly helpful error if you accidentally forget this.  (This may be
% changed in a future version.)

G2 = bvgraph('../data/wb-cs.stanford.graph');

%% Loading offline or online
% There are two ways of loading a bvgraph file: streaming mode (offline) or
% inmemory (online).  The difference is that when a file is loaded in
% streaming mode, the class does not actually load the .graph file into
% memory, but iterates over the file on the disk for every operation.  This
% is slower, but hardly uses any memory on the computer.  For example,

G = bvgraph('../data/wb-cs.stanford');
Goff = bvgraph('../data/wb-cs.stanford',struct('load_type','offline'));

whos

%%
% Here we find that Goff takes only 3k whereas G takes 28k of memory.  In
% this case, it seems silly to avoid loading 25k of memory to get the
% offline operations.  However, for larger graphs, the difference is much
% more pronounced.  Goff always takes about 3k, where G might takes
% hundreds of megabytes.

%% Some simple operations
% The bvgraph class in Matlab supports the following simple operations
size_G = size(G)
nzcount = nnz(G)
d = diag(G);
s1 = sum(G);
s2 = sum(G,2);

%%
% Let's use an operation to get the size of the matrix.

n = size(G,1);

%% Converting to a native Matlab matrix
% If the bvgraph file is small enough to fit into Matlab's memory as a
% Matlab sparse matrix, the sparse call will convert it.  

A = sparse(G);

%% Transposes
% The bvgraph class implements an implicit transform.  That is, taking the
% transpose of a matrix does _nothing_ to the underlying data and only
% changes the interpretation of the data in the Matlab interface.

Gt = G';
isequal(sum(Gt,1)',sum(G,2))

%%
% Because only the interpretation of the data is transposed, the istrans
% function reveals this state

istrans(G)
istrans(G')

%% PageRank opertions
% The main goal with implementing the BVGraph format inside of Matlab is to
% support computing PageRank on really large graphs.  The key computation
% in PageRank is the sub-stochastic matrix vector multiplication
% 
% $$y = (D^+ A)^T x$$
% 
% where x is the current iterate, D is the diagonal matrix of degrees, and 
%
% $$D^+ A$$
%
% is the substochastic matrix corresponding to a random walk on the graph
% for adjacency matrix A.  The bvgraph class supports a specialized method
% to compute this product without storing a vector for the diagonal of D.
% However, the substochastic_mult function is a little quirky.

% create the substochastic matrix corresponding to a random walk on A
[i j v] = find(A);
d = sum(A,2);
P = sparse(i, j, v(i)./d(i), size(A,1), size(A,2));
% now P = D^+ A;
x = ones(n,1);
y_G = substochastic_mult(G,x);
y_P = P*x;
norm(y_G-y_P,inf)

%%
% When applied to G', the substochastic_mult function computes the
% transpose of the sub-stochastic product.

y_Gt = substochastic_mult(G',x);
y_Pt = P'*x;
norm(y_Gt-y_Pt,inf)

%%
% Since one goal is PageRank, the bvgraph class has a specific PageRank
% method.  

help bvpagerank

x = bvpagerank(G);

%%
% As noted in the documentation, this method works on the data as 
% _on disk_ and removes any transpose operations performed on the matrix.

y = bvpagerank(G');

norm(x-y,inf)

%%
% To call PageRank on the tranpose (which takes additional memory, see the
% documentation oin the bvpagerank function.

%% What isn't implemented (yet)
% Currently, we do not support indexed accesses to the bvgraph type, so the
% following feature will fail

G(1,2)

%%
% Additionally, the multiplication operation is only supported for a double
% type.

x = true(size(G,1),1);
G*x

%%
% Finally, there is no way to output a Matlab matrix as a bvgraph file.
