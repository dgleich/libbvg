function bvg = bvgraph(filename,optionsu)
% BVGRAPH/BVGRAPH Construct a Matlab object around a Boldi-Vigna graph file.
%
% The bvgraph class represents a Boldi-Vigna compressed graph as a native
% Matlab object.  The class wraps a libbvg C object with a series of
% functions that make the graph look like a Matlab sparse matrix.  
% For the object G, the operations G*x, and G'*x, x'*G are all
% valid, and consequently, G can be used in many iterative methods without
% adaptation. 
%
% G = mwebgraph(filename) loads the graph represented by filename
% G = mwebgraph(filename,options) specifies non-default options for loading
%
% options.trans: internally transpose the graph [{0} | 1]
% options.load_type : determine how to load the graph
%   ['stream' | 'offline' | 'inmemory' | {'online'} | 'random']
%   The options 'stream' and 'offline' are synonymous, as are 'inmemory'
%   and 'online'.  The memory required satisfies the hierarchy
%   'stream' << 'inmemory' << 'random'
%
% If load_type is 'random', then the graph and offsets file are both
% loaded into memory.  This option requires the offsets file.  The
% difference between 'inmemory'/'online' and 'offline'/'stream' is that 
% 'online' reads the graph file into memory but not the offsets file. 
% The 'stream' option does not read the file into memory.
%
% The filename is the 'base' filename for the graph and 'filename.graph' and
% 'filename.properties' must exist.
%
%Example:
%  G = bvgraph('data/wb-cs.stanford') % compute a PageRank vector using bicgstab
%  n = size(G,1); alpha = 0.85; v = ones(n,1)./n;
%  id = G*ones(n,1); id(id ~= 0) = 1./id(id ~= 0);
%  x = bicgstab(@(x) x - alpha*(G'*(id.*x)), v, 1e-8, 500); x = x./norm(x,1);
%  y = diag(G);

%
% David Gleich
% 21 May 2007
% Copyright, Stanford University, 2007
%

% handle the case of making a copy...
if isa(filename,'bvgraph')
    bvg=filename;
    return
end

options = struct('load_type', 'online','trans',0);
if exist('optionsu','var')
    options = merge_structs(optionsu,options);
end

switch options.load_type
    case 'stream'
        offset_step = -1;
    case 'offline'
        offset_step = -1;
    case 'inmemory'
        offset_step = 0;
    case 'online'
        offset_step = 0;
    case 'random'
        offset_step = 1;
    otherwise
        error('bvgraph:invalidParameter','unknown load type: %s', options.load_type);
end

transp = 0;
if options.trans
    transp = 1;
end

if ~exist([filename '.graph'], 'file')
    error('bvgraph:fileNotFound', ...
        'The file %s.graph does not seem to exist!', filename);
end

if ~exist([filename '.properties'], 'file')
    error('bvgraph:fileNotFound', ...
        'The properties file %s.properties does not seem to exist!', filename);
end

if offset_step > 0 && ~exist([filename '.offsets'], 'file')
    error('bvgraph:fileNotFound', ...
        'The offsets file %s.offsets does not seem to exist!', filename);
end

[n,nz,smem,gmem,offsetmem] = bvgfun('load',filename,offset_step);

bvg = struct('n',n,'nz',nz,'smem',smem,'gmem',gmem,'offsetmem',offsetmem,...
    'offset_step', offset_step, 'filename', filename, ...
    'class', '', 'transp', transp);

bvg = class(bvg, 'bvgraph');
