clear
close all

function y = gampdf(x,a,b)
  y = 1/gamma(a) * b^a .* x.^(a-1) .* exp(-b.*x);
endfunction

function y = norm_gampdf(mu,lambda)
  mu0 = 0;
  beta = 2;
  a = 5;
  b = 6;
  y = normpdf(mu, mu0, (beta.*lambda).^(-1)).*gampdf(lambda,a,b);
endfunction

lambda=[0:0.01:2];
title ('Gamma Distribution');

%
subplot (3, 1, 1)
plot(lambda, gampdf(lambda, 0.1, 0.1));
xlabel ('{\bf \lambda}');
ylabel ('{\it Gam}(\lambda|a,b)');

%
subplot (3, 1, 2)
plot(lambda, gampdf(lambda, 1, 1));
xlabel ('{\bf \lambda}');
ylabel ('{\it Gam}(\lambda|1,1)');

%
subplot (3, 1, 3)
plot(lambda, gampdf(lambda, 4, 6));
xlabel ('{\bf \lambda}');
ylabel ('{\it Gam}(\lambda|4,6)');

%plot(lambda, norm_gampdf(0,lambda));
