% Done
% demo for chapter 03
clear; close all;
n = 100;
beta = 1e-1;

% Return a matrix with random elements uniformly distributed on the interval (0, 1)
% http://www.gnu.org/software/octave/doc/interpreter/Special-Utility-Matrices.html#index-rand
% http://www.gnu.org/software/octave/doc/interpreter/Special-Utility-Matrices.html#XREFrandn
X = rand(1,n);
w = randn;
b = randn;
t = w'*X+b+beta*randn(1,n);

% Return a row vector with n linearly spaced elements between base and limit.
x = linspace(min(X)-1,max(X)+1,n);   % test data
%
model = regress(X, t);
y = linInfer(x, model);
figure;
hold on;
plot(X,t,'*');
plot(x,y,'r-');
hold off
%{
[model,llh] = regressEbEm(X,t);
[y, sigma] = linInfer(x,model,t);
figure;
hold on;
plotBand(x,y,2*sigma);
plot(X,t,'o');
plot(x,y,'r-');
hold off
figure
plot(llh);
%%
[model,llh] = regressEbFp(X,t);
[y, sigma] = linInfer(x,model,t);
figure;
hold on;
plotBand(x,y,2*sigma);
plot(X,t,'o');
plot(x,y,'r-');
hold off
figure
plot(llh);
%}
