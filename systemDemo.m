%% Run closed loop system

G = Gcontroller * Gplant; % err -> actual position

% Analyze the closed loop system response
T = feedback(G, H); % desired postion -> actual position

%{
yd = ones(1,10001) .* 0.5;
t = 0:0.001:10;

lsimplot(T,yd,t);
%}


