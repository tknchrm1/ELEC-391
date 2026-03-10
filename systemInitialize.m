%========================
% System Initialization
% Author: Jeremey Fernandez & Tsz Kin Charm 
% Date: Febraury 10th, 2026
% Description: Initilazes the Plant Transfer Function based on the physical
% characteristics of the system
%========================

clc;

%% Constants

MU = 4 * pi * 10^-7; % Permeability of Free Space
MUPLA = 0;
s = tf('s');

%% User Inputs
% Circuit
% Resistance (Ra) - Measured with ohmmeter function of a multimeter
% Inductance (La) - Calculated using AC Current at a known frequency and 
% finding the impedance

Ra = 8.5; % Ohms
La = 1E-4; % Henry
Kt = 0.0329;
Ke = 0.0327;

% Inertia
% Implemented using a disk of some mass M
% I = M * R^2  / 2

m = 5.0; % Mass - kg 
r = 0.05; % Radius - m

%Jeq = (1/2) * m * r^2; % Inertia - kg * m^2
Jeq = 1E-5;

% Damping *********
% Implemented using EDDY CURRENT brakes with N52 magnets

% Magnet Circuit Parameters
damper.n = 8; % Number of magnets
damper.emf = damper.n*0.5; % T
damper.l = 29 * 1E-3; % m
damper.w = 9.5 * 1E-3; % m
damper.A = damper.l * damper.w; % m^2

%damper.lg = 6 * 1E-3;
damper.lg = 6 * 1E-3;

% Magnetic Circuit Calculations
damper.reluctanceAir = damper.lg/ (MU * damper.A);
damper.phi = damper.emf/damper.reluctanceAir;

Beq = damper.phi/damper.w;

% Stiffness (K)
% Implemented using elastic bands. Spring constant was calculated by 
% hanging a known mass and finding the change in length
numberBands = 1; % Number of Elastic Bands - pure
%Keq = 19.856*numberBands; % Spring Constant of Elastic Bands - T/m
Keq = 0;

% Declaration of System Parameters
fprintf("This system is operating under the following constraints:\n");
fprintf("Equivalent Inertia = %1.6f Kg * m^2\n", Jeq);
fprintf("Equivalent Damping = %1.6f Ns/m\n", Beq);
fprintf("Equivalent Stiffness = %1.2f N/m\n", Keq);

%% Plant Modelling
% Description:
% To attain our plant model, we would like to find the impedances of each
% portion of our motor and belt system. This includes:
% Ye - Electrical Impedance;
% Kt - Current-Torque Constant
% Ym - Mechanical Impedance

% Mechanical Impedance
% Using the known state space of a first order system, we can easily derive
% our mechanical impedance
% https://en.wikipedia.org/wiki/State-space_representation

% Known State Matrices for First Order System
A = [ 0 1 ; -Keq/Jeq -Beq/Jeq];
B = [ 0 ; 1/Jeq];
C = [1 0];
D = 0;

% Convert State Matrices to SISO System SS
sys = ss(A,B,C,D,'Statename', ...
    {'Angle (rad)', 'Angular Velocity (rad/s)'}, ...
    'InputName','Torque (Nm)', ...
    'OutputName','Position (m)');

% Convert SS to tf with ratio of velocity to torque
Ym = minreal(tf(sys)*s); % V/tau

% Reformat to the form 1/(s * J + B)
[Ymnum,Ymden] = tfdata(Ym);
Ym = tf(1,Ymden{1}*(1/Ymnum{1}(2))); 

%% Electrical Impedance
Ye = 1/(Ra + s*La); % current/voltage

%% Current Torque Constant
Kt = 0.0327; % torque/current - Nm/A

%% Plant Models
Gmotor = Ym*Ye*Kt; % velocity/voltage

%Adjustment for Counter EMF
Gmotor = feedback(Gmotor,Ke);

n = 1; % Gear ratio - Diameter of pulley to belt if it was a gear

Gplant = Gmotor * (1/n) * (1/s);

%{
fprintf("The plant transfer function G (Position/Voltage)is:");
display(Gplant);
%}

fprintf("\n");
fprintf("Model is now ready for control\n");













