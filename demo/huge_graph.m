%% Using *HUGE* Graphs in Matlab
% This example will demonstrate running PageRank on a 110M node graph with
% 1B edges.  The system is a 64-bit Dell Linux machine with 2GB of RAM.  You 
% _must_ have 2GB of RAM to run this example.  Also, you
% must download the webbase-2001.graph and webbase-2001.properties files from:
%   http://law.dsi.unimi.it/index.php?option=com_include&Itemid=65
% Note that this example will not work on a 32-bit version of Matlab running 
% on Windows, even on a machine with 2 GB of RAM.  I haven't tested it on a 
% 32-bit version of Linux.  

%% Load the graph
% The first step is to load the graph.  The .graph file is about 300 MB, 
% so we are going to stream in from disk, or load it in 'offline' mode.

%%
% Set the following path to the location of the webbase-2001.graph file
% on your machine
if ~exist('webbase_path','var')
    webbase_path = [];
end

G = bvgraph([webbase_path 'webbase-2001'],struct('load_type','offline'));

%%
% Let's check some simple properties of the graph.

n = size(G,1);
nz = nnz(G);
s = sum(G,2);   % compute the sum to get dangling nodes
s = s == 0;
d = diag(G);    % look at the diagonal to count self-loops

fprintf('The graph has\n');
fprintf('  %10i vertices,\n', n);
fprintf('  %10i edges,\n', nz);
fprintf('  %10i dangling nodes, and\n', sum(s));
fprintf('  %10i self-loops.\n',sum(d));

%%
% Let's see how much memory those operations took.

whos

%%
% At the moment, we are using quite a bit of memory!  When working with
% these large graph files, we need to be very careful not to exceed the 
% amount of memory on the system.  If we do, then Matlab will start 
% _swapping_ or writing memory to disk, which will cause the program to 
% run unreasonably slow.  

%% Check native memory use
% Just for fun, let's see how much memory it would require to load G 
% as a Matlab matrix.  On a 32-bit platform, a Matlab sparse matrix
% requires 4 bytes per row, and 5 bytes per non-zero.  On a 64-bit 
% platform, it requires 8 bytes per row, and 9 bytes per non-zero.  

matlab_32_mem = 4*(size(G,1)+1) + 5*nnz(G);
matlab_64_mem = 8*(size(G,1)+1) + 9*nnz(G);

fprintf('For an in-memory matrix, Matlab would need\n');
fprintf('  %10i bytes on a 32-bit machine\n', matlab_32_mem);
fprintf('  %10i bytes on a 64-bit machine\n', matlab_64_mem);

%%
% Wow, that's a lot of memory! 

%% Check PageRank memory use
% The PageRank computation using the Power Method requires 2 vectors of 
% storage.  Let's make sure we can fit everything into memory.  A matlab
% vector takes 8 bytes per row.  

pagerank_required_mem = (8*size(G,1))*2;

fprintf('The bvpagerank operation requires\n');
fprintf('  %10i bytes of memory\n', pagerank_required_mem);

%% 
% The operation takes about 1.9 GB of memory.  This requirement is why 
% this example requires a machine with 2 GB of memory.  Also, we could
% not run this example if we had loaded the graph into memory, because 
% the graph file takes 300 MB of memory itself.  If you have a machine
% with more than 2 GB of memory, check and see how much faster the code
% will run with the matrix in memory.

%% Computing PageRank
% With all of that preamble gone, all that is left to do is call
% the bvpagerank function.  Before we do this, however, we first clear
% all the existing vectors.

clear s d;
% WARNING: This takes about 2 hours!
x = bvpagerank(G);

%% A few notes
% The bvpagerank code carefully tries to avoid operations that create
% temporary vectors in Matlab.  Towards this end, we write strange 
% sequences of operations like

% x = x - y
% delta = norm(x,1)

%%
% instead of the more natural Matlab syntax

% delta = norm(x-y,1)

%%
% because the latter syntax creates another vector for the
% difference x-y that is just temporary.