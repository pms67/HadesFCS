function [roll, pitch, yaw] = ekfQuatAtt(gyr, acc, mag, airspeed, dt)

    persistent initStateCov initGBiasCov initABiasCov
    persistent processNoiseState processNoiseGBias processNoiseABias
    persistent measNoiseAcc
    persistent lpfGyrCoeff lpfAccCoeff lpfVaCoeff
    persistent P Q R
    persistent xhat
    persistent g
    persistent DT_STEPS
    
    persistent p q r ax ay az V

    persistent flag
    
    if isempty(flag)
        flag = 1;

        initStateCov = 1e-3;
        initGBiasCov = 1e-6;
        initABiasCov = 1e-6;

        processNoiseState = 0.1;
        processNoiseGBias = 1e-6;
        processNoiseABias = 1e-6;

        measNoiseAcc = 1.0;

        lpfGyrCoeff = 0.8;
        lpfAccCoeff = 0.9;
        lpfVaCoeff = 0.75;


        P = diag([ones(4,1)*initStateCov; ones(3,1)*initGBiasCov; ones(3,1)*initABiasCov]);
        Q = diag([ones(4,1)*processNoiseState; ones(3,1)*processNoiseGBias; ones(3,1)*processNoiseABias]);
        R = diag([ones(3,1)*measNoiseAcc]);
        xhat = zeros(10, 1);
        xhat(1)= 1.0;
                
        p = 0; q = 0; r = 0;
        ax = 0; ay = 0; az = 0;
        V = 0;
        
        DT_STEPS = 10;
        g = 9.81;
    end

    % Sensor readings (+ low-pass filter)
    p = lpfGyrCoeff * p + (1 - lpfGyrCoeff) * gyr(1);
    q = lpfGyrCoeff * q + (1 - lpfGyrCoeff) * gyr(2);
    r = lpfGyrCoeff * r + (1 - lpfGyrCoeff) * gyr(3);

    ax = lpfAccCoeff * ax + (1 - lpfAccCoeff) * acc(1);
    ay = lpfAccCoeff * ay + (1 - lpfAccCoeff) * acc(2);
    az = lpfAccCoeff * az + (1 - lpfAccCoeff) * acc(3);

    V = lpfVaCoeff * V + (1 - lpfVaCoeff) * airspeed;

    mx = mag(1);
    my = mag(2);
    mz = mag(3);  

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
          V*(bq - q) - g*(e0^2 - e1^2 - e2^2 + e3^2) - bz];

    % Jacobian of h
    C = [ 2*e2*g, -2*e3*g,  2*e0*g, -2*e1*g, 0, 0,  0, -1, 0, 0; ...
         -2*e1*g, -2*e0*g, -2*e3*g, -2*e2*g, 0, 0, -V, 0, -1, 0; ...
         -2*e0*g,  2*e1*g,  2*e2*g, -2*e3*g, 0, V,  0, 0, 0, -1];

    K = P * C' / (C * P * C' + R);
    P = (eye(10) - K * C) * P;

    xhat = xhat + K * ([ax - bx; ay - by; az - bz] - h);

    % Again, extract updated states from vector
    e0 = xhat(1); e1 = xhat(2); e2 = xhat(3); e3 = xhat(4);

    roll = atan2(2*(e0*e1 + e2*e3), e0*e0+e3*e3-e1*e1-e2*e2) * 180 / pi;
    pitch = asin(2*(e0*e2-e1*e3)) * 180 / pi;
    yaw = atan2(2*(e0*e3+e1*e2),e0*e0+e1*e1-e2*e2-e3*e3) * 180 / pi;
