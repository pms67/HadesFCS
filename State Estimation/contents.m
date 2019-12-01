% Small Fixed-Wing UAV State Estimation Examples
%
% This code provides example methods for performing state estimation
% (i.e. position and orientation) of a fixed-wing UAV (Unmanned Aerial
% Vehicle) using GPS, gyros, accelerometers, and a magnetometer as sensors.
% This code is meant as a companion to the JHU/APL Technical Digest article
% "Fundamentals of Small Unmanned Aircraft Flight", by Jeff Barton (see Reference 1).
%
% References:
%   1  Barton, J. D., “Fundamentals of Small Unmanned Aircraft Flight,” 
%      Johns Hopkins APL Technical Digest, Volume 31, Number 2 (2012).
%      http://www.jhuapl.edu/techdigest/TD/td3102/
%
% Definitions:
%   EKF: Extended Kalman Filter
%   AHRS: Attitude and Heading Reference System
%   INS: Inertial Navigation System
%
% Contents:
%
%  SUAS_Code
%   |-uav_state_estimation.m:                  Main state estimation driver routine
%   |
%   |-generate_uav_sensors_structure.m:        Make noisy sensor measurements from truth data
%   |                                            (Utilized in uav_state_estimation.m) 
%   |-display_uav_estimate_results:            Display results from uav_state_estimation.m
%   |                                            (Utilized in uav_state_estimation.m) 
%   |-perform_ekf.m:                           Extended Kalman Filter utility
%   |                                            (Utilized in uav_state_estimation.m) 
%   |-contents.m:                              This file
%   |
%   |-rotation_examples.m                      Stand-alone routine showing
%   |                                            conversions between rotation
%   |                                            methods utilized in this code
%   | 
%   |-UavData
%   |  '-UavData_LetterE.mat                   Data file providing truth data
%   |  '-UavData_LetterE_withGusting.mat       Data file providing truth data
%   |  '-UavData_Loiter.mat                    Data file providing truth data
%   |  '-UavData_Loiter_withGusting.mat        Data file providing truth data
%   |  '-UavData_SquareSpiral.mat              Data file providing truth data 
%   |  '-UavData_SquareSpiral_withGusting.mat  Data file providing truth data
%   |
%   |-AHRS_EKF_Euler                           Routines for Euler-based AHRS EKF state estimation
%   |  |-perform_state_estimation__ahrs_ekf_euler.m
%   |  |-compute_xdot_and_F__ahrs_ekf_euler.m
%   |  '-compute_zhat_and_H__ahrs_ekf_euler.m
%   |
%   |-AHRS_EKF_Quaternion                      Routines for Quaternion-based AHRS EKF state estimation
%   |  |-perform_state_estimation__ahrs_ekf_quaternion.m
%   |  |-compute_xdot_and_F__ahrs_ekf_quaternion.m
%   |  '-compute_zhat_and_H__ahrs_ekf_quaternion.m
%   |
%   |-AHRS_Feedback                            Routines for AHRS Feedback Method state estimation
%   |  '-perform_state_estimation__ahrs_ekf_euler.m
%   |
%   |-INS_EKF_Euler                            Routines for Euler-based INS EKF state estimation
%   |  |-perform_state_estimation__ins_ekf_euler.m
%   |  |-compute_xdot_and_F__ins_ekf_euler.m
%   |  '-compute_zhat_and_H__ins_ekf_euler.m
%   |
%   '-INS_EKF_Quaternion                       Routines for Quaternion-based INS EKF state estimation
%      |-perform_state_estimation__ins_ekf_quaternion.m
%      |-compute_xdot_and_F__ins_ekf_quaternion.m
%      '-compute_zhat_and_H__ins_ekf_quaternion.m
%
% To run state estimation examples, merely run uav_state_estimation.m.
%
% Copyright © 2012 The Johns Hopkins University / Applied Physics Laboratory LLC.  All Rights Reserved.

% SUAS Code Version: 1.0, October 16, 2012  
% Author: Jeff Barton at JHU/APL, jeffrey.barton@jhuapl.edu
% Website: http://www.jhuapl.edu/ott/Technologies/Copyright/SuasCode.asp

help contents
