function [ roll, pitch, yaw ] = ekfRollPitchYaw( gyr, acc, psiMag, V, T )

    persistent flag
    persistent g
    persistent x
    persistent P
    persistent Q
    persistent R
    persistent noiseGyr
    persistent noiseAcc
    persistent noiseMag
    persistent noiseBias
    
    if isempty(flag)
       
        flag = 1;
        
        g = 9.81;
        
        x = zeros(7, 1);
        x(1) = 1.0;
        
        P = 10.0 * eye(7);
        
        noiseGyr  = 0.003455;
        noiseAcc  = 0.015696;
        noiseMag  = 0.004242;
        
        noiseBias = 0.000100;
        
        Q = diag([3.0 * noiseGyr ^ 2, 3.0 * noiseGyr ^ 2, 3.0 * noiseGyr ^ 2, 3.0 * noiseGyr ^ 2, ...
                    noiseBias ^ 2, noiseBias ^ 2, noiseBias ^ 2]);
                
        R = diag([noiseAcc ^ 2, noiseAcc ^ 2, noiseAcc ^ 2, noiseMag ^ 2]);
        
    end
    
    p = gyr(1);
    q = gyr(2);
    r = gyr(3);
    
    % Extract states
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);    
    bp = x(5); bq = x(6); br = x(7);
    
    % State transition function f(x,u)
    x(1) = x(1) + 0.5 * T * (q1*(bp - p) + q2*(bq - q) + q3*(br - r));
    x(2) = x(2) + 0.5 * T * (q3*(bq - q) - q0*(bp - p) - q2*(br - r));
    x(3) = x(3) + 0.5 * T * (q1*(br - r) - q0*(bq - q) - q3*(bp - p));
    x(4) = x(4) + 0.5 * T * (q2*(bp - p) - q1*(bq - q) - q0*(br - r));
    
    % Normalise quaternion and re-extract states
    inorm = 1.0 / sqrt(x(1) * x(1) + x(2) * x(2) + x(3) * x(3) + x(4) * x(4));
    x(1) = x(1) * inorm;
    x(2) = x(2) * inorm;
    x(3) = x(3) * inorm;
    x(4) = x(4) * inorm;
    
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);
    
    % Jacobian of f(x,u)
    A = 0.5 * [0, bp - p, bq - q, br - r,  q1,  q2,  q3; ...
               p - bp,      0, r - br, bq - q, -q0,  q3, -q2; ...
               q - bq, br - r,      0, p - bp, -q3, -q0,  q1; ...
               r - br, q - bq, bp - p,      0,  q2, -q1, -q0; ...
               0,      0,      0,      0,   0,   0,   0; ...
               0,      0,      0,      0,   0,   0,   0; ...
               0,      0,      0,      0,   0,   0,   0];
           
    % Update error covariance matrix
    P = P + T * (A * P + P * A' + Q);
    
    % Output function h(x,u)
    h = [ g*(2*q0*q2 - 2*q1*q3); ...
         -V*(br - r) - g*(2*q0*q1 + 2*q2*q3); ...
          V*(bq - q) + g*(2*q1*q1 + 2*q2*q2 - 1); ...
          atan2(2*q0*q3 + 2*q1*q2, - 2*q2*q2 - 2*q3*q3 + 1)];
      
    % Jacobian of h(x,u)
    fact = 1.0 / ((2*q2^2 + 2*q3^2 - 1)^2 + (2*q0*q3 + 2*q1*q2)^2);
    C = [ 2*g*q2, -2*g*q3,  2*g*q0, -2*g*q1, 0, 0,  0; ...
         -2*g*q1, -2*g*q0, -2*g*q3, -2*g*q2, 0, 0, -V; ...
               0,  4*g*q1,  4*g*q2,       0, 0, V,  0; ...
        -(2*q3*(2*q2^2 + 2*q3^2 - 1)) * fact, -(2*q2*(2*q2^2 + 2*q3^2 - 1)) * fact, -(((2*q1)/(2*q2^2 + 2*q3^2 - 1) - (4*q2*(2*q0*q3 + 2*q1*q2))/(2*q2^2 + 2*q3^2 - 1)^2)*(2*q2^2 + 2*q3^2 - 1)^2)*fact, -(((2*q0)/(2*q2^2 + 2*q3^2 - 1) - (4*q3*(2*q0*q3 + 2*q1*q2))/(2*q2^2 + 2*q3^2 - 1)^2)*(2*q2^2 + 2*q3^2 - 1)^2)*fact, 0, 0,  0];
      
    % Kalman gain
    K = P * C' / (C * P * C' + R);
    
    % Update error covariance matrix
    P = (eye(7) - K * C) * P;
    
    % Update state estimate
    x = x + K * ([acc(1); acc(2); acc(3); psiMag] - h);
    
    % Normalise quaternion and re-extract states
    inorm = 1.0 / sqrt(x(1) * x(1) + x(2) * x(2) + x(3) * x(3) + x(4) * x(4));
    x(1) = x(1) * inorm;
    x(2) = x(2) * inorm;
    x(3) = x(3) * inorm;
    x(4) = x(4) * inorm;
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);
    
    % Convert to Euler angles
    roll  = atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (q1 * q1 + q2 * q2));
    pitch = asin(2 * (q0 * q2 - q3 * q1));
    yaw   = atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (q2 * q2 + q3 * q3));
end

