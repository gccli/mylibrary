% demo for chapter 03
clear; close all;

function plotBand(x, y, h, color)
  if nargin < 4
    color = [0,228,225]/255; %pink
  end
  x = x(:);
  y = y(:);
  h = h(:);
  fill([x;flipud(x)],[y+h;flipud(y-h)],color);
endfunction

function [y, sigma, p] = linInfer(X, model, t)
  % Compute linear model reponse y = w'*x+b and likelihood
  % X: d x n data
  % t: 1 x n response
  w = model.w;
  b = model.b;
  y = w'*X+b;
  if nargout > 1
    beta = model.beta;
    if isfield(model,'V')   % V*V'=inv(S) 3.54
      X = model.V'*bsxfun(@minus,X,model.xbar);
      sigma = sqrt(1/beta+dot(X,X,1));   % 3.59
    else
      sigma = sqrt(1/beta);
    end
    if nargin == 3 && nargout == 3
      p = exp(-0.5*(((t-y)./sigma).^2+log(2*pi))-log(sigma));
    end
  end
endfunction

function model = regress(X, t, lambda)
  % Fit linear regression model t=w'x+b
  % X: d x n data
  % t: 1 x n response
  if nargin < 3
    lambda = 0;
  end
  d = size(X,1);
  xbar = mean(X,2); % mean (x, dim)
  tbar = mean(t,2);

  X = bsxfun(@minus,X,xbar);
  t = bsxfun(@minus,t,tbar);

  S = X*X';
  dg = sub2ind([d,d],1:d,1:d);
  S(dg) = S(dg)+lambda;
  % w = S\(X*t');
  % Compute the Cholesky factor, R, of the symmetric positive definite matrix A, where $R^TR = A$
  R = chol(S);
  w = R\(R'\(X*t'));  % 3.15 & 3.28
  b = tbar-dot(w,xbar);  % 3.19
  model.w = w;
  model.b = b;
endfunction

n = 100;
beta = 4e-2;

# training set X,t
X = rand(1,n);
w = randn;
b = randn;
t = w'*X+b+beta*randn(1,n);

x = linspace(min(X)-1,max(X)+1,n);   % test data for predict


%1
model = regress(X, t);
y = linInfer(x, model);
figure;
hold on;
plot(X,t,'o');
plot(x,y,'r-');
title ("regress linear function");
hold off

%2
[model,llh] = regressEbEm(X,t);
[y, sigma] = linInfer(x,model,t);
figure;
hold on;
#plotBand(x,y,2*sigma);
plot(X,t,'o');
plot(x,y,'r-');
title ("Fit empirical Bayesian linear model with EM");
hold off
figure
plot(llh);

%3
[model,llh] = regressEbFp(X,t);
[y, sigma] = linInfer(x,model,t);
figure;
hold on;
%plotBand(x,y,2*sigma);
plot(X,t,'o');
plot(x,y,'r-');
title ("Fit empirical Bayesian linear model with Mackay fixed point method");
hold off
figure
plot(llh);
