initStateCov = 1e-3;
initGBiasCov = 1e-6;
initABiasCov = 1e-6;

processNoiseState = 0.1;
processNoiseGBias = 1e-6;
processNoiseABias = 1e-6;

measNoiseAcc = 0.01;
measNoiseMag = 0.1;

lpfGyrCoeff = 0.8;
lpfAccCoeff = 0.9;
lpfMagCoeff = 0.3;
lpfVaCoeff = 0.75;

P = diag([ones(4,1)*initStateCov; ones(3,1)*initGBiasCov; ones(3,1)*initABiasCov]);
Q = diag([ones(4,1)*processNoiseState; ones(3,1)*processNoiseGBias; ones(3,1)*processNoiseABias]);
R = diag([ones(3,1)*measNoiseAcc; measNoiseMag]);
xhat = zeros(10, 1);

dt = 0.1;
DT_STEPS = 1;
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
biasx = zeros(1, N);
biasy = zeros(1, N);
biasz = zeros(1, N);
              

p = 0; q = 0; r = 0;
ax = 0; ay = 0; az = 0;
mx = 0; my = 0; mz = 0;
V = 0;

xhat(1) = 1.0;

for n  = 1 : N
    
    % Sensor readings (+ low-pass filter)
    p = lpfGyrCoeff * p + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 1);
    q = lpfGyrCoeff * q + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 2);
    r = lpfGyrCoeff * r + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 3);

    ax = lpfAccCoeff * ax + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 1);
    ay = lpfAccCoeff * ay + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 2);
    az = lpfAccCoeff * az + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 3);

    V = lpfVaCoeff * V + (1 - lpfVaCoeff) * uavSensors.pitot_airspeed_mps(n);

    mx = lpfMagCoeff * mx + (1 - lpfMagCoeff) * uavSensors.mag3D_unitVector_in_body(n, 1);
    my = lpfMagCoeff * my + (1 - lpfMagCoeff) * uavSensors.mag3D_unitVector_in_body(n, 2);
    mz = lpfMagCoeff * mz + (1 - lpfMagCoeff) * uavSensors.mag3D_unitVector_in_body(n, 3);  

    dtfrac = dt / DT_STEPS;
    
    for k=1:DT_STEPS
    
        % Extract states from vector
        e0 = xhat(1); e1 = xhat(2); e2 = xhat(3); e3 = xhat(4);
        bp = xhat(5); bq = xhat(6); br = xhat(7);

        % State transition function
        f = [e1*(bp/2 - p/2) + e2*(bq/2 - q/2) + e3*(br/2 - r/2); ...
             e3*(bq/2 - q/2) - e0*(bp/2 - p/2) - e2*(br/2 - r/2); ...
             e1*(br/2 - r/2) - e0*(bq/2 - q/2) - e3*(bp/2 - p/2); ...
             e2*(bp/2 - p/2) - e1*(bq/2 - q/2) - e0*(br/2 - r/2); ...
             0; 0; 0; 0; 0; 0];

        % Jacobian of f
        A = [0, bp/2 - p/2, bq/2 - q/2, br/2 - r/2,  e1/2,  e2/2,  e3/2, 0, 0, 0; ...
             p/2 - bp/2,          0, r/2 - br/2, bq/2 - q/2, -e0/2,  e3/2, -e2/2, 0, 0, 0; ...
             q/2 - bq/2, br/2 - r/2,          0, p/2 - bp/2, -e3/2, -e0/2,  e1/2, 0, 0, 0; ...
             r/2 - br/2, q/2 - bq/2, bp/2 - p/2,          0,  e2/2, -e1/2, -e0/2, 0, 0, 0; ...
                      0,          0,          0,          0,     0,     0,     0, 0, 0, 0; ...
                      0,          0,          0,          0,     0,     0,     0, 0, 0, 0; ...
                      0,          0,          0,          0,     0,     0,     0, 0, 0, 0; ...
                      0,          0,          0,          0,     0,     0,     0, 0, 0, 0; ...
                      0,          0,          0,          0,     0,     0,     0, 0, 0, 0; ...
                      0,          0,          0,          0,     0,     0,     0, 0, 0, 0];


        xhat = xhat + dtfrac * f;
        P = P + dtfrac * (A * P + P * A' + Q);
        
    end
    
    % Again, extract updated states from vector
    e0 = xhat(1); e1 = xhat(2); e2 = xhat(3); e3 = xhat(4);
    bp = xhat(5); bq = xhat(6); br = xhat(7);
    bx = xhat(8); by = xhat(9); bz = xhat(10);
           
    % Output function
    h = [ g*(2*e0*e2 - 2*e1*e3) - bx; ...
         -V*(br - r) - g*(2*e0*e1 + 2*e2*e3) - by; ...
          V*(bq - q) - g*(e0^2 - e1^2 - e2^2 + e3^2) - bz; ...
          atan2(2*(e0*e3 + e1*e2), 1 - 2 * (e2*e2 + e3*e3))];
        
    % Jacobian of h
    fact = 1 / ((2*e2^2 + 2*e3^2 - 1)^2 + (2*e0*e3 + 2*e1*e2)^2);
    
    C = [2*e2*g, -2*e3*g,  2*e0*g, -2*e1*g, 0, 0,  0, -1,  0,  0; ...
        -2*e1*g, -2*e0*g, -2*e3*g, -2*e2*g, 0, 0, -V,  0, -1,  0; ...
        -2*e0*g,  2*e1*g,  2*e2*g, -2*e3*g, 0, V,  0,  0,  0, -1; ...
        -(2*e3*(2*e2^2 + 2*e3^2 - 1)) * fact, -(2*e2*(2*e2^2 + 2*e3^2 - 1)) * fact, -(((2*e1)/(2*e2^2 + 2*e3^2 - 1) - (4*e2*(2*e0*e3 + 2*e1*e2))/(2*e2^2 + 2*e3^2 - 1)^2)*(2*e2^2 + 2*e3^2 - 1)^2) * fact, -(((2*e0)/(2*e2^2 + 2*e3^2 - 1) - (4*e3*(2*e0*e3 + 2*e1*e2))/(2*e2^2 + 2*e3^2 - 1)^2)*(2*e2^2 + 2*e3^2 - 1)^2) * fact, 0, 0,  0,  0,  0,  0];
      
    
    K = P * C' / (C * P * C' + R);
    P = (eye(10) - K * C) * P;
    
    % Calculate yaw angle from magnetometer readings
    mphi = atan2(2*(e0*e1 + e2*e3), e0*e0+e3*e3-e1*e1-e2*e2);
    mtheta = asin(2*(e0*e2-e1*e3));
    
    yawMag = atan2(-my * cos(mphi) + mz * sin(mphi), mx * cos(mtheta) + (my * sin(mphi) + mz * cos(mphi)) * sin(mtheta));
    
    xhat = xhat + K * ([ax - bx; ay - by; az - bz; yawMag] - h);
     
    % Again, extract updated states from vector
    e0 = xhat(1); e1 = xhat(2); e2 = xhat(3); e3 = xhat(4);
    
    rollhat(n) = atan2(2*(e0*e1 + e2*e3), e0*e0+e3*e3-e1*e1-e2*e2) * 180 / pi;
    pitchhat(n) = asin(2*(e0*e2-e1*e3)) * 180 / pi;
    yawhat(n) = atan2(2*(e0*e3+e1*e2),e0*e0+e1*e1-e2*e2-e3*e3) * 180 / pi;
    if (yawhat(n) < 0)
        yawhat(n) = yawhat(n) + 360.0;
    end
    biasp(n) = xhat(5) * 180 / pi;
    biasq(n) = xhat(6) * 180 / pi;
    biasr(n) = xhat(7) * 180 / pi;
    biasx(n) = xhat(8);
    biasy(n) = xhat(9);
    biasz(n) = xhat(10); 
              
end

meanerror = mean(abs(roll'-rollhat));

subplot(2,3,1)
plot(t, roll, t, rollhat, 'r-.')
line([0,t(end)], [meanerror, meanerror], 'LineStyle', '--')
legend('True', 'Estimate', 'Mean Error')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Roll')

meanerror = mean(abs(pitch'-pitchhat));

subplot(2,3,2)
plot(t, pitch, t, pitchhat, 'r-.')
line([0,t(end)], [meanerror, meanerror], 'LineStyle', '--')
legend('True', 'Estimate', 'Mean Error')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Pitch')

meanerror = mean(abs(yaw'-yawhat));

subplot(2,3,3)
plot(t, yaw, t, yawhat, 'r-.')
line([0,t(end)], [meanerror, meanerror], 'LineStyle', '--')
legend('True', 'Estimate', 'Mean Error')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Yaw')

subplot(2,3,4)
plot(t, biasp, t, biasq, t, biasr)
legend('p', 'q', 'r')
xlabel('Time (s)')
ylabel('Bias (deg/s)')
title('Gyroscope Bias Estimates')

subplot(2,3,5)
plot(t, biasx, t, biasy, t, biasz)
legend('x', 'y', 'z')
xlabel('Time (s)')
ylabel('Bias (m/s^2)')
title('Accelerometer Bias Estimates')