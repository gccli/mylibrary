clear;
close all;

x = [0:0.01:9.99];
y = horzcat(normrnd(1,0.4,1,400),normrnd(4,0.1,1,600));

%
subplot (3, 1, 1)
hist(y, 20);
title ('Histogram');

subplot (3, 1, 2)
[a,b] = hist(y, 50);
a = a/length(y);
bar(a, "facecolor", "g");

y = binornd(1000, 0.2, 1, 1000);
size(y)
subplot (3, 1, 3)
%plot(x,y);
hist(y, 25, "facecolor", "r");
