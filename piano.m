% approximate solenoid model
L = 0.02;    % H  (estimate for a small coil)
R = 6;       % ohms (approx 6V/1A)
Kf = 15;     % N/A (approx linearized force constant)

% mechanical key parameters
m = 0.02;    % kg
b = 1.0;     % N*s/m
k = 1000;    % N/m  (≈1 N per mm)

s = tf('s');

G = Kf / ((L*s+R)*(m*s^2 + b*s + k));

% simulate step response to 5 V
t = 0:0.0005:0.2;  % 200 ms
v_step = 2;
[y,t] = step(v_step*G, t);

figure;
plot(t, y*1000);   % convert to mm
xlabel('Time (s)');
ylabel('Displacement (mm)');
title('Key displacement for 5V input to solenoid');
grid on;
