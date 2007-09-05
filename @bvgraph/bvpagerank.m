function [x,flag,hist] = bvpagerank(G,alpha,v,tol,maxiter,use_native)
% BVGRAPH/BVPAGERANK A simple implementation of PageRank for a bvgraph object.
%
% x = bvpagerank(G) computes an approximate PageRank vector x such that 
%   ||alpha*(P+d*v')*x + (1-alpha)*(e*v')*x - x||_1 < tol
% where alpha = 0.85, v = 1/n for all vertices, tol = 1e-8, and maxiter =
% 1000.
%
% x = bvpagerank(G,alpha,v,tol,maxiter) specifies all of the options of the
% PageRank system.  The type of PageRank system is a strongly personalized
% PageRank system.  To use the default value of an option, specify [].  
%
% By default, the pagerank function computes the PageRank operation for the
% matrix as stored _on disk_.  For example, bvpagerank(G) and bvpagerank(G')
% will return the same vector.  We use this behavior because computing
% bvpagerank(G') takes additional memory with respect to pagerank(G).  Since
% the goal of the bvgraph library is to support large-scale PageRank, we
% chose the memory-minimizing interpretation.  This implementation of
% PageRank will compute the vector for G', however, you must set a final
% parameter to 0 and call x = pagerank(G',[],[],[],[],0); 
%
% The algorithm used is the Power method for the PageRank algorithm.
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');
%   x = bvpagerank(G);
%   x = bvpagerank(G,0.9,[],1e-5,10);
%   y = bvpagerank(G',[],[],[],[],0); % turn off native optimizations

%
% David Gleich
% 21 May 2007
% Copyright, Stanford University, 2007
%

% 3 September 2007
% Added code to use the substochastic_mult features of the bvgraph class

if ~exist('alpha','var')
    alpha = 0.85;
elseif isempty(alpha)
    alpha = 0.85;
end

if ~exist('v','var') || isempty(v)
    implicit_v = 1;
else
    implicit_v = 0;
end

if ~exist('tol','var') || isempty(tol)
    tol = 1e-8;
end
if ~exist('maxiter','var') || isempty(maxiter)
    maxiter = 1000;
end

if ~exist('use_native','var') || isempty(use_native)
    use_native = 1;
end

if tol < 0
    error('bvpagerank:invalidParameter','tol must be positive');
end
if alpha < 0 || alpha >= 1
    error('bvpagerank:invalidParameter','alpha must be in [0,1)');
end
if implicit_v == 0
    if abs(sum(v) - 1) >= eps(1) || any(v < 0)
        error('bvpagerank:invalidParameter','v must satisfy sum(v) = 1 and all(v > 0)');
    end
end

n = size(G,1);

if use_native == 0
    id = G*ones(n,1);
    % determine how to apply inverse-outdegrees...
    nzcount = sum(id ~= 0);
    if (12*nzcount > 8*n)
        % in this case, just store the inverse degrees
        id(id ~= 0) = 1./id(id ~= 0);
        store_inds = 0;
    else
        store_inds = 1;
        nzidinds = int32(find(id ~= 0));
        id = 1./id(nzidinds);
    end
else
    % ensure that G is not tranposed
    if istrans(G)
        G = G';
    end
end

if implicit_v
    x = ones(n,1)./n;
else
    x = v;
end


hist = zeros(maxiter,1);
iter = 1;
delta = 1;
while (iter < maxiter)
    tic;
    if use_native == 0
        if store_inds
            y = alpha*x;
            y(nzidinds) = id.*x(nzidinds);
        else
            y = alpha*(id.*x);
        end
        y = G'*(y);
    else
        y = substochastic_mult(G',x);
        y = alpha*y;
    end
    ny = norm(y,1);
    if implicit_v
        y = y + (1-ny)*1./n;
    else
        y = y + (1-ny)*v;
    end
    
    x = x - y;
    delta = norm(x,1);
    x = y;
    hist(iter) = delta;
    if delta < tol
        break;
    end
    dt = toc;
    fprintf('iter %4i  %18g  %3.2f\n', iter, delta, dt);
    iter = iter+1;
end

hist = hist(1:iter);
if delta > tol
    warning('bvpagerank:didNotConverge','the algorithm did not converge in %i iterations', maxiter);
    flag = 1;
else
    flag = 0;
end

