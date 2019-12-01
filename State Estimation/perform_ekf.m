function [xhat_new P_new] = perform_ekf(compute_xdot_and_F,xhat_k,P_k,Q_t,compute_zhat_and_H,z_k,R_k,dt,varargin)
% PERFORM_EKF - Propagate and/or update a discrete-time Extended Kalman Filter.
%   PERFORM_EKF propagates the EKF state vector, xhat(k), and covariance, 
%   P(k), estimates from time k to time k+1.  If a measurement, z(k), and 
%   its associated sampled uncertainty matrix, R(k), are provided, the 
%   method updates the state vector and covariance based on information 
%   from the measurement.  Because non-linear dynamic models are often more 
%   intuitively represented in continuous time, PERFORM_EKF makes use of 
%   the continuous-time state derivatives, xdot(t), and process noise, Q(t).  
%   And because the state dynamics and measurement models are generally a 
%   function of the state (xhat), and possibly some additional inputs, 
%   PERFORM_EKF makes use of user-provided function handles for computing 
%   the non-linear and linearized dynamics and measurement models.
%
% Usage for propagating and updating states (xhat_k) and covariance (P_k) of n-state EKF:
% [xhat_new P_new] = perform_ekf( ...
%             @compute_xdot_and_F, ...% Handle to function which returns xdot (nx1 state derivatives) and F (nxn linearized dynamics model)
%             xhat_k, ...             % Current full state estimate (nx1)
%             P_k, ...                % Current state covariance matrix (nxn)
%             Q_t, ...                % Continuous time state process noise matrix (nxn)
%             @compute_zhat_and_H, ...% Handle to function which returns zhat (mx1 non-linear meas estimate) and H (mxm linearized meas model)
%             z_k, ...                % Current measurement vector (mx1)
%             R_k, ...                % Sampled measurement error covariance matrix (mxm)
%             dt, ...                 % Propagation time step interval, seconds: t(k+1) = t(k) + dt
%           [ u1, ... ]               % Optional input variables for compute_xdot_and_F() and/or compute_zhat_and_H()
%           [ u2, ... ]               % Optional input variables for compute_xdot_and_F() and/or compute_zhat_and_H()
%           [   .     ]
%           [ uN, ... ]               % Optional input variables for compute_xdot_and_F() and/or compute_zhat_and_H()
%             );
%
% More detailed information about PERFORM_EKF is available within the body
% of perform_ekf.m.
%
% Copyright © 2012 The Johns Hopkins University / Applied Physics Laboratory LLC.  All Rights Reserved.

% SUAS Code Version: 1.0, October 16, 2012  
% Author: Jeff Barton at JHU/APL, jeffrey.barton@jhuapl.edu
% Website: http://www.jhuapl.edu/ott/Technologies/Copyright/SuasCode.asp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% PERFORM_EKF implements a full-state discrete-time Extended Kalman Filter
% for a non-linear system and/or measurement model (See References 1 & 2).  
% System models are often derived as continuous-time models, where the 
% propagation of a state vector x(t) is defined by its time-derivative 
% xdot(t) (derivative of x(t) with respect to time).  At any time t, 
% xdot(t) must be modeled as a function of the system state, x(t), other 
% deterministic quantities (u(t)) that are not otherwise modeled in x(t), 
% and any non-deterministic influences, w(t).  Similarly, a non-linear 
% measurement vector, z(t), can also be modeled as a function of 
% x(t), u(t), and non-deterministic influences v(t).  Thus, a 
% continuous-time system to be estimated by a non-linear Kalman Filter 
% method can by modeled by:
%
%    Non-Linear Continuous-Time Model
%    --------------------------------
%    xdot(t) = f(x(t),u(t)) + w(t),           E(w(t)*w(t)')=Q(t)
%    z(t)    = h(x(t),u(t)) + v(t),           E(v(t)*v(t)')=R(t)
%
% In the Continuous-Time Model, f() and h() are functions representing the 
% state dynamics and measurement models, respectively.  Also, the state
% process noise, w(t), and the measurement noise, v(t), have statistical
% covariance matrices Q(t) and R(t), respectively. [E(w*w') is the expected
% value of the matrix w*w'. E(w*w') is also called the covariance of w.]
%
% But, the Extended Kalman Filter is based on a linearization of the
% non-linear system.  For the implemented full-state EKF, the system
% is linearized about the estimated state vector.  Thus, the
% continuous-time linearized system can be represented as:
%
%    Linearized Continuous-Time Model
%    --------------------------------
%    xdot(t) = F(t)*x(t) + Fu(t)*u(t) + w(t),    E(w(t)*w(t)')=Q(t)
%    z(t)    = H(t)*x(t) + Hu(t)*u(t) + v(t),    E(v(t)*v(t)')=R(t)
%
%    where:
%       F(t) = Jacobian of f(x,u) with respect to x(t) (Linearized mapping from x(t) to xdot(t).)
%       H(t) = Jacobian of h(x,u) with respect to x(t) (Linearized mapping from x(t) t z(t).)
%       Fu(t)= Jac. of f(x,u) wrt u(t). (Fu(t) is not explicitly needed because xdot(t) will be formed from f(x,u).)
%       Hu(t)= Jac. of h(x,u) wrt u(t). (Hu(t) is not explicitly needed because z(t) will be formed from h(x,u).)
%
% Finally, Kalman Filters are generally implemented in the discrete-time
% domain, thus the above continuous-time model must be converted to
% discrete time, where "k" indicates the time step:
%
%    Linearized Discrete-Time Model
%    --------------------------------
%    x(k+1)  = PHI(k)*x(k) + PHIu(k)*u(k) + w(k),   E(w(k)*w(k)')=Q(k)
%    z(k)    = H(k)  *x(k) + Hu(k)  *u(k) + v(k),   E(v(k)*v(k)')=R(k)
%
%    where:
%      PHI(k):  State Transition Matrix from time k to time k+1 
%               (PHI(k) is determined via VanLoan method, described later.)
%      H(k):    Linear mapping from states to measurement at time k
%               (H(k) is equal to H(t) at time k.)
%      Q(k):    Covariance of discrete-time process noise w(k) 
%               (Q(k) is not equal to inputted Q(t).  Q(k) is determined via VanLoan method.)
%      R(k):    Covariance of the sampled (discrete-time) measurement noise v(k). 
%      PHIu(k): Mapping from u(k) to x(k+1). (Not explicitly needed)
%      Hu(k):   Mapping from u(k) to z(k). (Not explicitly needed)
%
% The purpose of the Kalman Filter is to provide an estimate of x given
% measurements z.  In EKF parlance, the estimate of x is denoted xhat.
% Similarly, the estimate of the measurement z recreated from xhat is
% denoted zhat.  [For a linear measurement: zhat=H*xhat+Hu*u. For a 
% non-linear measurement: zhat=h(xhat,u).]
%
% To accomodate a wide array of system models with either linear or
% non-linear state dynamics and measurement models, PERFORM_EKF requires
% handles to user-provided functions to generate the dynamics and 
% measurement models:
%
%      @compute_xdot_and_F:  [xdot F] = compute_xdot_and_F(xhat,u1,u2,...,uN)
%         This function must return both the nx1 state derivative vector, 
%         xdot, and the nxn continuous-time linearized dynamics matrix, F.  
%         The inputs to the function are the current state estimates, xhat,
%         and any number of additional inputs (u1,...,uN), as necessary.  
%
%      @compute_zhat_and_H:  [zhat H] = compute_zhat_and_H(xhat,u1,u2,...,uN)
%         This function must return both a model-based estimate (zhat) of 
%         the mx1 measurement vector z and the mxn linearized measurement 
%         matrix H.  The inputs to the function are the current state  
%         estimates, xhat, and any number of additional inputs (u1,...,uN), 
%         as necessary.
%
% Note that @compute_xdot_and_F and @compute_zhat_and_H must use the exact 
% same input arguments: (xhat,u1,u2,...,uN).
%
% Using the inputted quantities, the EKF is performed in the following
% manner:
%
%    1) The Van Loan method (see Refs. 1 & 3) is used to numerically 
%       convert the continuous-time linearized dynamics model and process 
%       noise matrices, F(t) and Q(t), into the discrete-time state 
%       transition matrix, PHI(k), and discrete-time process noise, Q(k):
%          [F(t), Q(t), dt] => [PHI(k), Q(k)]
%
%    2) The state estimate vector at time k, xhat(k), is propagated dt  
%       seconds via the modeled state derivatives vector, xdot (using Euler
%       integration):
%          xhat(k+1) = xhat(k) + xdot*dt;
%
%    3) The state covariance matrix at time k, P(k), is propagated dt
%       seconds via the state transition matrix, PHI(k), and the
%       discrete-time process noise, Q(k):
%          P(k+1) = PHI(k)*P(k)*PHI(k)' + Q(k)
%
%    4) If a measurement is supplied (i.e. z_k is not empty), the
%       propagated state estimate, xhat(k+1), and covariance, P(k+1), are
%       updated based on the measurement.  The update steps are as follows:
%
%          a) Form the Kalman gain matrix, K, which forms a matrix
%             weighting the state uncertainties, P(k+1), with the
%             measurement uncertainty, R(k).
%                K = P(k+1)*H'*inv(H*P(k+1)*H'+R(k))
%
%          b) Update state estimate at time k+1 using the residual error
%             between the actual measurement, z, and the model of that
%             measurement, zhat.  The Kalman gain K dictates how much the
%             measurement will affect the state estimate:
%                xhat(k+1) = xhat(k+1) + K*(z-zhat)
%
%          c) Upate state covariance matrix at time k+1.  Effectively, this
%             update reduces the state uncertainties due to information
%             gained in the measurement:
%                P(k+1) = (I - K*H)*P(k+1)   (Note: I is the identity matrix)
%
% Notes:
%  - Kalman filtering nomenclature varies widely.  The variable usage
%    herein mostly follows that used in Reference 1.
%  - The theoretical EKF formulation, as represented here, involves 
%    a matrix inverse.  Matrix inversion is highly inefficient and
%    can result in numerical errors.  Other methods, such as Cholesky
%    Factorization, are generally preferred.  (See Reference 4)
%
% References:
% 1. Brown. R. G. and Hwang, P., "Introduction to Random Signals and 
%    Applied Kalman Filtering. John Wiley & Sons, Inc., New York, 1992
% 2. Wikipedia site on Extended Kalman Filter:
%    http://en.wikipedia.org/wiki/Extended_Kalman_filter
% 3. Wikipedia site on Discretization, for discussion of Van Loan Method:
%    http://en.wikipedia.org/wiki/Discretization
% 4. Matlab Central File Exchange contribution "Learning the Extended 
%    Kalman Filter" by Yi Cao: 
%    http://www.mathworks.com/matlabcentral/fileexchange/18189-learning-the-extended-kalman-filter
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if nargin==0
    error('PERFORM_EKF is not a stand-alone routine. type HELP PERFORM_EKF for information')
end

% Make derivatives vector (xdot) and linearized state dynamics
% matrix (F).
%   xdot: estimated state derivatives with respect to time
%            xdot(t) = dx/dt = f(xhat,u)  (nx1 vector)
%   F:    linearized state derivative (Jacobian) matrix 
%            F = d(xdot)/dx = d( f(xhat,u) )/dx  (nxn matrix)
%
% Note: varargin{:} consists of the any additional inputs (u1, u2, ...) 
% necessary to compute xdot and F.
[xdot F] = compute_xdot_and_F(xhat_k,varargin{:});

% Use VanLoan Method to convert from linearized continuous-domain state 
% model (F,Q_t), into discrete-domain state transformation (PHI_k & Q_k).
% Thus, assuming u(t) is relatively constant between times k and k+1, it 
% converts
%   xdot(t)=F*x(t)+Fu*u(t)+w(t), E[w(t)*w(t)']=Q(t) 
% to
%   x(k+1)=PHI(k)*x(k)+PHIu(k)*u(k)+w(k), E[w(k)*w(k)']=Q(k).
%
% VanLoan Method:
%
%       [ -F(t) | Q(t) ]
%  AA = [-------+------]*dt
%       [  zero | F(t)']
%
%                  [ ... | inv(PHI(k))*Q(k)]
%  BB = expm(AA) = [-----+-----------------]
%                  [ zero|      PHI(k)'    ]
%
% Thus, PHI(k) is the transpose of the lower-right of BB.  Using the upper
% right of BB, Q(k) = PHI(k) * inv(PHI(k))*Q(k).  The derivation of PHIu is 
% not provided because it is not needed.
%
nStates=length(xhat_k);
AA = [-F Q_t; zeros(nStates,nStates) F']*dt;
BB = expm(AA); % <- Matrix exponential!
PHI_k = BB(nStates+1:2*nStates,nStates+1:2*nStates)';
Q_k = PHI_k*BB(1:nStates,nStates+1:2*nStates);

% Predict state estimate xhat forward dt seconds.
%
% We use Euler integration on the non-linear state derivatives, 
% xdot(t)=f(x(t),u(t)), to propagate xhat dt seconds from time k to time
% k+1:
%    xhat(k+1) = xhat(k) + xdot(t)*dt.
% We alternatively could have used:
%    xhat(k+1) = PHI*xhat(k) + PHIu*u(k).
% Doing so, however, would have introduced some linearization error and
% required us to compute PHIu.
xhat_k = xhat_k + xdot*dt;

% Predict state covariance (P_k) forward dt seconds.
%   PHI_k: State transition matrix from time k to time k+1
%   Q_k:   Discrete-time process noise matrix
%
% Note: In the P_k equation below, the PHI_k*P_k*PHI_k' term directly
% propagates the "a priori" state uncertainty from time k to time k+1.  The
% Q_k term adds additional uncertainty at each time step due to the process
% noise, w(k).
%    P(k+1) = PHI(k)*P(k)*PHI(k)' + Q(k)
P_k = PHI_k*P_k*PHI_k' + Q_k;

% Perform measuremement update
if ~isempty(z_k)
    
    % zhat: re-creation of measurement z, based on current state estimate
    %          zhat(k) = zhat(t) = h(xhat,u) (mx1 vector)
    % H:    linearized measurement matrix (Jacobian of h, with respect to the EKF states)
    %          H = d(zhat)/dx = d( h(xhat,u) )/dx (mxn matrix)
    %
    % Note: varargin{:} consists of the any additional inputs (u1, u2, ...) 
    % necessary to compute zhat and H.
    [zhat H] = compute_zhat_and_H(xhat_k,varargin{:});

    % K: Kalman gain matrix
    %  K = P_k*H'*inv(H*P_k*H'+R_k);
    %
    % Note: in Matlab, inv() can be very slow and inaccurate, so we'll use
    % the forward-slash ("/", see "help mrdivide") instead.  
    % See Reference 4 for a potentially more efficient Cholesky 
    % Factorization method for computing K, xhat_k & P_k.
    K = (P_k*H')/(H*P_k*H'+R_k);
    
    % Update of state estimates using measurement residual (z-zhat).
    %
    % Effectively, K provides a means weighting how much the measurement
    % should affect the state estimate, xhat.  Thus, if the measurement 
    % uncertainty (R_k) is "large" compared with the state uncertainty (P_k)
    % then gain matrix K will be "small", meaning that the new measurement 
    % will have little effect on the state estimate.
    % In contrast, if the measurement uncertainty is small, then K will
    % be large meaning we have high confidence in the measurement (z_k) and
    % will weight it more than the predicted state estimate.
    xhat_k = xhat_k + K*(z_k-zhat);       
    
    % Update of state covariance matrix
    %
    % Effectively, this update reduces the state uncertainties due to 
    % information gained in the measurement.
    %   P(k+1) = (I - K*H)*P(k+1)   (Note: I is the identity matrix)
    %          = P(k+1) - K*H*P(k+1)
    P_k = (eye(length(xhat_k))-K*H)*P_k;  
    
end

% Assign output values.
% These should be inputted back into perform_ekf at the subsequent time 
% step.
xhat_new=xhat_k; % xhat(k+1)
P_new   =P_k;    % P(k+1)
