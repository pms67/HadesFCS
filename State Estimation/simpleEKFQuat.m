function [rollDeg, pitchDeg] = simpleEKFQuat(gyr_rps, acc_mps2, Va_mps, T)

    persistent flag
    persistent p
    persistent q
    persistent r
    persistent ax
    persistent ay
    persistent az
    persistent Va
       
    persistent lpfGyr
    persistent lpfAcc
    persistent lpfVa
   
    persistent x
    persistent P
    persistent Q
    persistent R
    
    persistent g
    
    if isempty(flag)
       
        flag = 1;
        
        g = 9.81;
        
        % Low-pass filtered measurements
        p = 0.0; q = 0.0; r = 0.0;
        ax = 0.0; ay = 0.0; az = 0.0;
        Va = 0.0;
        
        % Low-pass filter coefficients
        lpfGyr = 0.7;
        lpfAcc = 0.9;
        lpfVa  = 0.7;
        
        % Initialise state estimate vector
        x = zeros(4, 1);
        x(1) = 1.0;
        
        % Initialise covariance matrix
        cAtt0 = 0.001;
        P = diag(([1 1 1 1] * cAtt0) .^ 2);
        
        % Process noise matrix
        nProcAtt  = 0.00005;
        Q = diag(([1 1 1 1] * nProcAtt) .^ 2);
        
        % Measurement noise matrix
        nMeasAcc = 0.05;
        R = diag(([1 1 1] * nMeasAcc) .^ 2);
        
    end
    
    % Get measurements and low-pass filter them
    p = lpfGyr * p + (1 - lpfGyr) * gyr_rps(1);
    q = lpfGyr * q + (1 - lpfGyr) * gyr_rps(2);
    r = lpfGyr * r + (1 - lpfGyr) * gyr_rps(3);
    
    ax = lpfAcc * ax + (1 - lpfAcc) * acc_mps2(1);
    ay = lpfAcc * ay + (1 - lpfAcc) * acc_mps2(2);
    az = lpfAcc * az + (1 - lpfAcc) * acc_mps2(3);
            
    Va = lpfVa * Va + (1 - lpfVa) * Va_mps;
    
    % Extract states
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);

    % State transition function, xdot = f(x, u)                         
    f = 0.5 * [-q1, -q2, -q3; ...
                q0, -q3,  q2; ...
                q3,  q0, -q1; ...
               -q2,  q1,  q0] * [p; q; r];

    % Update state estimate
    x = x + T  * f;
    
    % Normalise quaternion
    qNorm = sqrt(x(1) * x(1) + x(2) * x(2) + x(3) * x(3) + x(4) * x(4));
    x(1) = x(1) / qNorm;
    x(2) = x(2) / qNorm;
    x(3) = x(3) / qNorm;
    x(4) = x(4) / qNorm;
    
    % Re-extract states
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);    
    
    % Compute Jacobian of f, A(x, u)
    A = [0, -0.5 * p, -0.5 * q, -0.5 * r; ...
         0.5 * p, 0, 0.5 * r, -0.5 * q; ...
         0.5 * q, -0.5 * r, 0, 0.5 * p; ...
         0.5 * r, 0.5 * q, -0.5 * p, 0];
         
    % Update error covariance matrix
    P = P + T * (A * P + P * A' + Q);
    
    % Compute accelerometer output estimates
    % Note: assuming here that u = Va, v = 0, w = 0
    % Would be good to set u = Va * cos(theta), v = 0, w = Va * sin(theta)
    % But need expressions for cos(theta) as quaternions...
    
    z = [-2 * g * (q1 * q3 - q2 * q0); ...
         Va * r - 2 * g * (q2 * q3 + q1 * q0); ...
        -Va * q - g * (1 - 2 * (q1 * q1 + q2 * q2))];
                    
    % Jacobian of z, C(x, u)    
    C = [ 2 * g * q2, -2 * g * q3,  2 * g * q0, -2 * g * q1; ...
         -2 * g * q1, -2 * g * q0, -2 * g * q3, -2 * g * q2; ...
          0, 4 * g * q1, 4 * g * q2, 0];
 
    % Kalman gain
    K = P * C' / (C * P * C' + R);
    
    % Update error covariance matrix
    P = (eye(4) - K * C) * P;
        
    % Update state estimate using accelerometer measurements
    x = x + K * ([ax; ay; az] - z);
    
    % Normalise quaternion
    qNorm = sqrt(x(1) * x(1) + x(2) * x(2) + x(3) * x(3) + x(4) * x(4));
    x(1) = x(1) / qNorm;
    x(2) = x(2) / qNorm;
    x(3) = x(3) / qNorm;
    x(4) = x(4) / qNorm;
             
    % Re-extract states
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);
      
    % Store state estimates
    rollDeg   = atan2(2 * (q0 * q1 + q2 * q3), q0 * q0 + q3 * q3 - q1 * q1 - q2 * q2) * 180 / pi;
    pitchDeg  = asin (2 * (q0 * q2 - q1 * q3)) * 180 / pi;

end