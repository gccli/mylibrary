clear
close all

x = [0:0.01:1-0.01];
y = horzcat(normrnd(1,0.5,1,30),normrnd(3,0.1,1,70));

%
subplot (3, 1, 1)
plot(x, y);
title ('Kernel density model');
xlabel ('{\bf x}');
%ylabel ('{\it Gam}(\lambda|a,b)');


subplot (3, 1, 2)
%plot(lambda, gampdf(lambda, 1, 1));
%
subplot (3, 1, 3)
