%% RCGS

Res   = 0.1;             % rad/s
TargPM = 60;             % rad

OS = 15/100;             % percent -> Based on furthest overshoot possible 
                         % without carriage falling off of rails for edge notes

Ts  = 0.333;             % seconds -> target BPM of 82, with eight notes
                         % Time per note / 2 * 0.9 for safety factor


Ess = 0;                 % pure

%% 
% PID Controller

% Gains
%{
Kp = 8.0;
Ki = 5.0;
Kd = 0.5;
%}

Kp = 0.30551;
Ki = 0.35172;
Kd = 0.015953;

% Sum all controllers
Gcontroller = Kp + Ki/s + Kd*s;

fprintf("Controller Initialized\n");

%% Sensor Model
DC = 0.4; %percntage of clock cycle

wd = 13;

dt = 0.0025; % s
CF = 400; % Hz

Nf = (CF/(10 * wd)) - DC - 0.5; % Maximum Filter Delay based on MDP

Ds = 0; % Sensor Dynamics
Ks = 1; % Sesnor Gain

%% ================================
% Question 8 - Delay
% ================================

H = 1/Ks * tf(CF/Nf,[1 CF/Nf]); % Gain  of Hc & Hs has to be 1

