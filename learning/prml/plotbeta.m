clear
close all
x=[0:0.01:1];

subplot (2, 2, 1)
plot(x, betapdf(x, 0.1,0.1));
subplot (2, 2, 2)
plot(x, betapdf(x, 1,1));
subplot (2, 2, 3)
plot(x, betapdf(x, 50,50));
subplot (2, 2, 4)
plot(x, betapdf(x, 8,4));
