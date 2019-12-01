initStateCov = 0.1;
initGBiasCov = 0.001;

processNoiseState = 0.001;
processNoiseGBias = 0.00001;

measNoiseAcc = 0.1;
measNoisePsi = 0.001;

lpfGyrCoeff = 0.1;
lpfAccCoeff = 0.5;
lpfVaCoeff = 0.75;


P = diag([ones(3,1)*initStateCov; ones(3,1)*initGBiasCov]);
Q = diag([ones(3,1)*processNoiseState; ones(3,1)*processNoiseGBias]);
R = diag([ones(3,1)*measNoiseAcc; measNoisePsi]);

% Initial state estimates
xhat = zeros(6, 1);

mx = uavSensors.mag3D_unitVector_in_body(1, 1);
my = uavSensors.mag3D_unitVector_in_body(1, 2);
mz = uavSensors.mag3D_unitVector_in_body(1, 3);   
xhat(3) = atan2(-my, mx) + 9.4 * pi / 180;

xhat(4) = mean(uavSensors.gyro_wb_rps(:, 1));
xhat(5) = mean(uavSensors.gyro_wb_rps(:, 2));
xhat(6) = mean(uavSensors.gyro_wb_rps(:, 3));

dt = 0.1;
g = 9.81;

N = length(uavTruth.time_s);

t = uavTruth.time_s';
roll = uavTruth.roll_deg;
rollhat = zeros(1, N);
pitch = uavTruth.pitch_deg;
pitchhat = zeros(1, N);
yaw = uavTruth.yaw_deg;
yawhat = zeros(1, N);

biasp = zeros(1, N);
biasq = zeros(1, N);
biasr = zeros(1, N);

p  = 0; q  = 0; r  = 0;
ax = 0; ay = 0; az = 0;
Va = 0;

figure
hold on
for n=1:N

    p = lpfGyrCoeff * p + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 1);
    q = lpfGyrCoeff * q + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 2);
    r = lpfGyrCoeff * r + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 3);
    
    ax = lpfAccCoeff * ax + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 1);
    ay = lpfAccCoeff * ay + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 2);
    az = lpfAccCoeff * az + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 3);
    
    Va = lpfVaCoeff * Va + (1 - lpfVaCoeff) * uavSensors.pitot_airspeed_mps(n);
   
    mx = uavSensors.mag3D_unitVector_in_body(n, 1);
    my = uavSensors.mag3D_unitVector_in_body(n, 2);
    mz = uavSensors.mag3D_unitVector_in_body(n, 3);   
        
    phi = xhat(1);
    theta = xhat(2);
    bp = xhat(4);
    bq =  xhat(5);
    br =  xhat(6);
       
    psimeas = atan2(-my * cos(phi) + mz * sin(phi), mx * cos(theta) + (my * sin(phi) + mz * cos(phi)) * sin(theta)) + 9.4 * pi / 180;
    
        
    % State transition function
    f =  [p - bp - cos(phi)*tan(theta)*(br - r) - sin(phi)*tan(theta)*(bq - q); ...
          sin(phi)*(br - r) - cos(phi)*(bq - q); ...
        -(sin(phi)*(bq - q))/cos(theta) - (cos(phi)*(br - r))/cos(theta); 0; 0; 0];                        

    xhat = xhat + dt * f;
    
    % Re-extract states
    phi = xhat(1);
    theta = xhat(2);
    psi = xhat(3);
    bp = xhat(4);
    bq =  xhat(5);
    br =  xhat(6);
    
    % Jacobian of f    
    A = [ sin(phi)*tan(theta)*(br - r) - cos(phi)*tan(theta)*(bq - q), -cos(phi)*(br - r)*(tan(theta)^2 + 1) - sin(phi)*(bq - q)*(tan(theta)^2 + 1), 0, -1, -sin(phi)*tan(theta), -cos(phi)*tan(theta); ...
          cos(phi)*(br - r) + sin(phi)*(bq - q), 0, 0, 0, -cos(phi), sin(phi); ...
         (sin(phi)*(br - r))/cos(theta) - (cos(phi)*(bq - q))/cos(theta), -(cos(phi)*sin(theta)*(br - r))/cos(theta)^2 - (sin(phi)*sin(theta)*(bq - q))/cos(theta)^2, 0,  0, -sin(phi)/cos(theta), -cos(phi)/cos(theta); ...
         0, 0, 0, 0, 0, 0; ...
         0, 0, 0, 0, 0, 0; ...
         0, 0, 0, 0, 0, 0];                  
    
    
    P = P + dt* (A * P + P * A' + Q);
                                  
    % Jacobian of h   
    C = [0,                                   g*cos(theta) - Va*cos(theta)*(bq - q), 0,             0, -Va*sin(theta),              0; ...
	 -g*cos(phi)*cos(theta), Va*cos(theta)*(bp - p) + Va*sin(theta)*(br - r) + g*sin(phi)*sin(theta), 0, Va*sin(theta),              0, -Va*cos(theta); ...
	 g*cos(theta)*sin(phi),                          g*cos(phi)*sin(theta) - Va*sin(theta)*(bq - q), 0,             0,  Va*cos(theta),              0; ...
	                      0,                                                                       0, 1,             0,              0,              0];
                  
                  
    K = P * C' / (C * P * C' + R);
    P = (eye(6) - K * C) * P;
        
    % Output function h
    h = [g*sin(theta) - Va*sin(theta)*(bq - q); ...
         Va*sin(theta)*(bp - p) - Va*cos(theta)*(br - r) - g*cos(theta)*sin(phi); ...
         Va*cos(theta)*(bq - q) - g*cos(phi)*cos(theta); ...
         psi];
                              
    xhat = xhat + K * ([ax; ay; az; psimeas] - h);

    rollhat(n) = xhat(1) * 180 / pi;
    pitchhat(n) = xhat(2) * 180 / pi;
    yawhat(n) = xhat(3) * 180 / pi;
    biasp(n) = xhat(4) * 180 / pi;
    biasq(n) = xhat(5) * 180 / pi;
    biasr(n) = xhat(6) * 180 / pi;
    
end

subplot(4,1,1)
plot(t, roll, t, rollhat, 'r-.')
legend('True', 'Estimate')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Roll')

subplot(4,1,2)
plot(t, pitch, t, pitchhat, 'r-.')
legend('True', 'Estimate')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Pitch')

subplot(4,1,3)
plot(t, yaw, t, yawhat, 'r-.')
legend('True', 'Estimate')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Yaw')

subplot(4,1,4)
plot(t, biasp, t, biasq, t, biasr)
legend('p', 'q', 'r')
xlabel('Time (s)')
ylabel('Bias (deg/s)')
title('Gyroscope Bias Estimates')
