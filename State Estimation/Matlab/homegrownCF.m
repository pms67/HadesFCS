dt = 0.1;
g = 9.81;
alpha = 0.01;

N = length(uavTruth.time_s);

t = uavTruth.time_s';
roll = uavTruth.roll_deg;
rollhat = zeros(1, N);
pitch = uavTruth.pitch_deg;
pitchhat = zeros(1, N);

p = 0; q = 0; r = 0;
ax = 0; ay = 0; az = 0;

lpfGyrCoeff = 0.4;
lpfAccCoeff = 0.9;

figure
hold on
for n=2:N

    p = lpfGyrCoeff * p + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 1);
    q = lpfGyrCoeff * q + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 2);
    r = lpfGyrCoeff * r + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 3);
    
    ax = lpfAccCoeff * ax + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 1);
    ay = lpfAccCoeff * ay + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 2);
    az = lpfAccCoeff * az + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 3);
       
    mx = uavSensors.mag3D_unitVector_in_body(n, 1);
    my = uavSensors.mag3D_unitVector_in_body(n, 1);
    mz = uavSensors.mag3D_unitVector_in_body(n, 1);   
    
    phidot   = p + q * sin(rollhat(n-1)) * tan(pitchhat(n-1)) + r * cos(rollhat(n-1)) * tan(pitchhat(n-1));
    thetadot = q * cos(rollhat(n-1)) - r * sin(rollhat(n-1));
    
    rollacc  = atan2(ay, sqrt(ax * ax + az * az));
    pitchacc = asin(ax / g);
    
    rollhat(n)  = alpha * rollacc  + (1 - alpha) * (rollhat(n-1)  + dt * phidot);
    pitchhat(n) = alpha * pitchacc + (1 - alpha) * (pitchhat(n-1) + dt * thetadot);
    
end

    subplot(2,1,1)
    plot(t(1:n), roll(1:n), t(1:n), rollhat(1:n) * 180 / pi, 'r-.')
    if (n == 1)
       legend('True', 'Estimate')
       xlabel('Time (s)')
        ylabel('Angle (deg)')
        title('Roll')
    end

    subplot(2,1,2)
    plot(t(1:n), pitch(1:n), t(1:n), pitchhat(1:n) * 180 / pi, 'r-.')
    if (n == 1)
       legend('True', 'Estimate')
        xlabel('Time (s)')
       ylabel('Angle (deg)')
       title('Pitch')
   end



