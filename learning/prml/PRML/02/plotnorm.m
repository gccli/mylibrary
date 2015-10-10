clear
close all

x=[-1:0.01:1];
plot(x, normpdf(x, 0, 0.1));
title ('Gaussian Probability Distribution');
xlabel ('{\bf x}');
ylabel ('{\it N}(x|\mu,\sigma^2)');
text (0, 0.5, '\mu');
legend ('{\sl N}(x|\mu,\sigma^2)');
